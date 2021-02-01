//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#ifndef _qm_parse_h
#define _qm_parse_h

#include <stdio.h>
#ifdef __APPLE__
#include <sys/syslimits.h>
#else
#include <linux/limits.h>
#endif

#include "base_emudisk.h"

typedef struct _qmlist {
	struct _qmlist *next;
	void *udata;
} QMLIST;

typedef struct _rominfo {
	char *romname;
	long romaddr;
} ROMITEM;

typedef struct {
	char config_file[PATH_MAX + 1];
	char config_file_opt;
	EMUDEV_t *qdev;
	long ramtop;
	char romdir[256];
	char sysrom[256];
	char romim[256];
	char ser1[64];
	char ser2[64];
	char ser3[64];
	char ser4[64];
	char prtcmd[64];
	char bootdev[5];
	char bdi1[64];
	short cpu_hog;
	short fastStartup;
	short skip_boot;
	short strict_lock;
	short no_patch;
} QMDATA;

extern FILE *lopen(const char *s, const char *mode);
extern char *ExpandName(char *);

#include <stddef.h>

typedef void (*PVFV)(void *, void *, ...);

typedef union {
	int mval;
	void *(*mfun)(QMLIST *, char *);
} uxt;

typedef struct {
	char *id;
	PVFV func;
	long offset;
	uxt mx;
} PARSELIST;

#ifndef PATH_MAX
#include <limits.h>
#endif

extern QMDATA QMD;

#endif
