//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#include "QL_68000.h"
#include "QL.h"
#include "QL_config.h"
#include "QL_instaddr.h"
#include "RB_screen.h"
#include "base_proto.h"
#include "base_xcodes.h"
#include "QDOS.h"

#define ALT 1
#define SHIFT 4
#define CTRL 2


extern uw32 orig_kbenc;

void rts(void);
void debug2(char*,long);

#define CHAR_BUFF_LEN 50

uw16		asciiChar=0;

int alphaLock, optionKey, gKeyDown, controlKey, shiftKey, altKey;

volatile Cond soundOn;
int display_mode=4;


void SetDisplay(w8 d, Cond flag)
{
  if (d==0) display_mode=4;
  if (d==8) display_mode=8;
  set_rtc_emu();
}

void KillSound(){}
void BeepSound(unsigned char *arg){}

static uw8		IPC_len[16]={0,0,0,0,0,0,0,0,0,1,16,0,0,0,0,0};
static uw8		charBuff[CHAR_BUFF_LEN][2];
static uw16		charAscii[CHAR_BUFF_LEN];
static short	charHead=0;
static short	charTail=0;

static Cond	capsLockStatus=false;
static Cond	pendingCh1Receive=false;
static Cond	pendingCh2Receive=false;
static uw8		IPC_com;
static uw8		IPCW_buff[16];
static short	IPCW_n=0;
static short	IPCW_p=0;
uw8		IPCR_buff[22];
uw16		IPCR_ascii[22];
short	IPCR_n=0;
short	IPCR_p=0;

void ZeroKeyboardBuffer(void)
{	charHead=charTail=0;
	gKeyDown=false;
	asciiChar=0;
}

static uw8 KeyRow(short row)
{
    int mod;

    if(row == 7)
        mod = rb_shiftstate + (rb_altstate << 2) + (rb_controlstate << 1);
    else
        mod=0;

    return rb_keyrow[row] + mod;
}

void pic_set_irq(int irq, int state)
{
}

Cond IPC_Command(void)	/* returns false for commands to handle low-level, true otherwise */
{
	switch(ReadByte(aReg[3])&15){
	case 1:		/* IPC status */
	case 8:		/* read keyboard */
		return false;
	case 9:		/* keyboard direct read */
		reg[1]=KeyRow(ReadByte(aReg[3]+6));
		return true; /*break;*/
	case 10: /* initiate sound generation */
		BeepSound((unsigned char*)theROM+(aReg[3]&0x3ffffe));
		return true; /*break;*/
	case 11: /* kill sound */
		KillSound();
		return true; /*break;*/
	case 15:	/* IPC test *//**//* neu, .hpr 21.5.99 */
		reg[1]=~ReadByte(aReg[3]+6);
		return true;	/**//*break;*/
	default:	/* ignore RS232 and other commands */
	  reg[0]=-15;	/**//* neu, .hpr 21.5.99 */
	    return true; 	/**//*break;*/
	}
	return true;
}

void UseIPC(void)		/* ROM patch: executing IPC command */
{
	if((Ptr)gPC-(Ptr)theROM-2==IPC_CMD_ADDR)
	{	if(IPC_Command()) rts();
		else qlux_table[code=0x40e7]();
	}
	else
	{	exception=4;
		extraFlag=true;
		nInst2=nInst;
		nInst=0;
	}
}

void ReadIPC(void)		/* ROM patch: reading from IPC */
{
	if((Ptr)gPC-(Ptr)theROM-2==IPCR_CMD_ADDR)
	{
	  rts();
	  if(IPCR_n>0)
	    {
	      asciiChar=IPCR_ascii[IPCR_p];
	      
	      reg[1]=IPCR_buff[IPCR_p++];
	      IPCR_n--;
	    }
	  else reg[1]=0;
	}
	else
	  {
	    exception=4;
	    extraFlag=true;
	    nInst2=nInst;
	    nInst=0;
	  }
}

static uw8 qCharLen(void)
{	short n;
	n=charTail-charHead;
	if(n<0) n+=CHAR_BUFF_LEN;
	return (uw8)n;
}

