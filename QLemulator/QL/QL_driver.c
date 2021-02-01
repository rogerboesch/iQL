//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#include "QL_68000.h"

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>

#include "QL.h"
#include "base_xcodes.h"

#include "base_driver.h"
#include "QDOS.h"

#include "QL_instaddr.h"
#include "base_unix.h"
#include "base_cfg.h"
#include "base_proto.h"

#define min(_a_,_b_)(_a_<_b_ ? _a_ : _b_)
#define max(_a_,_b_)(_a_>_b_ ? _a_ : _b_)

#ifdef __EMX__
#define strncasecmp strncmp
#endif

#ifdef strchr
#undef strchr
#endif
#define strchr(_str_,_c_) my_strchr(_str_,_c_)



extern void rts(void);
void DrvOpen(void);
void DrvIO(void);
void DrvClose(void);

int qerrno;

int prt_init(int,void *);
int prt_open(int, void**);
int prt_test(int, char*);
void prt_close(int, void *);
void prt_io(int, void *);



int num_drivers=0;

open_arg prt_opt_vals[]={0,1};
open_arg prt_tra_vals[]={0,1};
open_arg prt_cmdoptions[]={0};  /*simpler to check for arg presence ;-) */
open_arg prt_cmd[]={0};
struct PARENTRY prt_pars[]={{parse_option,"f",prt_opt_vals},
			    {parse_option,"t",prt_tra_vals},
			    {parse_nseparator,"_",prt_cmdoptions},
			    {parse_nseparator,"!",prt_cmd},
			    {NULL,NULL,NULL}};
struct NAME_PARS prt_name={ "PRT",4,&prt_pars};

#define BDEV
#ifdef BDEV
extern int boot_init(int,void *);
extern int boot_open(int, void**);
extern int boot_test(int, char*);
extern void boot_close(int, void *);
extern void boot_io(int, void *);


struct PARENTRY boot_pars[]={{NULL,NULL,NULL}};
struct NAME_PARS boot_name={"BOOT",0,&boot_pars};
#endif

#ifdef NEWSERIAL
extern int ser_init(int,void *);
extern int ser_open(int, void**);
extern int ser_test(int, char*);
extern void ser_close(int, void *);
extern void ser_io(int, void *);

open_arg ser_unit[]={1};
open_arg ser_ovals1[]={0,1,2,3,4};
open_arg ser_ovals2[]={-1,-1,0};
open_arg ser_ovals3[]={-1,-1,0,1};
open_arg ser_sv0[]={0};   /* Dummy '_' separator*/
open_arg ser_sv[]={9600};

struct PARENTRY ser_pars[]={{parse_value,NULL,ser_unit},
			    {parse_option,"OEMS",ser_ovals1},
			    {parse_option,"HI",ser_ovals2},
			    {parse_option,"RZC",ser_ovals3},
			    {parse_separator,"_",ser_sv0}, /* dummy */
			    {parse_separator,"b",ser_sv},
			    {NULL,NULL,NULL}};
struct NAME_PARS ser_name={"SER",6,&ser_pars};
#endif

#ifdef NEWPTY
extern int pty_init(int, void *);
extern int pty_open(int, void**);
extern int pty_test(int, char*);
extern void pty_close(int, void *);
extern void pty_io(int, void *);

open_arg pty_ovals1[]={0,1,2};
open_arg pty_ovals2[]={-1,-1,0,1,3};
open_arg pty_val[]={(open_arg)""};
struct PARENTRY pty_pars[]={{parse_option,"IK",pty_ovals1},
			    {parse_option,"RZCT",pty_ovals2},
			    {parse_nseparator,"_",pty_val},
			    {NULL,NULL,NULL}};
struct NAME_PARS pty_name={"PTY",3,&pty_pars};
#endif

#ifdef POPEN_DEV
extern int popen_init(int);
extern int popen_open(int, void**);
extern int popen_test(int, char*);
extern void popen_close(int, void *);
extern void popen_io(int, void *);
open_arg popen_val[]={(open_arg)""};
struct PARENTRY popen_pars[]={{parse_nseparator,"_",popen_val},
			      {NULL,NULL,NULL}};
