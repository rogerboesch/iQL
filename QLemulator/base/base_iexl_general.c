//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#define STATIC static

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "QL_68000.h"
#include "RB_screen.h"

int wait_time_ms = 0;

void debug(char*);
void debug2(char*,long);
extern void DbgInfo(void);
void dosignal(void);

void    (**qlux_table)(void);

#ifdef DEBUG_QL
extern int trace_rts=0;
#endif

#define INLINE inline


uw32 rtop_hard;
int extInt=0;

#ifdef DEBUG_QL
#define TRR  {trace_rts=20;}
#else 
#define TRR
#endif

#define D_ISREG

#include "base_memaccess.h"
#include "base_mmodes.h"

rw32
(*iexl_GetEA[8])(ashort) /*REGP1*/ ={GetEA_mBad,GetEA_mBad,GetEA_m2,GetEA_mBad,GetEA_mBad,
		    GetEA_m5,GetEA_m6,GetEA_m7};
vml rw32 (**ll_GetEA)(ashort) /*REGP1*/ =iexl_GetEA;

rw8
(*iexl_GetFromEA_b[8])(void)={GetFromEA_b_m0,GetFromEA_b_mBad,GetFromEA_b_m2,
			 GetFromEA_b_m3,GetFromEA_b_m4,GetFromEA_b_m5,GetFromEA_b_m6,GetFromEA_b_m7};
vml rw8 (**ll_GetFromEA_b)(void)=iexl_GetFromEA_b;


rw16
(*iexl_GetFromEA_w[8])(void)={GetFromEA_w_m0,GetFromEA_w_m1,GetFromEA_w_m2,
			 GetFromEA_w_m3,GetFromEA_w_m4,GetFromEA_w_m5,GetFromEA_w_m6,GetFromEA_w_m7};
vml rw16 (**ll_GetFromEA_w)(void)=iexl_GetFromEA_w;

rw32
(*iexl_GetFromEA_l[8])(void)={GetFromEA_l_m0,GetFromEA_l_m1,GetFromEA_l_m2,
			 GetFromEA_l_m3,GetFromEA_l_m4,GetFromEA_l_m5,GetFromEA_l_m6,GetFromEA_l_m7};
vml rw32 (**ll_GetFromEA_l)(void)=iexl_GetFromEA_l;

void
(*iexl_PutToEA_b[8])(ashort,aw8) /*REGP2*/ ={PutToEA_b_m0,PutToEA_b_mBad,PutToEA_b_m2,PutToEA_b_m3,PutToEA_b_m4,PutToEA_b_m5,PutToEA_b_m6,PutToEA_b_m7};

vml void (**ll_PutToEA_b)(ashort,aw8) /*REGP2*/ =iexl_PutToEA_b;

void
(*iexl_PutToEA_w[8])(ashort,aw16) /*REGP2*/ ={PutToEA_w_m0,PutToEA_w_m1,PutToEA_w_m2,
			    PutToEA_w_m3,PutToEA_w_m4,PutToEA_w_m5,PutToEA_w_m6,PutToEA_w_m7};

vml void (**ll_PutToEA_w)(ashort,aw16) /*REGP2*/ =iexl_PutToEA_w;

void
(*iexl_PutToEA_l[8])(ashort,aw32) /*REGP2*/ ={PutToEA_l_m0,PutToEA_l_m1,PutToEA_l_m2,
			    PutToEA_l_m3,PutToEA_l_m4,PutToEA_l_m5,PutToEA_l_m6,PutToEA_l_m7};

void (**ll_PutToEA_l)(ashort,aw32) /*REGP2*/ =iexl_PutToEA_l;

Cond
(*ConditionTrue[16])(void)={CondT,CondF,CondHI,CondLS,CondCC,CondCS,CondNE,
			    CondEQ,CondVC,CondVS,CondPL,CondMI,CondGE,CondLT,CondGT,CondLE};
vml Cond (**ll_ConditionTrue)(void)=ConditionTrue;

#ifndef G_reg
w32             reg[16];                        /* registri d0-d7/a0-a7 */
#endif
w32             usp,ssp;                        /* user and system stack
						   pointer (aggiornato solo quello non attivo) */

#ifndef GREGS
uw16    *pc;                            /* program counter : Ptr nella */
gshort    code;
int      nInst;
#endif

Cond    trace,supervisor,xflag,negative,zero,overflow,carry;    /*flags */
char    iMask;                          /* SR interrupt mask */
Cond    stopped;                        /* processor status */
volatile char   pendingInterrupt;       /* interrupt requesting service */

w32      *g_reg;

#ifndef ZEROMAP
w32             *theROM;                        /* Ptr to ROM in Mac memory */
#endif

w32             *ramTop;                        /* Ptr to RAM top in Mac
						   memory */
