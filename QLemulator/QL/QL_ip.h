//
// QLIP (c) 1998 Jonathan Hudson & Richard Zidlicky
// This code is part of the uqlx QDOS emulator for Unix
//
// This file is outside the uqlx copyright and may be freely copied,
// used and modified for any non-commercial purpose.
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"

#ifdef IPDEV

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct
{
  int status;  /* 0 OK, -1 check connection(async), -2 error */
  int lerrno;
  int sock;
  struct sockaddr_in name;
} ipdev_t;


int ip_init(int,void *);
int ip_test(int,char *);
int ip_open(int, void **);
void ip_close(int , void *);
void ip_io(int , void *);


#endif