struct NAME_PARS popen_name={"popen",1,&popen_pars};
#endif

#include "QL_files.h"
#include "QL_filespriv.h"
#include "QL_vfs.h"

#ifdef IPDEV
#include "QL_ip.h"
open_arg ip_host[]={(open_arg)""};
open_arg ip_port[]={(open_arg)""};
struct PARENTRY ip_pars[]={
			    {parse_mseparator,"_:",ip_host}, /* dummy */
			    {parse_mseparator,":",ip_port},
			    {NULL,NULL,NULL}};
struct NAME_PARS tcp_name={"TCP",2,&ip_pars};
struct NAME_PARS udp_name={"UDP",2,&ip_pars};
struct NAME_PARS uxs_name={"UXS",2,&ip_pars};
struct NAME_PARS uxd_name={"UXD",2,&ip_pars};
struct NAME_PARS sck_name={"SCK_",0, NULL};

#endif

#ifdef TEST
int bg_init(int, void *);
int bg_open(int,void **);
int  bg_test(int,char *);
void bg_close(int, void *);
void bg_io(int, void *);

open_arg bg_val[]={133};
open_arg bg_ovals[]={-1,'a','b','c','d','e','f','g','h','i','j','k','l'};
open_arg bg_sv1[]={11};
open_arg bg_sv2[]={12};
open_arg bg_sv3[]={13};
open_arg bg_sv4[]={(open_arg)"string1"};
open_arg bg_sv5[]={(open_arg)"string2"};
struct PARENTRY bg_pars[]={{parse_value,NULL,bg_val},
			   {parse_option,"abcdefghijkl",bg_ovals},
			   {parse_separator,"_",bg_sv1},
			   {parse_separator,"/",bg_sv2},
			   {parse_separator,"x",bg_sv3},
			   {parse_nseparator,"--",bg_sv4},
			   {parse_nseparator,",",bg_sv5},
			   {NULL,NULL,NULL}};
struct NAME_PARS bg_name={"BG",4,&bg_pars};
#endif

#ifdef XSCREEN

open_arg scr_sv1[]={448};
open_arg scr_sv2[]={180};
open_arg scr_sv3[]={32};
open_arg scr_sv4[]={16};
open_arg scr_sv5[]={-1};

struct PARENTRY  scr_pars[]={{parse_separator,"_",scr_sv1},
			     {parse_separator,"x",scr_sv2},
			     {parse_separator,"a",scr_sv3},
			     {parse_separator,"x",scr_sv4},
			     {parse_separator,"_",scr_sv5},
			     {NULL,NULL,NULL}};
struct NAME_PARS scr_name={"SCR",6,&scr_pars};
struct NAME_PARS con_name={"CON",6,&scr_pars};
#endif

#ifdef SOUND
#include "QL_sound.h"
extern int sound_init(int, void *);
extern int sound_open(int, void**);
extern int sound_test(int, char*);
extern void sound_close(int, void *);
extern void sound_io(int, void *);


open_arg sound_oread[]= {-1,'r'};
open_arg sound_sm[]= {-1,'m','s'};
open_arg sound_x[]= {0,'x'};
open_arg sound_subdev[]={-1,'1','2','3','4','5','6','7','8','9','s'};
open_arg sound_sv2[]={20001};

struct PARENTRY  sound_pars[]={{parse_option,"r",sound_oread},
                               {parse_option,"ms",sound_sm},
                               {parse_option,"x",sound_x},
                               {parse_option,"123456789",sound_subdev},
                               {parse_separator,"_",sound_sv2},
			       {NULL,NULL,NULL}};
struct NAME_PARS sound_name={"SOUND",5,&sound_pars};
#endif

struct NAME_PARS qvf_name={"VFS_",0,NULL};

