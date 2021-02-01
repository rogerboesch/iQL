//
// Copyright (c) 1996 jonathan hudson
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"

/* most of the code has moved to QLserio.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <fcntl.h>

#include "QL_68000.h"
#include "QL.h"
#include "QL_instaddr.h"
#include "QL_serial.h"
#include "base_unix.h"
#include "base_cfg.h"

static short defbaud[1] = {9600};

serdev_t *sparams[MAXSERIAL+1];

Cond SetBaudRate(short rate)
{
    short i;
    
    for(i = 1; i < MAXSERIAL+1; i++)
    {
	defbaud[0] = rate;
	if (sparams[i])
	  {
	    (sparams[i])->baud = rate;
	    if((sparams[i])->fd > 0)
	      {
		tty_baud(sparams[i]);
	      }
	  }
    }
    return 1;
}
