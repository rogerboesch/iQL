//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#include "QL_68000.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <time.h>

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "base_xcodes.h"
#include "QL_config.h"
#include "QL_instaddr.h"

#include "base_unix.h"
#include "base_boot.h"
#include "base_proto.h"
#include "QL_sound.h"
#include "base_uxfile.h"
#include "QL_screen.h"
#include "RB_screen.h"

extern char* rb_get_system_path(void);
extern char* rb_get_system_rom(void);
extern char* rb_get_toolkit_rom(void);
extern char* rb_get_device_win_path(void);
extern char* rb_get_device_mdv1_path(void);
extern char* rb_get_device_mdv2_path(void);
extern char* rb_get_device_printer(void);
extern char* rb_get_printer_device(void);
extern int rb_get_ramtop(void);
extern int rb_get_fast_startup(void);
extern int rb_get_cpu_hog(void);
extern int rb_get_color(void);

extern void ql_set_title(char* title);

#define TIME_DIFF 283996800
void GetDateTime(w32 *);
long long qlClock = 0;

#ifdef VTIME
int qlttc = 50;
int qltime = 0;
int qitc = 10;
#endif

int ux_boot = 0;
int ux_bfd = 0;
char *ux_bname = "";
int start_iconic = 0;
char *pwindow = NULL;
int is_patching = 0;

extern uw32 rtop_hard;
extern int screen_drawable;
extern int wait_time_ms;

int do_update = 0; /* initial delay for screen drawing */

#define min(_a_, _b_) (_a_ < _b_ ? _a_ : _b_)
#define max(_a_, _b_) (_a_ > _b_ ? _a_ : _b_)

int rx1, rx2, ry1, ry2, finishflag, doscreenflush;
int QLdone = 0;
int QLrestart = 0;

extern void FlushDisplay(void);

extern void DbgInfo(void);

void btrap3(void);

extern void SchedulerCmd(void);
extern void KbdCmd(void);

extern void process_events(void);

int script = 0;
int redir_std = 0;

int scrcnt = 0;

#ifdef VTIME
volatile poll_req = 0; /* debug only */
#endif

int noints = 0;
int schedCount = 0;
extern int min_idle;

long pagesize = 4096;
int pageshift = 12;

void cleanup_dialog()
{
}

int rtc_emu_on = 0;
void prep_rtc_emu()
{
}

void set_rtc_emu()
{
}

/* Read a system variable word sized from QDOS memory    */
/* NOTE(TF): Apparently not used anywhere in the code!   */
uw16 sysvar_w(uw32 a)
{
	return RW((Ptr)theROM + 0x28000 + a);
}

/* Read a system variable (long) from QDOS memory        */
uw32 sysvar_l(uw32 a)
{
	return RL((Ptr)theROM + 0x28000 + a);
}

#ifdef SHOWINTS
static long alrm_count = 0;
static long a_ticks = 0;
static long aa_cnt = 0;
#endif

static int flptest = 0;

void dosignal()
{
    rb_AtomicSetPoll(0);
#ifdef SOUND
	if (delay_list)
		qm_sound();
#endif

    if (!script && !QLdone) {
		QLRBProcessEvents();
    }

#ifdef SHOWINTS
	aa_cnt += alrm_count;
	alrm_count = 0;
	if (++a_ticks > 49) {
		printf("received %d alarm signals, processed %d\n", aa_cnt,
		       a_ticks);
		a_ticks = aa_cnt = 0;
	}
#endif

	if (--scrcnt < 0) {
		doscreenflush = 1;
		set_rtc_emu();
	}

	if (flptest++ > 25) {
		flptest = 0;
		TestCloseDevs();
		// FIXME: CHECK Impact process_ipc();
	}

#ifndef xx_VTIME
	FrameInt();
#endif
}

extern int xbreak;
void cleanup(int err) {
	//cleanup_ipc();
	CleanRAMDev("RAM");
	QLRBExit();
}

void signalTimer() {
#ifdef VTIME
	poll_req++;
#endif
	schedCount = 0;
#ifdef SHOWINTS
	alrm_count++;
#endif
}

void ontsignal(int sig)
{
    // FIXME: Check
	/*set_rtc_emu();*/ /* .. not yet working */
	//signalTimer ();
}

/* rather crude but reliable */
static int fat_int_called = 0;

void on_fat_int(int x)
{
    if (fat_int_called == 1) {
        printf("ERROR CODE: %d\n", 45);
    }
	if (fat_int_called > 1)
		raise(9);

    fat_int_called++;

	alarm(0);

    printf("Terminate on signal %d\n", x);
	printf("This may be due to an internal error,\n"
	       "a feature not emulated or an 68000 exception\n"
	       "that typically occurs only when QDOS is unrecoverably\n"
	       "out of control\n");
	dbginfo("FATAL error, PC may not be displayed correctly\n");
	cleanup(44);
}

