//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/times.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "QL_68000.h"
#include "QL.h"
#include "QL_instaddr.h"
#include "base_xcodes.h"
#include "base_unix.h"
#include "base_cfg.h"
#include "base_proto.h"
#include "base_util.h"
#include "QDOS.h"

extern int inside;
extern int screen_drawable;
extern int extInt;
extern int sct_size;
extern int QLdone;

extern int rx1,rx2,ry1,ry2,finishflag,doscreenflush,scrcnt;
extern int script;


struct paste_buf
{
  struct paste_buf *next;  /* linked list */
  char *p;                 /* next char */
  int size;                /* # of chars */
  char text[0];
};

struct paste_buf *paste_bl;
int mouse_emu_reg;
int ochd=0;

int min_idle=5;
int HasPTR=0;
int quickMouseUpdate=1;

struct paste_buf *paste_bl=NULL;

#define max(x,y) ({typeof(x) _x=(x); typeof(y) _y=(y); _x>_y ? _x: _y;})
#define sgn(x) ({typeof(x) _x=(x);typeof(x) res=0; res=(_x>0)-(_x<0);res;})

#ifdef MOUSE
static w32 iscrdrv=0;
static w32 ptrscrdrv=0;
static int lastx=0;
static int lasty=0;
static int llastx=0;
static int llasty=0;
static int lastactive=0;
static int ign_x=-1;
static int ign_y=-1;
int invisible=0;

int getMouseX(){return ReadWord(ptrscrdrv+0x20);}
int getMouseY(){return ReadWord(ptrscrdrv+0x22);}
int ptractive(){return !ReadByte(ptrscrdrv+0x38);}


void setMouseXY(int x, int y)
{
  if (x<1) x=1;
  if (y<1) y=1;
  WriteByte(ptrscrdrv+0x16,ReadByte(ptrscrdrv+0x16)+max(abs(x-getMouseX()),abs(y-getMouseY())));

  WriteWord(ptrscrdrv+0x20,x);
  WriteWord(ptrscrdrv+0x22,y);
  WriteByte(ptrscrdrv+0x52-0x18,0);
  WriteByte(ptrscrdrv+0x8f-0x18,0);
}


void print_mousepos()
{
  printf("mouse x,y = %4d,%4d\n",getMouseX(),getMouseY());
}

#endif  /* MOUSE */

void do_hotactn()
{
  char hotch[3];

  hotch[0]=ReadByte(ptrscrdrv+0x8d-0x18);
  hotch[1]=ReadByte(ptrscrdrv+0x8e -0x18);
  hotch[2]=194;

  //insert_keyQ(3,hotch);
}

void paste_into_keyq()
{
  uw32 saveA3;

  do {
    if (!paste_bl) return;

    saveA3=aReg[3];
    aReg[2]=ReadLong(0x2804c);	/* :HIER: abs. sysref raus *//* kann auch null oder -ve werden, wert+aktion dann verwerfen! */
    if (aReg[2]<=0) return;

    QLvector(0xde,2000000l);
    if (reg[2]<1)
      {
	return;
      }
    
    reg[1]=*(paste_bl->p++); /* get char */
    if (paste_bl->p-paste_bl->text>=paste_bl->size)
      {
	struct paste_buf *p;

	p=paste_bl->next;
	free(paste_bl);
	paste_bl=p;
      }
      
    QLvector(0xe0,2000000l);
  }while(reg[0]==0);

  aReg[3]=saveA3;
}

w32 getSCRdriver()
{
  return ReadLong(ReadLong(ReadLong(0x28078))+4);	/* :HIER: abs. sysref raus */
}


/* link in scheduler task */
void SchedInit()
{
  reg[1]=0x10;
  reg[2]=0;
  QLtrap(1,0x18,2000000l);

  if (*reg==0)
    {
      Ptr p=(Ptr)theROM+aReg[0];
      p = p + 4;
	
      WL( p, SCHEDULER_CMD_ADDR);
      WW((Ptr)theROM+SCHEDULER_CMD_ADDR, SCHEDULER_CMD_CODE);
	
      QLtrap(1,0x1e,200000l);
    }
#ifdef MOUSE
  iscrdrv=getSCRdriver();
  if (V3)printf("Initial Screen driver is %x\n",iscrdrv);
#endif
}

int MButtonUndetected=0;
int MButtonPendRelease=0;
int relbnr=0;

