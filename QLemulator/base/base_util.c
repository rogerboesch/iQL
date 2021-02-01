//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

/* some miscelaneous utility fucntions */

#include "QL_68000.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#ifdef IPDEV
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <sys/ioctl.h>

#include "QL_serial.h"
#include "base_driver.h"
#include "base_cfg.h"
#include "base_util.h"
#include "QDOS.h"

int check_pend(int fd,int mode) 
{
  struct timeval tv;
  fd_set wfd,errfd,rfd,*xx;
  int res;

  switch(mode)
    {
    case SLC_READ: xx=&rfd;;
      break;
    case SLC_WRITE: xx=&wfd;
      break;
    case SLC_ERR: xx=&errfd;
      break;
    default : printf("wrong mode for check_pend: %d\n",mode);
      return 0;     
    }
  
  tv.tv_sec=0;
  tv.tv_usec=0;
  
  FD_ZERO(&wfd);
  FD_ZERO(&errfd);
  FD_ZERO(&rfd);

  FD_SET(fd,xx);
  

  res=select(fd+1,&rfd,&wfd,&errfd,&tv);
  
  if (res<0) return 0;
  return (res>0);
}
