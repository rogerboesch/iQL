//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#ifdef __APPLE__
#include <sys/syslimits.h>
#else
#include <linux/limits.h>
#endif

#include "QL_68000.h"
#include "base_unix.h"
#include "base_proto.h"
#include "base_uxfile.h"
#include "base_emudisk.h"
#include "base_cfg.h"

char* rb_get_resource_path(void);
char* rb_get_system_path(void);

QMDATA QMD = {
	.config_file = "ql.ini",
	.config_file_opt = 0,
	.qdev = qdevs, /* ddvlist */
	.romim = "", /* romlist */
	.ramtop = 4096, /* RAM top */
	.romdir = "roms/", /* rom dir */
	.sysrom = "ql_jm.rom", /* system ROM */
	.ser1 = "", /* ser1 */
	.ser2 = "", /* ser2 */
	.ser3 = "", /* ser3 */
	.ser4 = "", /* ser4 */
	.prtcmd = "lpr", /* print cmd */
	.bootdev = "mdv1", /* boot device */
	.bdi1 = "", /* BDI Unit 1 */
	.cpu_hog = 1, /* CPU hog=true */
	.fastStartup = 1, /* fastStartup=true */
	.skip_boot = 1, /* skip_boot=true */
	.strict_lock = 1, /* do strict locking for disk images */
	.no_patch = 0, /* no_patch, default off */
};

static char *strim(char *s)
{
	char *p;
	short n;
	char *c;

	n = strlen(s);
	p = s + n;
	while (*--p <= ' ' && n--) {
		*p = '\0';
	}
	c = strchr(s, '#');
	if (c)
		*c = 0;
	return s;
}

void CheckDev(EMUDEV_t *qd, char *d1, char *d2, char *d3)
{
	/*DIR *dirp;*/
	short ndev = 0;
	short len = 0;
	short idev = -1;
	short lfree = -1;
	short i;
	char tmp[401];
	int err;

	struct stat sbuf;

	len = strlen(d1);
	if (isdigit(*(d1 + len - 1))) {
		len--;
		ndev = *(d1 + len) - '0';
		*(d1 + len) = 0;
	} else {
		ndev = -1;
	}

	for (i = 0; i < MAXDEV; i++) {
		if ((qd + i)->qname && (strncasecmp(qd[i].qname, d1, len) == 0)) {
			idev = i;
			break;
		}
        else if (qd[i].qname == NULL && lfree == -1) {
			lfree = i;
		}
	}

	if (idev == -1 && lfree == -1) {
		printf("sorry, no more free entries in Directory Device Driver table\n");
		printf("check your ini if you really need all this devices\n");

		return;
	}

	if (idev != -1 && ndev == 0) {
		memset((qd + idev), '\0', sizeof(EMUDEV_t));
	}
    else {
		if (lfree != -1) {
			idev = lfree;
			(qd + idev)->qname = strdup(d1);
		}
        
		if (ndev && ndev < 9) {
			if (d2 && *d2) {
				short dlen;
				char *dnam;

				if (*d2 == '~') {
					d2++;
					strncpy(tmp, getenv("HOME"), 400);
					
                    if (*d2) {
						strncat(tmp, d2, 400);
                    }
				}
                else {
					strncpy(tmp, d2, 400);
                }
                
				dlen = strlen(tmp);
				if (dnam = malloc(dlen + 16)) {
					sprintf(dnam, tmp, getpid());
					dlen = strlen(dnam);
					if (*(dnam + dlen - 1) != '/') {
						dlen++;
					}
                    
					err = stat(dnam, &sbuf);
					if (V1 && err < 0 && strcasecmp((qd + idev)->qname, "ram")) {
                        printf("problem, stat failed\n");
						printf(" - MountPoint %s for device %s%d_ may not be accessible\n", dnam, (qd + idev)->qname, ndev);

					} else {
						if (sbuf.st_mode == S_IFDIR) {
							*(dnam + dlen - 1) =
								'/';
							*(dnam + dlen) = '\0';
						} else {
							/**/
						}
					}

					(qd + idev)->mountPoints[ndev - 1] =
						dnam;
					(qd + idev)->Present[ndev - 1] = 1;
				}
			} else
				(qd + idev)->Present[ndev - 1] = 0;

			if (d3) {
				int flag_set = 0;

				if ((strstr(d3, "native") != NULL) ||
				    (strstr(d3, "qdos-fs") != NULL))
					flag_set |=
						(qd + idev)->Where[ndev - 1] =
							1;
				else if (strstr(d3, "qdos-like") != NULL)
					flag_set |=
						(qd + idev)->Where[ndev - 1] =
							2;

				flag_set |= (qd + idev)->clean[ndev - 1] =
					(strstr(d3, "clean") != NULL);
				if (!flag_set)
					printf("WARNING: flag %s in definition of %s%d_ not recognised\n",
					       d3, d1, ndev);
			}
		}
	}
}

static void ParseDevs(EMUDEV_t **qd, char *s)
{
	char *p1, *p2;
	short n;
	char *d1, *d2, *d3;

	d1 = d2 = d3 = NULL;

	for (p1 = s, n = 0;; n++) {
		short k;
		char c, *p3;

		p2 = strchr(p1, ',');
		if (p2) {
			k = (p2 - p1);
		} else {
			k = strlen(p1);
		}
		p3 = (p1 + k);
		c = *p3;
		*p3 = 0;
		switch (n) {
		case 0:
			d1 = p1;
			break;
		case 1:
			d2 = p1;
			break;
		case 2:
			d3 = p1;
			break;
		}

		if (!(c)) {
			break;
		}
		p1 = p2 + 1;
	}
	if (d1 && *d1) {
        char path[256];
        strcpy(path, rb_get_system_path());
        strcat(path, d2);

		CheckDev(*qd, d1, path, d3);
	}
}