struct DRV Drivers[]={
  {0,prt_init, prt_test, prt_open, prt_close, prt_io, &prt_name, 0},
#ifdef NEWSERIAL
  {0,ser_init, ser_test, ser_open, ser_close, ser_io, &ser_name, 0},
#endif
#ifdef NEWPTY
  {0,pty_init, pty_test, pty_open, pty_close, ser_io, &pty_name, 0},
#endif
#ifdef BDEV
  {0,boot_init, boot_test, boot_open, boot_close, boot_io, &boot_name, 0},
#endif
#ifdef TEST
  {0,bg_init,bg_test,bg_open,bg_close,bg_io,&bg_name,0},
#endif
#ifdef IPDEV
  {0, ip_init, ip_test, ip_open, ip_close, ip_io, &tcp_name, 0},
  {0,ip_init,ip_test,ip_open,ip_close,ip_io,&udp_name,0},
  {0,ip_init,ip_test,ip_open,ip_close,ip_io,&uxs_name,0},
  {0,ip_init,ip_test,ip_open,ip_close,ip_io,&uxd_name,0},
  {0,ip_init,ip_test,ip_open,ip_close,ip_io,&sck_name,0},
#endif
#ifdef POPEN_DEV
  {0,popen_init,popen_test,popen_open,popen_close,popen_io,&popen_name,0},
#endif
#ifdef SOUND
  {0,sound_init, sound_test, sound_open, sound_close, sound_io, &sound_name, 0x100},
#endif
  /* QVFS should always be LAST in the list */
#ifdef QVFS
  {0,qvf_init, qvf_test, qvf_open, qvf_close, qvf_io, &qvf_name, 0},
#endif
  {0,NULL,NULL,NULL,NULL,NULL,NULL}};


w32 DEV_IO_ADDR, DEV_CLOSE_ADDR;


struct DRV *dget_drv()
{
  struct DRV *p;

  p=Drivers;
  while(p->open!=NULL)
    {
      if (p->ref==aReg[3]+0x18)
	return p;
      p++;
    }
  return 0;
}

static void InitDevDriver(struct DRV *driver, int indx)
{
  w32	savedRegs[4];
  w32	*p;
  char  *name=(driver->namep)->name;

  BlockMoveData(aReg,savedRegs,4*sizeof(w32));
  reg[1]=40+strlen(name);
  if((strlen(name)&1)!=0) reg[1]++;

  if (driver->slot != 0)
    {
      if(driver->slot <reg[1])
	{
	  printf("requested driver size for driver %s too small: %d\n",*name,driver->slot);
	  goto ddier;
	}
      reg[1]=driver->slot;
    }

  reg[2]=0;
  QLtrap(1,0x18,200000l);		/* allocate memory for the driver linkage block */
  if((*reg)==0)
    {
      driver->ref=aReg[0];
      p=(w32*)(aReg[0]+(Ptr)theROM+4);
      WL(p,DEV_IO_ADDR);       /* io    */
      WL(p+1,(w32) ((Ptr)(p+3)-(Ptr)theROM));             /* open  */
      WL(p+2,DEV_CLOSE_ADDR);  /* close */

      WW(p+3,DEVO_CMD_CODE);

      strcpy((Ptr)(p+6)+4,name);      /* name for QPAC2 etc */
      WW((Ptr)(p+3+3)+2,strlen(name));
      WL((Ptr)(p+3)+2,0x264f4eba);    /* so much code is needed to fool QPAC2 ...*/
      WL((Ptr)(p+4)+2,0x2c566046);
      WL((Ptr)(p+5)+2,0x6044604a);


      if ((*(driver->init))(indx,p-1) <0)
	goto ddier;

      QLtrap(1,0x20,20000l);	/* link directory device driver in IOSS */
    }
 ddier:
  BlockMoveData(savedRegs,aReg,4*sizeof(w32));
}

