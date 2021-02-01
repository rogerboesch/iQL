//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

/* script redirection IO */

#include <fcntl.h>
#include <unistd.h>

#include "QL_68000.h"
#include "base_util.h"
#include "base_script.h"
#include "base_proto.h"
#include "QDOS.h"

int script_read_enable=0;

int script_read(void *p, void *buf, int len)
{
  int res;

  if (!script_read_enable)
    return 0;

  res=read(0,buf,len);
  if (res<0) res=qmaperr();
  return res;
}
int script_write(void *p, void * buf, int len)
{
  int res;

  res=write(1,buf,len);

  if (res<0) res=qmaperr();
  return res;
}
int script_pend(void *p)
{
  if (!script_read_enable)
    return QERR_NC;

  if (check_pend(0,SLC_READ)) return 0;
  else return QERR_NC;
}

