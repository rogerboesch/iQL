//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#include <string.h>

#include "QL_68000.h"
#include "QL.h"
#include "QL_serial.h"
#include "QL_instaddr.h"
#include "QL_screen.h"
#include "base_boot.h"
#include "base_proto.h"
#include "QDOS.h"

extern int HasPTR;


/* external debugging aid */
/* replaced by gdb's "qldbg" command */
#if 0
void qldbg(void)
{
  exception=32+14;
  extraFlag=true;
  nInst2=nInst;
  nInst=0; 
}
#endif

void trap0(void)
{
  exception=32+(code&15);
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
}

void trap1(void)
{	short op;

	op=(short)(*reg)&0x7f;

	/*	if ((op>3 && op<6) || (op>7 && op<0xc))*/
	  DECR_SC(); /*schedCount--;*/
		
	if(op>=0x11 && op<=0x15)	/* Gestione dell'hardware */
	{	switch(op){
		case 0x11:	/* IPC command */
			*reg=0;	/**//* neu, .hpr 21.5.99 */
			if(!IPC_Command()) goto doTrap;
			break;
		case 0x12:			/* set baud rate */
			*reg=SetBaudRate((short)reg[1])? 0:-15;
			break;  
		case 0x15:			/* adjust clock */
			if(!reg[1]) break;
			qlClock=(uw32)reg[1];
		case 0x13:			/* read clock */
			GetDateTime((uw32*)(reg+1));
			*reg=0;
			prep_rtc_emu();
			break;
		case 0x14:			/* set clock */
		  {
		        w32 i;
		    
			GetDateTime(&i);
			qlClock-=i-(uw32)reg[1];
			*reg=0;
			break;
		  }
		}
	}
	else
	{
	    {
	    doTrap:	exception=33;
	    extraFlag=true;
	    nInst2=nInst;
	    nInst=0;
	    }
	}
}

void trap2(void)
{
  DECR_SC();
  exception=34;
  extraFlag=true;
  nInst2=nInst;
  nInst=0;
}



void trap3(void)
{
    DECR_SC();

    if (!HasPTR && *reg==0x70)
        QLPatchPTRENV();

    exception=35;
    extraFlag=true;
    nInst2=nInst;
    nInst=0;
}

extern int script_read_enable;

void btrap3(void)
{
  DECR_SC();
  
  if (((w8)reg[0])==1)
    {
      *((char*)reg+4+RBO)= BOOT_SELECT;
      reg[0]=0;
      qlux_table[code]=trap3;
      script_read_enable=1;
    }
  else trap3();
}

void FastStartup(void)
{	
  if((Ptr)gPC-(Ptr)theROM-2!=RL(&theROM[1]))
    {
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      return;
    }

  memset((Ptr)theROM+131072l,0,RTOP-131072l);

  while(RL((w32*)gPC)!=0x28000l) gPC++;
  gPC-=4;
  aReg[5]=RTOP;
}