static void *ParseROM(QMLIST *pn, char *s)
{
	char *p1, *p2;
	short n;
	ROMITEM *cp;

	if ((cp = (ROMITEM *)calloc(sizeof(ROMITEM), 1))) {
		pn->next = NULL;
		pn->udata = cp;

		for (p1 = s, n = 0;; n++) {
			short k;
			char c, *p3;

			p2 = strchr(p1, ',');
			if (p2) {
				k = (p2 - p1);
			} else {
				k = strlen(p1);
			}
			p3 = (p1 + k);
			c = *p3;
			*p3 = 0;

			switch (n) {
			case 0:
				cp->romname = (char *)strdup(p1);
				break;
			case 1:
				cp->romaddr = strtol(p1, NULL, 0);
				break;
			}
			if (!(*p3 = c)) {
				break;
			}
			p1 = p2 + 1;
		}
	}
	return cp;
}

static void ParseList(void *p, char *s, void *(*func)(void *, char *))
{
	QMLIST *pn, **ph;

	ph = (QMLIST **)p;
	if ((pn = (QMLIST *)calloc(sizeof(QMLIST), 1))) {
		if ((func)(pn, s) != NULL) {
			if (*ph == NULL) {
				*ph = pn;
			} else {
				QMLIST *pz, *pl;
				for (pl = *ph; pl; pl = pl->next) {
					pz = pl;
				}
				pz->next = pn;
			}
		} else {
			free(pn);
		}
	}
}

static void pString(char *p, char *s, ...)
{
	va_list va;
	int n;

	if (*s) {
		va_start(va, s);
		n = va_arg(va, int);
		strncpy(p, s, n);
		*(p + n) = 0;
	}
}

static void pInt4(long *p, char *s, ...)
{
	*p = strtol(s, NULL, 0);
}

static void pInt2(short *p, char *s, ...)
{
	*p = (short)strtol(s, NULL, 0);
}

static PARSELIST pl[] = {
	{ "DEVICE", (PVFV)ParseDevs, offsetof(QMDATA, qdev) },
	{ "ROMIM", (PVFV)pString, offsetof(QMDATA, romim), 255 },
	{ "PRINT", (PVFV)pString, offsetof(QMDATA, prtcmd), 63 },
	{ "SER1", (PVFV)pString, offsetof(QMDATA, ser1), 63 },
	{ "SER2", (PVFV)pString, offsetof(QMDATA, ser2), 63 },
	{ "SER3", (PVFV)pString, offsetof(QMDATA, ser2), 63 },
	{ "SER4", (PVFV)pString, offsetof(QMDATA, ser2), 63 },
	{ "ROMDIR", (PVFV)pString, offsetof(QMDATA, romdir), 255 },
	{ "SYSROM", (PVFV)pString, offsetof(QMDATA, sysrom), 255 },
	{ "RAMTOP", (PVFV)pInt4, offsetof(QMDATA, ramtop) },
	{ "CPU_HOG", (PVFV)pInt2, offsetof(QMDATA, cpu_hog) },
	{ "FAST_START", (PVFV)pInt2, offsetof(QMDATA, fastStartup) },
	{ "SKIP_BOOT", (PVFV)pInt2, offsetof(QMDATA, skip_boot) },
	{ "STRICT_LOCK", (PVFV)pInt2, offsetof(QMDATA, strict_lock) },
	{ "NO_PATCH", (PVFV)pInt2, offsetof(QMDATA, no_patch) },
	{ "BOOT_DEV", (PVFV)pString, offsetof(QMDATA, bootdev), 4 },
	{ "BDI1", (PVFV)pString, offsetof(QMDATA, bdi1), 63 },
	{ NULL, NULL },
};

FILE *lopen(const char *s, const char *mode)
{
	FILE *fp;
	char fnam[PATH_MAX];

	if (*s == '~') {
		char *p = fnam;
		strcpy(p, getenv("HOME"));
		strcat(p, s + 1);
		s = p;
	}

	fp = fopen(s, mode);
	if (fp == NULL) {
		char pname[512];
		char *pf;

		if ((pf = getenv("HOME"))) {
			short n;

			strcpy(pname, pf);
			n = strlen(pname);
			if (*(pname + n - 1) != '/'
#ifdef __NT__
			    && *(pname + n - 1) != '\\'
#endif
			) {
				*(pname + n) = '/';
				*(pname + n + 1) = '\0';
			}
			strcat(pname, s);
			fp = fopen(pname, mode);
		}
	}

	return fp;
}

void QMParams(void) {
	FILE *fp;
	char *pf;
	int rv = 0, iil;
	QMDATA *p;

    char path[256];
    sprintf(path, "%s%s", rb_get_system_path(), QMD.config_file);
    
	if (!(fp = fopen(path, "r"))) {
        printf("ERROR: did not locate config file %s\n", QMD.config_file);
	}

	printf("Using Config: %s\n", QMD.config_file);

	p = &QMD;

	if (fp) {
		char buff[256];
		char *ptr;
		ptr = (char *)p;

		while (fgets(buff, 256, fp) == buff) {
			char *s;
			PARSELIST *ppl;

			strim(buff);

			for (ppl = pl; ppl->id; ppl++) {
				if (strncasecmp(buff, ppl->id, strlen(ppl->id)) == 0) {
					int l;

					if ((s = strchr(buff, '='))) {
						s++;
						l = strspn(s, " \t");
					}

					if (s) {
						(ppl->func)((ptr + ppl->offset), s+1, ppl->mx.mfun);
						break;
					}
				}
			}
		}
        
		fclose(fp);
	}
    else {
		printf("Warning: could not find %s\n", pf);
    }
}