w32             RTOP;                           /* QL ram top address */
short   exception;                      /* pending exception */
w32             badAddress;                     /* bad address address */
w16             readOrWrite;            /* bad address action */
w32             dummy;                          /* free 4 bytes for who care */
Ptr             dest;                           /* Mac address for
read+write operations */

Cond mea_acc;
w32             lastAddr;                       /* QL address for
						   read+write operations */

volatile Cond   extraFlag;      /* signals exception or trace */


char    dispScreen=0;           /* screen 0 or 1 */
Cond    dispMode=0;                     /* mode 4 or 8 */
Cond    dispActive=true;        /* display is on ? */
Cond    badCodeAddress;

int   nInst2;
extern int script;

volatile w8     intReg=0;
volatile w8     theInt=0;

Cond doTrace;            /* trace after current instruction */

void ProcessInterrupts(void) 
{   
  /* gestione interrupts */
  if( exception==0 && (pendingInterrupt==7 || pendingInterrupt>iMask)
      && !doTrace)
    {    
      if(!supervisor)
	{   
	  usp=(*m68k_sp);
	  (*m68k_sp)=ssp;
	}
      ExceptionIn(24+pendingInterrupt);
      WriteLong((*m68k_sp)-4,(Ptr)pc-(Ptr)theROM);
      (*m68k_sp)-=6;
      WriteWord(*m68k_sp,GetSR());
      SetPCX(24+pendingInterrupt);
      iMask=pendingInterrupt;
      pendingInterrupt=0;
      supervisor=true;
      trace=false;
      stopped=false;
      extraFlag=false;
    }
}

rw16 GetSR(void)
{   
  rw16 sr;
  sr=(w16)iMask<<8;
  if(trace) sr|=0x8000;
  if(supervisor) sr|=0x2000;
  if(xflag) sr|=16;
  if(negative) sr|=8;
  if(zero) sr|=4;
  if(overflow) sr|=2;
  if(carry) sr|=1;
  return sr;
}

void REGP1 PutSR(aw16 sr)
{
  Cond oldSuper;
  oldSuper=supervisor;
  trace=(sr&0x8000)!=0;
  extraFlag=doTrace || trace || exception!=0;
  if(extraFlag)
    {
      nInst2=nInst;
      nInst=0;
    }
  supervisor=(sr&0x2000)!=0;
  xflag=(sr&0x0010)!=0;
  negative=(sr&0x0008)!=0;
  zero=(sr&0x0004)!=0;
  overflow=(sr&0x0002)!=0;
  carry=(sr&0x0001)!=0;
  iMask=(char)(sr>>8)&7;
  if(oldSuper!=supervisor)
    {
      if(supervisor)
	{
	  usp=(*m68k_sp);
	  (*m68k_sp)=ssp;
	}
      else
	{
	  ssp=(*m68k_sp);
	  (*m68k_sp)=usp;
	}
    }
  ProcessInterrupts();
}

rw16 REGP1 BusErrorCode(aw16 dataOrCode)
{
  if(supervisor) dataOrCode+=4;
  return dataOrCode+readOrWrite+8;
}


void REGP1 SetPCX(int i)
{
#ifdef BACKTRACE
  Ptr p=pc;
#endif

  pc=(uw16*)((Ptr)theROM+(RL(&theROM[i])&ADDR_MASK));

#ifdef TRACE
  CheckTrace();
#ifdef BACKTRACE
  AddBackTrace(p,-i);
#endif
#endif

  if(((char)(int)pc&1)!=0)
    {
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      readOrWrite=16;
      badAddress=(Ptr)pc-(Ptr)theROM;
      badCodeAddress=true;
    }
}

#ifdef BACKTRACE
void SetPCB(w32 addr, int type)
{

  Ptr p=pc;


  if(((char)addr&1)!=0)
    {
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      readOrWrite=16;
      badAddress=addr;
      badCodeAddress=true;

      return;
    }

  pc=(uw16*)((Ptr)theROM+(addr&ADDR_MASK));

  CheckTrace();
  AddBackTrace(p,type);
}

#endif

INLINE void REGP1 SetPC(w32 addr)
{
  if(((char)addr&1)!=0)
    {
      exception=3;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      readOrWrite=16;
      badAddress=addr;
      badCodeAddress=true;

      return;
    }

  pc=(uw16*)((Ptr)theROM+(addr&ADDR_MASK));
#ifdef TRACE
  CheckTrace();
#endif
}

