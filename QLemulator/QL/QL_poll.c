//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

/* hook for QDOS poll routine */

#include "QL_68000.h"
#include "QL.h"
#include "QL_instaddr.h"
#include "base_xcodes.h"
#include "base_unix.h"
#include "base_proto.h"
#include "QDOS.h"

extern volatile int poll_req;


void init_poll()
{
  reg[1]=0x10;
  reg[2]=0;
  QLtrap(1,0x18,2000000l);
  
  if (*reg==0)
    {
      Ptr p=(Ptr)theROM+aReg[0];
      p = p + 4;
      
      WL( p, POLL_CMD_ADDR);
      WW((Ptr)theROM+POLL_CMD_ADDR, POLL_CMD_CODE);
      
      QLtrap(1,0x1c,200000l);
    }
}


void PollCmd()
{
  if((Ptr)gPC-(Ptr)theROM-2!=POLL_CMD_ADDR)
    {
      exception=4;
      extraFlag=true;
      nInst2=nInst;
      nInst=0;
      return;
    }

#ifdef VTIME
  if (poll_req>1)
    printf("poll_req=%d in PollCmd()\n",poll_req);
  poll_req=0;
#endif
  
  if (isMinerva)
    MReadKbd();

  rts();
}


