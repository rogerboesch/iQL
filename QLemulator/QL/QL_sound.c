//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#ifdef SOUND

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/soundcard.h>

#include "QL_68000.h"
#include "QL_sound.h"
#include "base_proto.h"
#include "base_util.h"
#include "base_driver.h"

// FLUSH: sync (start playing)
// FS.HEADR get parameters
// FS.HEADS set ..
// FS.MDINF get dsp capabilities (ni yet)

// right now only allow 1 device
struct sound_data *delay_list=NULL;
static int sound_iu=0;
struct sound_data *sdata=NULL;

open_arg sound_par[5];
open_arg dsound_par[5]; // save pars for delayed call

static int setpars(int fd, int channels, int freq);
static int sndwritebuf(struct sound_data  *p, void *buf, int pno, int flush);
static void ensure_flush(struct sound_data *p);
static int do_open(int ix,void **p);

int sound_init(int idx, void *d){}

#define o_letter(x) (x>31?x:' ')

#define SP_R    (dsound_par[0].i)
#define SP_MS   (dsound_par[1].i)
#define SP_X    (dsound_par[2].i)
#define SP_N    (dsound_par[3].i)
#define SP_FREQ (dsound_par[4].i)

static int delay_open(int ix, void **p)
{
  // only allow 1 delayed open AFTER preceeding channel was closed
  if (!sdata || !sdata->close)
    return 0;

  *p=sdata;
  sdata->close=0;
  sdata->reopen=1;
  return 1;
}

int sound_open(int ix,void **p)
{
  memcpy(dsound_par,sound_par,(5*sizeof(open_arg)));
  if (sound_iu)
    {
      if (delay_open(ix,p))
	return 0;
      return QERR_IU;
    }

  return do_open(ix,p);
}

static int do_open(int ix,void **p)
{
  struct sound_data *priv;
  int channels,res;
  int fd,flags,fmt;

  // disallow combinations like SOUNDs3_13451
  if (SP_N != -1 && (SP_MS !=-1 || SP_FREQ !=20001))
    return QERR_BP;

  // Allow SOUND1 to SOUND9 to match Simon Goodwin's 68K SOUND device
  if (SP_N != -1)
    {
      if ((SP_N >= '3') && (SP_N <= '4'))
	{
	  SP_FREQ=10000;
	}
                               
      if (SP_N > '4')
	{
	  SP_FREQ=40000;
	}
      
      channels = 2 - (SP_N & 1); // Odd numbers mono, even stereo
    }
  else channels = SP_MS==2 ? 2 : 1;
  flags = (SP_R>0 ? O_RDWR: O_WRONLY);
  
  // reopen ? ..need to really reopen it because flags might have changed
  if (!p)
    close(sdata->fd);
#ifndef TESTNS
  fd=open("/dev/dsp",flags);
#else
  fd=open("/dev/null",flags);
#endif
  if (fd==-1)
    {
      perror("opening /dev/dsp");
      return qmaperr();
    }

  if (p)
    {
      priv=(struct sound_data *)malloc(sizeof(struct sound_data)+SNDBUFFSIZE);
      if (!priv) 
	{
	  close(fd);
	  return QERR_OM;
	}
      priv->tail=priv->buf=(char*)priv+sizeof(struct sound_data);
      sdata=priv;
      sound_iu++;
    }
  else 
    priv=sdata;
  
  *p=priv;

  priv->reopen=0;
  priv->fd=fd;
  priv->pid=0;
  priv->close=0;
  priv->autoflush=!SP_X;
  priv->dticks=0;

  res=setpars(fd, channels, SP_FREQ);
  if (res!=0)
    {
      sound_close(ix,priv);
      return res;
    }
  return 0;
}

int sound_test(int id,char *name)
{
  return decode_name(name,Drivers[id].namep,&sound_par);
}

int sound_pend(struct sound_data *p)
{
  if (check_pend(p->fd,SLC_READ)) return 0;
  else return QERR_NC;
}

int sound_read(struct sound_data  *p, void *buf, int pno)
{
  int res;

  res=read(p->fd,buf,pno);

  if (res<0) res=qmaperr();
  return res;
}