#ifdef UX_WAIT
#include <sys/wait.h>
struct cleanup_entry {
	void (*cleanup)();
	unsigned long int id;
	pid_t pid;
	struct cleanup_entry *next;
};

static struct cleanup_entry *cleanup_list = NULL;

static int qm_wait(fc) int *fc;
{
	int pid;

	pid = wait3(fc, WNOHANG, (struct rusage *)NULL);
	return pid;
}

/* exactly like fork but registers cleanup handler */
int qm_fork(void (*cleanup)(), unsigned long id)
{
	struct cleanup_entry *ce;
	int pid;

	pid = fork();
	if (pid > 0) {
		ce = (void *)malloc(sizeof(struct cleanup_entry));
		ce->pid = pid;
		ce->id = id;
		ce->cleanup = cleanup;
		ce->next = cleanup_list;
		cleanup_list = ce;
	}
	return pid;
}

static void qm_reaper()
{
	struct cleanup_entry *ce, **last;
	int pid, found;
	int failcode;

	run_reaper = 0;
	while ((pid = qm_wait(&failcode)) > 0) {
		ce = cleanup_list;
		last = &cleanup_list;
		found = 0;
		while (ce) {
			if (pid == ce->pid) {
				*last = ce->next;
				(*(ce->cleanup))(ce->pid, ce->id, failcode);
				free(ce);
				found = 1;
				break;
			}
			last = &(ce->next);
			ce = ce->next;
		}
		if (!found)
			printf("hm, pid %d not found in cleanup list?\n", pid);
	}
}
#else
int qm_fork(void (*cleanup)(void), unsigned long id) { return 0; }
#endif

void init_signals()
{
}

int load_rom(char *, w32);

void ChangedMemory(int from, int to)
{
	int i;
	uw32 dto, dfrom;

	/* QL screen memory involved? */
	if ((from >= qlscreen.qm_lo && from <= qlscreen.qm_hi) ||
	    (to >= qlscreen.qm_lo && to <= qlscreen.qm_hi)) {
		dfrom = max(qlscreen.qm_lo, from);
		dto = min(qlscreen.qm_hi, to);

		for (i = 0; i < sct_size; i++)
			scrModTable[i] =
				(i * pagesize + qlscreen.qm_lo <= dto &&
				 i * pagesize + qlscreen.qm_lo >= dfrom);
	}
}

char **argv;
int argc;

void DbgInfo(void)
{
	int i;

	/* "ssp" is ssp *before* sv-mode was entered (if active now) */
	/* USP is saved value of a7 or meaningless if not in sv-mode */
	printf("DebugInfo: PC=%lx, code=%x, SupervisorMode: %s USP=%x SSp=%x A7=%x\n",
	       (Ptr)pc - (Ptr)theROM, code, (supervisor ? "yes" : "no"), usp,
	       ssp, *m68k_sp);
	printf("Register Dump:\t Dn\t\tAn\n");
	for (i = 0; i < 8; i++)
		printf("%d\t\t%8x\t%8x\n", i, reg[i], aReg[i]);
}

long uqlx_tz;

long ux2qltime(long t)
{
	return t + TIME_DIFF + uqlx_tz;
}

long ql2uxtime(long t)
{
	return t - TIME_DIFF - uqlx_tz;
}

void GetDateTime(w32 *t)
{
	struct timeval tp;

#ifndef VTIME

	gettimeofday(&tp, (void *)0);
	*t = ux2qltime(tp.tv_sec) + qlClock;
	;
#else
	*t = qltime;
#endif
}

int rombreak = 0;

int allow_rom_break(int flag)
{
	if (flag < 0)
		return rombreak;

	if (flag) {
		rombreak = 1;
	} else {
		rombreak = 0;
	}
	return rombreak;
}

void init_uqlx_tz()
{
	struct tm ltime;
	struct tm gtime;
	time_t ut;

	ut = time(NULL);
	ltime = *localtime(&ut);
	gtime = *gmtime(&ut);

	gtime.tm_isdst = ltime.tm_isdst;
	uqlx_tz = mktime(&ltime) - mktime(&gtime);
}

w32 ReadQlClock(void)
{
	w32 t;

	GetDateTime(&t);
	return t;
}

int impopen(char *name, int flg, int mode)
{
	char buff[PATH_MAX], *p;
	int r, md;

	md = mode;

	if ((r = open(name, flg, md)) != -1)
		return r;

	if (*name == '~') {
		char *p = buff;
		strcpy(p, getenv("HOME"));
		strcat(p, name + 1);
		name = p;
	}

	return open(name, flg, md);

	//strcpy(buff,IMPL);
	p = buff + strlen(buff);
	if (*(p - 1) != '/')
		strcat(buff, "/");
	strncat(buff, name, PATH_MAX);

	return open(buff, flg, md);
}