void InitDrivers()
{
  struct DRV *p=Drivers;

  DEV_IO_ADDR=0x14020;
  DEV_CLOSE_ADDR=0x14022;

  WW(((uw16*)((Ptr)theROM+DEV_IO_ADDR)),DEVIO_CMD_CODE);
  WW(((uw16*)((Ptr)theROM+DEV_CLOSE_ADDR)),DEVC_CMD_CODE);

  qlux_table[DEVO_CMD_CODE]=DrvOpen;
  qlux_table[DEVIO_CMD_CODE]=DrvIO;
  qlux_table[DEVC_CMD_CODE]=DrvClose;

  while (p->open!=NULL)
    {
      InitDevDriver(p++,p-Drivers);
      num_drivers++;
    }
}


char *a0addr(Cond check)
{
  char *f;
  if(*aReg<131072 || *aReg>RTOP-130)
    {
      return nil;
    }
  f=(char *)((Ptr)theROM+((*aReg)&ADDR_MASK_E));
  if(!check) return f;
  if(DGET_ID(f)==DRV_ID) return f;
  return nil;
}

void DrvIO(void)
{
  char *f;
  struct DRV *driver;
  int ix;


  if((long)((Ptr)gPC-(Ptr)theROM)-2 != DEV_IO_ADDR)
    {
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      return;
    }
  f=a0addr(false);

  if(f==nil)
    {
      *reg=QERR_NO;	/* overflow */
      rts();
      return;
    }

  driver=dget_drv(); /*&Drivers[DGET_DRV(f)];*/
  ix=driver-Drivers;

  if (driver==0)
    {
      printf("possible driver problem ??\n");
      return;
    }


  (*(driver->io))(ix,DGET_PRIV(f));

  rts();
}


void DrvOpen(void)
{
  char * f=nil;
  char *name;
  struct DRV *p, *drv=Drivers;
  void *priv;
  int err,found=0;

  name=(char *)((Ptr)theROM+((*aReg)&ADDR_MASK_E));

    /* get device */

    p=dget_drv();
    found=(*(p->open_test))(p-Drivers,name);

    if (!found)
      {
	reg[0]=QERR_NF;
	goto end;
      }
    if (found==-2)
      {
	reg[0]=qerrno;
	goto end;
      }
    if (found==-1)
      {
	reg[0]=QERR_BN;
	goto end;
      }

    err=(*(p->open))(p-Drivers,&priv);

    if (err==0)
      {
	reg[1]=DRV_SIZE;
	reg[2]=0;
	QLvector(0xc0,20000l);
	if ((uw16)reg[0]) goto end;

	f=a0addr(false);
	if (f)
	  {
	    DSET_ID(f,DRV_ID);
	    DSET_PRIV(f,priv);
	  }
      }
    if (err>=0)
      reg[0]=0;

    if (err<0)
      reg[0]=err;
 end:
    rts();
}



void DrvClose(void)
{
  char *f;
  struct DRV *driver;
  int ix;
  w32 saved_regs[16];

  if((long)((Ptr)gPC-(Ptr)theROM)-2 != DEV_CLOSE_ADDR)
    {
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      return;
    }

  //save_regs(saved_regs);
  f=a0addr(false);

  if(f==nil)
    {
      *reg=QERR_NO;	/* overflow */
      rts();
      return;
    }

  driver=dget_drv(); /*&Drivers[DGET_DRV(f)];*/
  ix=driver-Drivers;

  if (driver==0)
    {
      printf("possible driver problem ??\n");
      return;
    }

  (*(driver->close))(ix,DGET_PRIV(f));

  QLvector(0xc2,20000l);

  rts();

}

/* *********** generic helper routines  *********** */

char *strchr_noalpha(char *str, char c)
{
  char *res=(char*)0;
  do
    {
    if (*str==c)
      {
	res=str;
	break;
      }
    }while(*str++);

  return res;
}
char *strchr_alpha(char *str, char c)
{
  char *res=(char*)0;

  c=c&0xdf;
  do
    {
    if (((*str)&0xdf)==c)
      {
	res=str;
	break;
      }
    }while(*str++);

  return res;
}