void DoIPCCommand(uw8 IPC_com)
{	uw8 b;

	switch(IPC_com){
	case 1:		/* test status */
		b=0;
		if(charHead!=charTail || gKeyDown) b|=1;
		if(soundOn) b|=2;
		if(pendingCh1Receive) b|=16;
		if(pendingCh2Receive) b|=32;
		*IPCR_buff=b;
		IPCR_n=1;
		break;
	case 8:
		b=qCharLen();
		if(b>7) b=7;
		*IPCR_buff=b;
		if(gKeyDown) (*IPCR_buff)|=8;
		IPCR_n=1;
		while(b--)
		{	IPCR_ascii[IPCR_n]=charAscii[charHead];
			IPCR_buff[IPCR_n++]=charBuff[charHead][0];
			IPCR_ascii[IPCR_n]=charAscii[charHead];
			IPCR_buff[IPCR_n++]=charBuff[charHead++][1];
			
			if(charHead>=CHAR_BUFF_LEN) charHead=0;
		}
		break;
	case 9:
		*IPCR_buff=KeyRow(*IPCW_buff);
		IPCR_n=1;
		break;
	default: IPCR_n=0;
	}
	IPCR_p=0;
}

void WriteIPC(void)		/* ROM patch: writing to IPC */
{
  if((Ptr)gPC-(Ptr)theROM-2==IPCW_CMD_ADDR)
    {
      rts();
      if(IPCW_n--) IPCW_buff[IPCW_p++]=(uw8)(*reg);
      else
	{
	  IPC_com=(short)(*reg)&15;
	  if(IPC_com!=1 && IPC_com!=8) debug2("Bad IPC Command : ",IPC_com);
	  IPCW_n=IPC_len[IPC_com];
	  IPCW_p=0;
	}
      if(!IPCW_n) DoIPCCommand(IPC_com);
    }
  else
    {
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
    }
}

void queueKey(short m,short code,uw16 asciiChar)
{
  charBuff[charTail][1]=code;
  code=0;
  if(m&SHIFT) code|=4;
  if(m&CTRL) code|=2;
  if(m&ALT) code|=1;
  
    charAscii[charTail]=asciiChar;
    charBuff[charTail++][0]=code;
    
    if(charTail>=CHAR_BUFF_LEN) charTail=0;
}

void CheckCapsLock(short m)
{	if(((m&alphaLock)!=0)!=capsLockStatus)
	{	capsLockStatus=!capsLockStatus;
		queueKey(m,33,0);
		if((m&(shiftKey|controlKey|optionKey))!=0) queueKey(0,33,0);	/* per coerenza con il tasto del Mac */
	}
}

void QL_KeyTrans(void)
{	if((Ptr)gPC-(Ptr)theROM-2!=KEYTRANS_CMD_ADDR)
	{	exception=4;
		extraFlag=true;
		nInst2=nInst;
		nInst=0;
		return;
	}
	if(asciiChar)
	{	reg[1]=asciiChar;
		asciiChar=0;
		WriteLong(aReg[7],ReadLong(aReg[7])+4);
		rts();
	}
	else qlux_table[code=KEYTRANS_OCODE]();
}

/* Minerva Keyboard handling */
void KbdCmd(void)   /* do IPC command */
{
  if (reg[0]==9)
    {
      *((char*)reg+4+RBO)=KeyRow(ReadByte(aReg[3]+3));
      WriteLong(aReg[7],ReadLong(aReg[7])+2);
    }
  
  rts();
}

int MButtonDown=0;

void MReadKbd()
{
  int ccode,mod;
  
  while(qCharLen())
    {
      asciiChar=charAscii[charHead];
      ccode=charBuff[charHead][1];
      mod=charBuff[charHead++][0];
      
      if (charHead>=CHAR_BUFF_LEN) charHead=0;

      reg[1]=ccode;
      reg[2]=mod;
      

      aReg[2]=ReadLong(0x2804c);
      QLsubr(ReadWord(0x150)+0x4000,2000000);
    }

  if (gKeyDown)
    {
      aReg[2]=ReadLong(0x2804c);
      reg[5]=-1;/*(gKeyDown!=0)<<4;*/
      
      QLsubr(ReadWord(0x152)+0x4000,2000000);
    }
} 

/* Minerva Keyboard encode routine */
void KBencCmd()
{
  if (asciiChar)
    {
      reg[1]=asciiChar;
      
      WriteLong(aReg[7],ReadLong(aReg[7])+2);
      rts();
    }
  else   /* fall through into original routine */
    {
      pc = (Ptr)theROM+orig_kbenc;
    }
}