// write to SOUND:  allways write to malloced buffer
// (auto)flush clears this buffer in parent
//             writes to dsp and flushes in child

// flush works immediately, autoflush only if at least
// 1000 bytes available or after 5 frames (.1s) delay
static int sound_write(struct sound_data  *p, void *buf, int pno)
{
  int res;

  res=sndwritebuf(p,buf,pno,p->autoflush);

  return res;
}

static void fsxinf(struct sound_data *p)
{
  reg[0] = QERR_NI;
}
static void headr(struct sound_data *p)
{
  // test buflen
  int val,rv;
  int err;
  struct dsp_ctrl *x=aReg[1]+(Ptr)theROM;
  int fd=p->fd;

  if (((uw16)reg[2])<sizeof(struct dsp_ctrl))
    {
      reg[0]= QERR_OR;
      return;
    }

  err = ioctl(fd, SOUND_PCM_READ_RATE, &val);
  if (err ==  -1)
    perror("SOUND_PCM_READ_RATE ioctl failed");
  else x->freq=h2ql(val);  //x->freq=htonl(val);
  rv=err;

  err = ioctl(fd, SOUND_PCM_READ_CHANNELS, &val);
  if (err ==  -1)
    perror("SOUND_PCM_READ_CHANNELS ioctl failed");
  else x->channels=h2qw(val);  //x->channels=htons(val);
  rv |= err;

  err = ioctl(fd, SOUND_PCM_READ_BITS, &val);
  if (err == -1)
    perror("SOUND_PCM_READ_BITS ioctl failed");
  else x->bits=h2qw(val);//x->bits=htons(val);
  rv |= err;

  val=AFMT_QUERY;
  err = ioctl(fd, SOUND_PCM_SETFMT, &val); 
  if (err ==  -1)
    perror("SOUND_PCM_SETFMT ioctl failed");
  else x->format=h2ql(val);//x->format=htonl(val);
  rv |= err;

  reg[1] = sizeof(struct dsp_ctrl);
  aReg[1] += sizeof(struct dsp_ctrl);
  if (rv) reg[0]=QERR_BP;
}
static int setpars(int fd, int channels, int freq)
{
  int err;
  int val;

  val = (freq == 20001) ? 20000 : freq;
  err = ioctl(fd, SOUND_PCM_WRITE_RATE, &val);
  if (err == -1)
    perror("SOUND_PCM_WRITE_RATE ioctl failed");
  err = ioctl(fd, SOUND_PCM_READ_RATE, &val);
  if (err ==  -1)
    perror("SOUND_PCM_READ_RATE ioctl failed");
  // currently ignore error on readback and hope it works..
  if ( freq != 20001 && freq != val)
    return QERR_OR;

  val = AFMT_U8;
  err = ioctl(fd, SOUND_PCM_SETFMT, &val); 
  if (err ==  -1)
    perror("SOUND_PCM_SETFMT ioctl failed");
    // must be very obscure system.. forget error handling

  val = channels;
  err = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &val);
  if (err ==  -1)
    perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
  err = ioctl(fd, SOUND_PCM_READ_CHANNELS, &val);
  if (err ==  -1)
    perror("SOUND_PCM_READ_CHANNELS ioctl failed");
  if ( val != channels )
    return QERR_OR;
  return 0;
}
static void heads(struct sound_data *p)
{
  int val,rv;
  int err;
  int fd=p->fd;
  struct dsp_ctrl *x=aReg[1]+(Ptr)theROM;

  val=q2hl(x->freq);
  err = ioctl(fd, SOUND_PCM_WRITE_RATE, &val);
  if (err == -1)
    perror("SOUND_PCM_WRITE_RATE ioctl failed");
  err = ioctl(fd, SOUND_PCM_READ_RATE, &val);
  if (err ==  -1)
    perror("SOUND_PCM_READ_RATE ioctl failed");
  else x->freq=h2ql(val);
  rv=err;

  val=q2hl(x->channels);
  err = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &val);
  if (err ==  -1)
    perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
  err = ioctl(fd, SOUND_PCM_READ_CHANNELS, &val);
  if (err ==  -1)
    perror("SOUND_PCM_READ_CHANNELS ioctl failed");
  else x->channels=h2qw(val);
  rv |= err;

  val=q2hw(x->bits);
  err = ioctl(fd, SOUND_PCM_WRITE_BITS, &val);
  if (err == -1)
    perror("SOUND_PCM_WRITE_BITS ioctl failed");
  err = ioctl(fd, SOUND_PCM_READ_BITS, &val);
  if (err == -1)
    perror("SOUND_PCM_READ_BITS ioctl failed");
  else x->bits=h2qw(val);
  rv |= err;

  val=q2hl(x->format);
  err = ioctl(fd, SOUND_PCM_SETFMT, &val); 
  if (err ==  -1)
    perror("SOUND_PCM_SETFMT ioctl failed");
  val=AFMT_QUERY;
  err = ioctl(fd, SOUND_PCM_SETFMT, &val); 
  if (err ==  -1)
    perror("SOUND_PCM_SETFMT ioctl failed");
  else x->format=h2ql(val);
  rv |= err;

  aReg[1] += sizeof(struct dsp_ctrl);
  if (rv) reg[0]=QERR_BP;
}