int load_rom(char *name, w32 addr)
{
	struct stat b;
	int r;
	int fd;

    char path[256];
    sprintf(path, "%s%s", rb_get_system_path(), name);
	fd = impopen(path, O_RDONLY, 0);
    
	if (fd < 0) {
		perror("Warning: could not find ROM image ");
		printf(" - rom name %s\n", name);
		return 0;
	}

	fstat(fd, &b);
	if (b.st_size != 16384 && addr != 0)
		printf("Warning: ROM size of 16K expected, %s is %d\n", name,
		       (int)b.st_size);
	if (addr & 16383)
		printf("Warning: addr %x for ROM %s not multiple of 16K\n",
		       addr, name);

	r = read(fd, (Ptr)theROM + addr, b.st_size);
	if (r < 0) {
		perror("Warning, could not load ROM \n");
		printf("name %s, addr %x, QDOS origin %p\n", name, addr,
		       theROM);
		return 0;
	}
	if (V3)
		printf("loaded %s \t\tat %x\n", name, addr);
	close(fd);

	return r;
}

int scr_planes = 2;
int scr_width, scr_height;

int verbose = 2;

extern int shmflag;

#ifndef XSCREEN
void parse_screen(char *x)
{
	printf("sorry, '-g' option works only with XSCREEN enabled,\ncheck your Makefile\n");
}
#endif

int sct_size;
char *oldscr;

static char obuf[BUFSIZ];

void CoreDump()
{
	int fd, r;

	fd = open("qlcore", O_RDWR | O_CREAT, 0644);
	if (fd < 0)
		perror("coredump failed: read: :");
	if (fd > -1) {
		r = write(fd, theROM, 1024 * 1024);
		if (!r)
			perror("coredump failed: write: ");
		close(fd);
		if (r)
			printf("memory dump saved as qlcore\n");
	}
}

char *IQL_VERSION = "0.60";

#include "base_cfg.h"

int toggle_hog(int val) {
	if (val < 0)
		return QMD.cpu_hog;
    
	QMD.cpu_hog = val;
	return QMD.cpu_hog;
}

// MARK: - Emulator functions (internal)

static void QLApplyParam(void) {
    // Replaces ini files
    strcpy(&(QMD.sysrom[0]), rb_get_system_rom());
    strcpy(&(QMD.romim[0]), rb_get_toolkit_rom());
}

static void QLSetParams(void) {
	char sysrom[200];
	int mem = -1, hog = -1, no_patch = -1;

	setvbuf(stdout, obuf, _IOLBF, BUFSIZ);

	*sysrom = 0;

    //QLApplyParam();
    QMParams();

	if (mem > 0 && mem < 17)
		mem = mem * 1024;

	if (mem != -1)
		QMD.ramtop = mem;
	if (hog != -1)
		QMD.cpu_hog = 1;
	if (no_patch != -1)
		QMD.no_patch = 1;
	if (QMD.no_patch)
		do_update = 1;

    do_update = 1;
	toggle_hog(QMD.cpu_hog);

	RTOP = QMD.ramtop * 1024;
}