#ifdef MOUSE
void QLButton(int bnr,int press)
{
#ifdef DEBUG_M	/* .hpr, 13.9.99 probehalber *//**/
  printf("Button %d %s\n",bnr,press?"pressed":"released");
  gKeyDown=press;
#endif

   if (press && bnr!=2)
     MButtonUndetected=5;

   if (press==0 && MButtonUndetected)
     {
       MButtonPendRelease=5;
       relbnr=bnr;
       return;
     }

  if (HasPTR )
    {

      if (press==0)
	WriteByte(ptrscrdrv+0xaf-0x18,0);

      switch (bnr)
	{
	case 1: 
	  if (press) WriteByte(ptrscrdrv+0xaf-0x18,1);
	  WriteByte(ptrscrdrv+0x8c-0x18,0);
	  break;
	  /*  case 2: hot */
	case 2:
	  if (press)
	    do_hotactn();
	  break;
	case 3: 
	  if (press) WriteByte(ptrscrdrv+0xaf-0x18,2);
	  WriteByte(ptrscrdrv+0x8c-0x18,0);
	  break;
	}
      WriteByte(ptrscrdrv+0x16,ReadByte(ptrscrdrv+0x16)+1); /* int count ?*?*/
    }
}
#endif

#ifdef MOUSE

void MouseTask()
{
  int x,y;
  static int pendreset=0;
  w32 saveA3;

  saveA3=aReg[3];
  aReg[0]=0;

  if (MButtonUndetected>0)
    {
      MButtonUndetected--;
    }

  if (MButtonPendRelease>0)
    {
      MButtonPendRelease--;
      if (MButtonPendRelease==0)
	{
	  QLButton(relbnr,0);
	}
    }
  
  /* lets hope PTR_GEN is the first screen driver extension that gets loaded */
  if (iscrdrv && !ptrscrdrv && iscrdrv!=getSCRdriver())
    {
      reg[3]=0;
      QLtrap(3,0x70,200000l);
      if (((w16)reg[0]) != 0)
	{
	  iscrdrv=getSCRdriver();
	  printf("registered new screen driver at %x\niop.pinf returns %x\n",iscrdrv,(w16)reg[0]);
	}
      else
	{
	  ptrscrdrv=getSCRdriver();
	  if(V3)printf("PTRgen screen driver is %x\n",ptrscrdrv);
	  HasPTR=1;
	}
    }

  paste_into_keyq();

  if (HasPTR && !QLdone)
    {
      x=getMouseX();
      y=getMouseY();
	
      {
	if (!(ptractive())){
	  if (!lastactive)
	    {
	      pendreset=50;
	      lastactive=1;
	    }
	}
	else 
	  {
	    pendreset=0;
	    if (!invisible)
	      {
		if(!script)
		  {
		    invisible=1;
		    //InvisibleMousePtr();
		  }
	      }
	    lastactive=0;
	  }
      }
#ifndef QLMOUSE
      if (pendreset)
	if ( --pendreset<=0 )
	  {
	    if(!script)
	      {
		invisible=0;
	      }
	    
	  }
#endif	  
      if ( (x!=lastx || y!=lasty) && lastx*lasty)
	{

        if (/*inside &&*/ !script){
	    lastx=x;
	    lasty=y;
	    ign_x=x;ign_y=y;
	  }
	  
	}
    }
  aReg[3]=saveA3;
}
#endif

#define DISPLAY_CHANGED() display_changed()
static inline int display_changed()
{
  int i=0;
  
  while (i<sct_size) 
    {
      if(scrModTable[i]) return 1;
      i++;
    }
  
  return 0;
}


#ifdef MOUSE
void QLMovePointer(int pos_x,int pos_y)
{
  if (pos_x == ign_x && pos_y == ign_y)
    {
      ign_x=-1;
      ign_y=-1;
      return;
    }
  if (HasPTR)
    if (pos_x!=lastx || pos_y!=lasty)
      {
	getMouseX();
	getMouseY();
	
	setMouseXY(pos_x,pos_y);

	llastx=lastx; lastx=pos_x;
	llasty=lasty; lasty=pos_y;
      }
}
#endif

void SchedulerCmd()
{
	w32 saved_regs[16];

	if ((long)((Ptr)gPC-(Ptr)theROM)-2 != SCHEDULER_CMD_ADDR)
	{
		exception=4;
		extraFlag=true;
		nInst2=nInst;
		nInst=0;
		return;
	}

	save_regs(saved_regs);

	if (!script)

		if ((schedCount>min_idle ||
				(quickMouseUpdate && ptractive() && (llastx!=getMouseX() || llasty!=getMouseY()))) &&
				/*screen_drawable &&*/
				((DISPLAY_CHANGED()) ||
						(rx1<=rx2 && ry1<=ry2))  )
		{
			scrcnt=5;
			doscreenflush=0;
		}


#ifdef MOUSE
	MouseTask();
#endif

	if (QMD.cpu_hog==0 && schedCount++>min_idle)  /* QDOS running idle */
	{
		if (nInst>5)
		{
			nInst2=nInst;
			nInst=5;
		}
	}
	restore_regs(saved_regs);

	rts();
}


 