char *my_strchr(char *str, char c)
{
  if (isalpha(c))
    return strchr_alpha(str, c);
  else return strchr_noalpha(str,c);
}


static char rest_name[1025];
static char *ppname;


int parse_separator(char **name,int nlen,char *opts,open_arg *vals,open_arg *res)
{
  if (tolower(**name)!=*opts)
    {
      res->i=vals->i;
      return 0;
    }

  (*name)++;
  return parse_value(name,nlen-1,NULL,vals,res);
}

int parse_nseparator(char **name,int nnlen,char *opts,open_arg *vals,open_arg *res)
{
  char *nend;
  int nlen;

  if (**name!=*opts)
    {
    noval:
      res->s=vals->s;
      return 0;
    }

  (*name)++;
  nend=strchr(*name,opts[1]);  /**/
  if (!nend) nend=strchr(*name,0);
  if (!nend || nend==*name) goto noval;

  nlen=nend-*name;
  strncpy(ppname,*name,nlen);
  ppname[nlen]=0;
  res->s=ppname;

  ppname=&ppname[nlen+1];
  *name=nend;
  if (*name<strchr(*name,0))
    (*name)++;
  return 1;
}

#ifdef IPDEV
int parse_mseparator(char **name,int nnlen,char *opts,open_arg *vals,open_arg *res)
{
  char *nend;
  int nlen;

  if (**name!=*opts)
    {
    noval:
      res->s=vals->s;
      return 0;
    }

  (*name)++;
  nend=strchr(*name,opts[1]);  /**/
  if (!nend) nend=strchr(*name,0);
  if (!nend || nend==*name) goto noval;

  nlen=nend-*name;
  memcpy(ppname,*name,nlen);
  ppname[nlen]=0;
  res->s=ppname;

  ppname=&ppname[nlen+1];
  *name=nend;
  return 1;
}
#endif

int parse_value(char **name,int nlen,char *opts,open_arg *vals,open_arg *res)
{
  char *p=*name;
  int r;

  r=strtol(*name,name,10);
  if (*name==p)
    {
      res->i=vals->i;
      return 0;
    }
  else {
    res->i=r;
    return 1;
  }
}

int parse_option(char **name,int nlen,char *opts,open_arg *vals,open_arg *res)
{
  char *p;

  if (**name)
    p=strchr(opts,**name);
  else p=NULL;
  if (!p)
    {
      res->i=vals->i;
      return 0;
    }

  (*name)++;
  res->i=vals[p-opts+1].i;

    return 1;
}


/* returns -1 bad name, 0 not found, >0 success*/
int decode_name(char *name, struct NAME_PARS *ndescr, open_arg *parblk)
{
  int res;
  open_arg rval;
  struct PARENTRY *pars;
  char *nend;
  int j;
  int i=ndescr->pcount;

  ppname=rest_name;

  if ( RW(name)<strlen(ndescr->name) || strncasecmp(name+2,ndescr->name,strlen(ndescr->name)) )
    return 0;
  nend=name;
  if (RW(name)>1024) return -1;
  name=rest_name;

  j=RW(nend)-strlen(ndescr->name);
  strncpy(name,nend+2+strlen(ndescr->name),j);
  name[j]=0;

  nend=&name[j];
  pars=ndescr->pars;
  while(i-- /*&& name<nend*/ && pars->func)
    {
      res=(*(pars->func))(&name,nend-name,pars->opt,pars->values,&rval);
      switch (res)
	{
	case -1 : return -1;
	case 0 :
	case 1 : pars++; *parblk++=rval;
	  break;
	default:

            return -1;
	}
    }
  if (name != nend) return -1;
  else return 1;
}





static char buf[1024];

int ioskip(int (*io_read)(void *, void *,int), void *priv,int len)
{
  int res=0,ss=0;

  while(len>0)
    {
      res=(*io_read)(priv,buf,min(len,1024));
      len-=1024;
      if (res>0) ss+=res;
      else break;
    }
  if (res<0) return -ss;
    else  return ss;
}


