//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#ifndef QLSCREEN_H
#define QLSCREEN_H

typedef struct _SCREEN_SPECS {
	uw32 qm_lo;
	uw32 qm_hi;
	uw32 qm_len;

	uw32 linel;
	int yres;
	int xres;
} screen_specs;

extern screen_specs qlscreen;

void QLPatchPTRENV(void);

#endif