#if 1
void ShowException(void){}
#else
void ShowException(void)
{
  short i;
  int p1,p2,p4;
  unsigned char *p3;

  long xc=exception+3;

  if (exception==0) return;


  p1=(xc);
  p2=((Ptr)pc-(Ptr)theROM-(xc==4? 0:2));
  if(xc==4)
    {
      p3="Illegal code=";
      p4=(code);
    }
  else
    {
      p3="";
      if(xc==3) p3="address error";
      if(xc==5) p3="divide by zero";
      if(xc==6) p3="CHK instruction";
      if(xc==7) p3="TRAPV instruction";
      if(xc==8) p3="privilege violation";
      if(xc==9) p3="trace xc";
      if(xc==10) p3="Axxx instruction code";
      if(xc==11) p3="Fxxx instruction code";
      if(xc>35 && xc<48) {p3="TRAP instruction"; p4=xc-35;}
      else p4=0;
    }
  printf("Exception %s %d at PC=%x, xx=%d\n",p3,p1,p2,p4);
}
#endif

extern int tracetrap;

#define UpdateNowRegisters()

void REGP1 ExceptionIn(char x)
{
  if (!tracetrap) return;

  printf("Entering TRAP #%d\n",x-32);
  DbgInfo();
}

void ExceptionOut()
{
  if (!tracetrap) return;

  printf("RTE\n");
  DbgInfo();
}

void ExceptionProcessing()
{
  if(pendingInterrupt!=0 && !doTrace) ProcessInterrupts();
  if(exception!=0)
    {
      if(exception<32 || exception>36) /* tutte le eccezioni
					  tranne le trap 0-4 */
	{
	  extraFlag=exception<3 || (exception>9 &&
				    exception<32) || exception>47;
	  if(!extraFlag) extraFlag=ReadLong(0x28050l)==0;
	  if(extraFlag)
	    {
	      UpdateNowRegisters();
	      /*ShowException(); */
	      nInst=nInst2=0;
	    }
	}
      if(!supervisor)
	{
	  usp=(*m68k_sp);
	  (*m68k_sp)=ssp;
	}
      ExceptionIn(exception);
      (*m68k_sp)-=6;
      WriteLong((*m68k_sp)+2,(w32)pc-(w32)theROM);
      WriteWord((*m68k_sp),GetSR());
      SetPCX(exception);
      if(exception==3) /* address error */
	{
	  (*m68k_sp)-=8;
	  WriteWord((*m68k_sp)+6,code);
	  WriteLong((*m68k_sp)+2,badAddress);
	  WriteWord((*m68k_sp),BusErrorCode(badCodeAddress? 2:1));
	  badCodeAddress=false;
	  if(nInst) exception=0;
	} else exception=0; /* allow interrupts */
      extraFlag=false;
      supervisor=true;
      trace=false;
    }
   if(doTrace)
    {
      if(!supervisor)
	{
	  usp=(*m68k_sp);
	  (*m68k_sp)=ssp;
	}
      ExceptionIn(9);
      (*m68k_sp)-=6;
      WriteLong((*m68k_sp)+2,(Ptr)pc-(Ptr)theROM);
      WriteWord((*m68k_sp),GetSR());
      SetPCX(9);
      if(nInst==0) exception=9;       /* no interrupt allowed here */
      supervisor=true;
      /*doTrace=*/trace=false;
      extraFlag=false;
      stopped=false;
    }
  doTrace=trace;
  if(doTrace) {nInst2=nInst;nInst=1;}
  if(pendingInterrupt!=0 && !doTrace)   /* delay interrupt after trace exception */
    {
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }

}

/******************************************************************/
/* now read in ReadByte etc macros */

rw32 AREGP GetEA_mBad(ashort r)
{
  exception=4;
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
  return 0;
}

void ExecuteLoop(void)  /* fetch and dispatch loop */
{
  while(--nInst>=0)
    {

#ifdef TRACE
      if (pc>tracelo) DoTrace();
#endif

      qlux_table[code=RW(pc++)&0xffff]();
      
        if (wait_time_ms > 0) {
            usleep(wait_time_ms);
        }
    }

    if (rb_AtomicGetPoll()) {
        dosignal();
    }
    
  if(extraFlag)
    {
      nInst=nInst2;
      ExceptionProcessing();
      if(nInst>0) ExecuteLoop();
    }
}

void ExecuteChunk(long n)       /* execute n emulated 68K istructions */
{
  if((long)pc&1) return;

  extraFlag=false;
  ProcessInterrupts();

  if(stopped) return;
  exception=0;

  extraFlag=trace || doTrace || pendingInterrupt==7 ||
    pendingInterrupt>iMask;

  nInst=n+1;
  if(extraFlag)
    {
      nInst2=nInst;
      nInst=0;
    }

  ExecuteLoop();
}

void InitialSetup(void) /* 68K state when powered on */
{
  ssp=*m68k_sp=RL(&theROM[0]);
  SetPC(RL(&theROM[1]));
  if(V3)printf("initial PC=%x SP=%x\n",(void*)pc-(void*)theROM,ssp);
	
  iMask=7;
  supervisor=true;
  trace=doTrace=false;
  exception=0;
  extraFlag=false;
  pendingInterrupt=0;
  stopped=false;
  badCodeAddress=false;
}