void ioread(int(*io_read)(), void *priv, uw32 addr, int *count, int lf)
{
  int cnt,ocnt,startpos;
  int c, fn, err,sz,e;
  char *p;
  Ptr i=0;
  uw32 to,from;

  cnt=*count;
  from=addr;

  to=from+cnt;

  if (from<131072)
    {
      err=ioskip(io_read,priv,131072-from);
      if (err<0)
	{
	  cnt=-err;
	  e=QERR_NC;
	  goto errexit;
	}
      else from +=err;
    }
  if (to>=RTOP) to=RTOP;
  ocnt=cnt=to-from;

  e=0;
  if (cnt<0) cnt=0;

  if(cnt>0)
    {
      if (lf)
	{
	  for(i=0,fn=cnt,p=(Ptr)theROM+from;fn>0;)
	    {
	      e=err=(*io_read)(priv,p,1);

	      if (err<0)
		{
		  cnt=(long)((Ptr)p-(Ptr)theROM)-from;
		  goto errexit;
		}

	      if (err==0 && *(p-1)!=10)
		{
		  cnt=(long)((Ptr)p-(Ptr)theROM)-from;
		  e=QERR_EF;    /* QERR_NC ???*/
		  goto errexit;
		}

	      p+=e;
	      fn-=e;
	      if (*(p-1)==10) {i=p; break;}

	    }
	  if (i)
	    {cnt=(long)((Ptr)i-(Ptr)theROM)-from; e=0;}
	  else {cnt=ocnt; e=QERR_BF;}
	}
      else  /* non LF part here */
	{
	  e=err=(*io_read)(priv,(Ptr)theROM+from,cnt);
	  if (e<=0)
	    {
	      if (e==0 && cnt>0) e=QERR_EF;
	      cnt=0;
	      goto errexit;
	    }
	  else
	    if (e<cnt) e=QERR_NC;
	    else  e=0;
	  cnt=err;
	}
    }

errexit:
  *count=cnt;
  ChangedMemory(from,from+cnt);

  reg[0]=e;
  return;
}


/* io_handle, similar to SERIO vector */

void io_handle(int (*io_read)(), int (*io_write)(), int (*io_pend)(),
	       void *priv)
{
  void *addr;
  int  err,res;
  char c;
  int count,rc_count;
  w32 qaddr;
  int op=(w8)reg[0];

  reg[0]=0;

  switch (op)
    {
    case 0:
      *reg=(*io_pend)(priv);
      break;

    case 1:
      res=(*io_read)(priv,&c,1);
      if (res==1)
      *((char*)reg+4+RBO)=c;
      else *reg=res ? res: QERR_EOF;
      break;

    case 2:                   /* read line */
      count=max(0,(uw16)reg[2]-(uw16)reg[1]);
      rc_count=(uw16)reg[1];
      qaddr=aReg[1];
      ioread(io_read,priv,qaddr,&count,true);
      //(uw16)reg[1]=count+rc_count;
      SETREG16(reg[1], count+rc_count);
      aReg[1]=qaddr+count;
      break;

    case 3:			/* fetch string */
      qaddr=aReg[1];
      count=max(0,(uw16)reg[2]-(uw16)reg[1]);
      rc_count=(uw16)reg[1];
      ioread(io_read,priv,qaddr,&count,false);
      reg[1]=count+rc_count;
      aReg[1]=qaddr+count;
      break;

    case 5:
      res=(*io_write)(priv,(Ptr)reg+4+RBO,1);
      if (res<0) *reg=res;
      break;

    case 7:			/* send string */
      count=(uw16)reg[2];
      res=(*io_write)(priv,(Ptr)theROM+aReg[1],count);
      if (res<0)
	{
	  count=0;
	  *reg=res;
	} else count=res;

      reg[1]=count;
      aReg[1]+=count;
      break;

    case 0x48:		/* read file into memory */
      qaddr=aReg[1];
      count=reg[2];
      rc_count=reg[1];
      ioread(io_read,priv,qaddr,&count,false);
      aReg[1]=qaddr+count;
      break;

    case 0x49:
      count=reg[2];
      res=(*io_write)(priv,(Ptr)theROM+aReg[1],count);
      if (res<0)
	{
	  count=0;
	  *reg=res;
	} else count=res;

      aReg[1]+=count;
      break;

    default:
      *reg=QERR_BP;
      break;
    }
}