static void sound_done(pid_t pid, int id)
{
  struct sound_data *p=(void*)id;
  
  // p==NULL is invalid for closed channels
  if (p)
    {
    p->pid=0;
    if (p->autoflush && ((p->tail)-(p->buf)))
      ensure_flush(p);
    if (p->pid)
      return;   // wait for complete
    if (p->reopen)
      do_open(0,NULL);
    else if (p->close)
      {
	close(p->fd);
	sound_iu--;
	free(p);
      }

    }
}
static void delay_flush(struct sound_data *p)
{
  delay_list=p;
}
void qm_sound(void)
{
  struct sound_data *p;
  p=delay_list;

  if ((p->dticks)>4)
    {
      ensure_flush(p);
    }
  else 
    {
      p->dticks++;
    }
}
static void ensure_flush(struct sound_data *p)
{
  int res,pid,len;

  len= (p->tail) - (p->buf);
  // don´t test len>0 here, only close would call it with 0 and
  // that is needed to get cleanup handler
  if (!p->pid)
    {
      pid=qm_fork(sound_done,(long)p);
      if (pid==0)
	{
	  res = write(p->fd,p->buf,len);
	  if (res < 0)
	    perror("SOUND write failed");
	  res = ioctl(p->fd, SOUND_PCM_SYNC, 0);  
	  if (res == -1)
	    perror("SOUND_PCM_SYNC ioctl failed");
        
        // FIXME: Proper error handling (before was exit)
        return;
	}
      p->pid=pid;
      p->tail=p->buf;
      p->dticks=0;
      delay_list=NULL;
    }
}
static void sound_sync(struct sound_data *p)
{   
  int status,pid;

  if (!p->pid)
    {
      ensure_flush(p);
      if (((uw16)reg[3]) != 0)
	reg[0]=QERR_NC;
      else reg[0]=0;
    }
  else reg[0]=0;
}
static int sndwritebuf(struct sound_data  *p, void *buf, int pno, int flush)
{
  int len=SNDBUFFSIZE-(p->tail-p->buf);
  
  if (pno==0) return 0;
  if (pno<len) len=pno;
  if (len)
    {
      memcpy(p->tail,buf,len);
      p->tail += len;
      if (flush && !p->pid)
	{
	  if (p->tail-p->buf >= 1000)
	    ensure_flush(p);
	  else
	    delay_flush(p);
	}
      return len;
    }
  else 
    { // no place left in buffer
      ensure_flush(p);
      return QERR_NC;
    }
  
}
void sound_io(int ix, void *priv)
{
  struct sound_data *p=priv;
  int op=(w8)reg[0];

  switch(op)
    {
    case 65: return sound_sync(priv);
    case 69: return fsxinf(priv);
    case 70: return heads(priv);
    case 71: return headr(priv);

    default:
      io_handle(sound_read, sound_write, sound_pend, priv);
      return;
    }
}

void sound_close(int ix, void *priv)
{
  struct sound_data *p=priv;
  int pid;

  if (p->close)
  p->close=1;
  ensure_flush(p);
}
#endif
