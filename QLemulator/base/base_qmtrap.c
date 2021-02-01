//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#include "QL_68000.h"
#include "QL_config.h"
#include "QL_instaddr.h"
#include "base_proto.h"


int tracetrap=0;

void QMExecuteLoop(uw16 *oldPC)  /* fetch and dispatch loop */
{     
  register void           (**tab)(void);

        tab=qlux_table;

rep:
        while(likely(--nInst>=0 && oldPC!=pc) /* && oldPC!=pc+1 && oldPC!=pc+2 */) 
	  { 
	    tab[code=RW(pc++)&0xffff]();
	  }
	
        if(extraFlag) 
        {       
	  nInst=nInst2;
	  
	  ExceptionProcessing();
	  if(nInst>0) goto rep;
        }
 
	tracetrap=0;
}

void QLchunk(w16 *oldPC,long n)       /* execute n emulated 68K istructions */
{  
  uw16 savePOLLM;
  int save_ninst;
  
  savePOLLM=ReadWord(0x28030);
  WriteWord(0x28030,0);
  
  if((long)pc&1) return;
  
  if(stopped) return;
  if (extraFlag==0) exception=0;

  save_ninst=nInst;
  nInst=n;
  if(extraFlag)
    {      
      nInst2=nInst;

      nInst=0;
    }
  
  QMExecuteLoop(oldPC);

  nInst=save_ninst;
  WriteWord(0x28030,savePOLLM);
}

void QLtrap(int t,int id,int nMax)
{
  reg[0]=id;

  exception=32+t;
  extraFlag=true;
  
  QLchunk(pc,nMax);
}

void QLvector(int which, int nMax)
{
  uw32 ea;
  w16 *savedPC=pc;
  
  ea=ReadWord(which);
  
  WriteLong((*m68k_sp)-=4,(w32)((Ptr)pc-(Ptr)theROM));
  SetPC(ea);

  extraFlag=false;
  exception=0;
  
  QLchunk(savedPC,nMax);
  
}

void QLsubr(uw32 ea, int nMax)
{
  w16 *savedPC=pc;
  
  WriteLong((*m68k_sp)-=4,(w32)((Ptr)pc-(Ptr)theROM));
  SetPC(ea);

  extraFlag=false;
  exception=0;
  
  QLchunk(savedPC,nMax);
}