static int QLInit() {
	int rl = 0;
	void *tbuff;

	rx1 = 0;
	rx2 = qlscreen.xres - 1;
	ry1 = 0;
	ry2 = qlscreen.yres - 1;
	finishflag = 0;

    char temp[256];
    sprintf(temp, "iQL V%s - QL Emulator (%s)", IQL_VERSION, IQL_RELEASE);
    ql_set_title(temp);

    tzset();

	theROM = malloc(RTOP);
	if (theROM == NULL) {
		printf("sorry, not enough memory for a %dK QL\n", RTOP / 1024);
		return -1;
	}

	tbuff = malloc(65536 * sizeof(void *));
	{
		char roms[PATH_MAX];
		char *p = NULL;
		
        if (!rl) {
			p = (char *)stpcpy(roms, QMD.romdir);

            if (*(p - 1) != '/') {
				*p++ = '/';
			}
			
            strcpy(p, QMD.sysrom);

			rl = load_rom(roms, (w32)0);
			
            if (!rl) {
				printf("Could not find qdos ROM image\n");
                QLdone = 1;
				return -2;
			}
		}

        if (strlen(QMD.romim)) {
			p = (char *)stpcpy(roms, QMD.romdir);
		
            if (*(p - 1) != '/') {
				*p++ = '/';
			}
			strcpy(p, QMD.romim);

			rl = load_rom(roms, 0xC000);
			if (!rl) {
				printf("Could not find expansion rom, exiting\n");
                QLdone = 1;
                return -3;
			}
		}
	}

	init_uqlx_tz();
	init_signals();
	init_iso();

	LoadMainRom(); /* patch QDOS ROM*/

	/* Minerva cannot handle more than 16M of memory... */
    if (isMinerva && RTOP > 16384 * 1024) {
		RTOP = 16384 * 1024;
    }
    
    /* ...everything else not more than 4M */
    if (!isMinerva && RTOP > 4096 * 1024) {
		RTOP = 4096 * 1024;
    }
    
	rtop_hard = RTOP;

	if (isMinerva) {
		qlscreen.xres = qlscreen.xres & (~(7));
		qlscreen.linel = qlscreen.xres / 4;
		qlscreen.qm_len = qlscreen.linel * qlscreen.yres;

		qlscreen.qm_lo = 128 * 1024;
		qlscreen.qm_hi = 128 * 1024 + qlscreen.qm_len;
        
		if (qlscreen.qm_len > 0x8000) {
			if (((long)RTOP - qlscreen.qm_len) <
			    256 * 1024 + 8192) {
				printf("Sorry, not enough RAM for this screen size\n");
				goto bsfb;
			}

            qlscreen.qm_lo = ((RTOP - qlscreen.qm_len) >> 15) << 15; /* RTOP MUST BE 32K aligned.. */
			qlscreen.qm_hi = qlscreen.qm_lo + qlscreen.qm_len;
			RTOP = qlscreen.qm_lo;
		}
	}
    else {
	bsfb:
		qlscreen.linel = 128;
		qlscreen.yres = 256;
		qlscreen.xres = 512;

		qlscreen.qm_lo = 128 * 1024;
		qlscreen.qm_hi = 128 * 1024 + 32 * 1024;
		qlscreen.qm_len = 0x8000;
	}

    scr_width = qlscreen.xres;
	scr_height = qlscreen.yres;

    if (!script) {
		QLRBScreen();
    }
    
#ifdef TRACE
	TraceInit();
#endif
	EmulatorTable(tbuff);
#ifdef G_reg
	reg = _reg;
#endif
    
	if (!isMinerva) {
		qlux_table[IPC_CMD_CODE] = UseIPC; /* installl pseudoops */
		qlux_table[IPCR_CMD_CODE] = ReadIPC;
		qlux_table[IPCW_CMD_CODE] = WriteIPC;
		qlux_table[KEYTRANS_CMD_CODE] = QL_KeyTrans;
        qlux_table[FSTART_CMD_CODE] = FastStartup;
	}

    qlux_table[ROMINIT_CMD_CODE] = InitROM;
	qlux_table[MDVIO_CMD_CODE] = MdvIO;
	qlux_table[MDVO_CMD_CODE] = MdvOpen;
	qlux_table[MDVC_CMD_CODE] = MdvClose;
	qlux_table[MDVSL_CMD_CODE] = MdvSlaving;
	qlux_table[MDVFO_CMD_CODE] = MdvFormat;
	qlux_table[POLL_CMD_CODE] = PollCmd;

#ifdef SERIAL
#ifndef NEWSERIAL
	qlux_table[OSERIO_CMD_CODE] = SerIO;
	qlux_table[OSERO_CMD_CODE] = SerOpen;
	qlux_table[OSERC_CMD_CODE] = SerClose;
#endif
#endif

	qlux_table[SCHEDULER_CMD_CODE] = SchedulerCmd;
	if (isMinerva) {
		qlux_table[MIPC_CMD_CODE] = KbdCmd;
		qlux_table[KBENC_CMD_CODE] = KBencCmd;
	}
	qlux_table[BASEXT_CMD_CODE] = BASEXTCmd;

	if (QMD.skip_boot)
		qlux_table[0x4e43] = btrap3;

	g_reg = reg;

	InitialSetup();

	if (isMinerva) {
		reg[1] = RTOP & ((~16383) | 1 | 2 | 4 | 16);
		SetPC(0x186);
	}

	QLdone = 0;
    return 1;
}

static int QLStep(void) {
    ExecuteChunk(3000);
    return QLdone;
}

static void QLCleanUp() {
    cleanup(0);
}

static void QLLoop() {
    QLrestart = 0;
    
    while (!QLdone) {
        QLStep();
        
        if (QLrestart) {
            QLSetParams();
            
            if (QLInit() != 1) {
                return;
            }

            QLrestart = 0;
        }
    }
}

// MARK: - Emulator functions (public)

void QLTimer() {
    rb_AtomicSetPoll(1);
    schedCount = 0;
}

int QLStart() {
    QLSetParams();
    if (!QLInit()) {
        return 0;
    }
    
    QLCleanUp();
    QLLoop();
    
    return 1;
}

void QLRestart() {
    wait_time_ms = 0;
    QLrestart = 1;
}

void QLStop() {
    QLdone = 1;
}

void QLSetSpeed(int ms) {
    wait_time_ms = ms;
}