/* returns (where possible) appropriate QDOS error based on errno value*/
int qmaperr(void)
{
  switch(errno)
    {
    case EPERM   : return QERR_RO;
    case EBUSY   : return QERR_IU;
    case ENOENT  : return QERR_NF;
    case ESRCH   : return QERR_BJ;
    case EINTR   : return QERR_NC;
    case EAGAIN  : return QERR_NC;
    case EBADF   : return QERR_RO;
    case ENOMEM  : return QERR_OM;
    case EACCES  : return QERR_RO;
    case EEXIST  : return QERR_EX;
    case EINVAL  : return QERR_BP;
    case ESPIPE  : return QERR_BP;
    case EROFS   : return QERR_RO;

    default : perror("warning : unknown error");
      return QERR_NI;
    }
}


/*********************************************************/
/* here comes a simple driver example*/

struct PRT_PRIV {FILE *file; int tra;};

int prt_pend()
{
  return QERR_EF;  /* never anything pending */
}

int prt_read(struct PRT_PRIV *p,void *buf,int len)
{
  return QERR_EF;
}

int prt_write(struct PRT_PRIV *p,void *buf,int len)
{
  int res,i,sig,nlen;
  char conv[4];

  if (len==0) return 0;

  if (p->tra)
    {
      i=0;
      sig=0;

      while(len-->0 && !sig)
	{
	  nlen=tra_conv(conv,buf+i,1);
	restart:
	  res=write(fileno(p->file),conv,nlen);
	  if (res<nlen)
	    {
	      sig=1;
	      if (res>0) nlen -=res;
	      if (res>0 || errno==EAGAIN || errno==EINTR)
		goto restart;
	      else
		{
		  if (!i) return qmaperr();
		  else return i;
		}
	    }
	  /* successfull write, phew..:*/
	  i++;
	}
      return i;
    }
  else
    {
      res=write(fileno(p->file), buf, len);
    }

  if (res<0) return qmaperr();
  else return res;
}



int prt_init(int idx, void *d)
{
  return 0;
}

open_arg prt_par[4];

int prt_test(int id, char *name)
{
  return decode_name(name,Drivers[id].namep,prt_par);
}

int prt_open(int id, void **priv)
{
  FILE *f;
  struct PRT_PRIV *p;

  *priv=p=malloc(sizeof(struct PRT_PRIV));
  if (*priv==NULL) return -1;

  if (prt_par[2].i || prt_par[3].i)
    {
      char *p;
      if (prt_par[3].i)
	{
	  p=(void*)malloc((prt_par[2].s ? strlen(prt_par[2].s) : 0)+strlen(prt_par[3].s)+2);
	  strcpy(p,prt_par[3].s);
	}
      else
	{
	  p=(void*)malloc(strlen(QMD.prtcmd)+strlen(prt_par[2].s)+2);
	  strcpy(p,QMD.prtcmd);
	}
      strcat(p," ");
      if (prt_par[2].s)strcat(p,prt_par[2].s); /* add options etc */
      printf("executing command �%s�\n",p);
      f=popen(p,"w");
      if (!f) return qmaperr();
      printf("executing command �%s�\n",p);
      free(p);
    }
  else /* simple case */
    {
      f=popen(QMD.prtcmd,"w");
      if (!f) return qmaperr();
    }

  p->file=f;
  p->tra=prt_par[1].i;

  return 0;
}

void prt_close(int id, void *priv)
{
  struct PRT_PRIV *p=priv;

  pclose(p->file);
  free(p);
}


void prt_io(int id, void *priv)
{
  io_handle(prt_read,prt_write,prt_pend, priv);
}
