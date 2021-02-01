//
//  RB_screen.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>

#include "QL_hardware.h"
#include "QL_68000.h"
#include "QL_mouse.h"
#include "QL_screen.h"
#include "rb_screen.h"
#include "rb_virtual_keys.h"
#include "base_cfg.h"
#include "base_proto.h"

extern void ql_render_screen(void* buffer);

void* pixel_buffer = NULL;

typedef struct QLRect_ {
    int x;
    int y;
    int w;
    int h;
} QLRect;

static QLRect dest_rect;

struct QLcolor {
	int r;
	int g;
	int b;
};

struct QLcolor QLcolors[8] = {
	{ 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xFF }, { 0xFF, 0x00, 0x00 },
	{ 0xFF, 0x00, 0xFF }, { 0x00, 0xFF, 0x00 }, { 0x00, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0x00 }, { 0xFF, 0xFF, 0xFF },
};

uint32_t iQLcolors[8] = {4278190080,4294901760,4278190335,4294902015,4278255360,4294967040,4278255615,4294967295};

static void QLRBSetDestRect(int w, int h) {
	float target_aspect = (float)qlscreen.xres / (float)qlscreen.yres;
	float window_aspect = (float)w / (float)h;
	float scale;

	if (target_aspect > window_aspect) {
		scale = (float)w / (float)qlscreen.xres;
		dest_rect.x = 0;
		dest_rect.y = ceilf((float)h - (float)qlscreen.yres * scale) / 2.0;
		dest_rect.w = w;
		dest_rect.h = ceilf((float)qlscreen.yres * scale);
	}
    else {
		scale = (float)h / (float)qlscreen.yres;
		dest_rect.x = ceilf((float)w - ((float)qlscreen.xres * scale)) / 2.0;
		dest_rect.y = 0;
		dest_rect.w = ceilf((float)qlscreen.xres * scale);
		dest_rect.h = h;
	}
}

void QLRBScreen(void) {
    // FIXME: Don't hard code
    QLRBSetDestRect(512, 256);

    pixel_buffer = (void *)malloc(qlscreen.xres * qlscreen.yres * 4);

	rb_AtomicSetPoll(0);
}

void QLRBUpdateScreenByte(uint32_t offset, uint8_t data) {
	int t1, t2, i, color;
	uint16_t dataword = 0;

	if (offset & 1) {
		offset--;
        
		dataword = (uint8_t)ReadByte(qlscreen.qm_lo + offset) << 8;
		dataword |= data & 0xFF;
	}
    else {
		dataword = (uint16_t)data << 8;
		dataword |= (uint8_t)ReadByte((qlscreen.qm_lo + offset) + 1) & 0xFF;
	}

	QLRBUpdateScreenWord(offset, dataword);
}

void QLRBUpdateScreenWord(uint32_t offset, uint16_t data) {
	int t1, t2, i, color;
	uint32_t *pixel_ptr32;

	t1 = data >> 8;
	t2 = data & 0xFF;

    pixel_ptr32 = pixel_buffer + (offset * 16);

	if (display_mode == 8) {
		for (i = 0; i < 8; i += 2) {
			uint32_t x;

			color = ((t1 & 2) << 1) + ((t2 & 3)) + ((t1 & 1) << 3);

			x = iQLcolors[color];

			*(pixel_ptr32 + 7 - (i)) = x;
			*(pixel_ptr32 + 7 - (i + 1)) = x;

			t1 >>= 2;
			t2 >>= 2;
		}
	}
    else {
		for (i = 0; i < 8; i++) {
			uint32_t x;

			color = ((t1 & 1) << 2) + ((t2 & 1) << 1) + ((t1 & 1) & (t2 & 1));

			x = iQLcolors[color];

			*(pixel_ptr32 + 7 - i) = x;

			t1 >>= 1;
			t2 >>= 1;
		}
	}

    rb_AtomicSetScreenUpdate(1);
}

void QLRBUpdateScreenLong(uint32_t offset, uint32_t data) {
	QLRBUpdateScreenWord(offset, data >> 16);
	QLRBUpdateScreenWord(offset + 2, data & 0xFFFF);
}

void QLRBUpdatePixelBuffer() {
	uint8_t *scr_ptr = (void *)theROM + qlscreen.qm_lo;
	uint32_t *pixel_ptr32;
	int t1, t2, i, color;

    pixel_ptr32 = pixel_buffer;

	while (scr_ptr < (uint8_t *)((void *)theROM + qlscreen.qm_lo + qlscreen.qm_len)) {
		t1 = *scr_ptr++;
		t2 = *scr_ptr++;

		if (display_mode == 8) {
			for (i = 0; i < 8; i += 2) {
				uint32_t x;

				color = ((t1 & 2) << 1) + ((t2 & 3)) +
					((t1 & 1) << 3);

				x = iQLcolors[color];

				*(pixel_ptr32 + 7 - (i)) = x;
				*(pixel_ptr32 + 7 - (i + 1)) = x;

				t1 >>= 2;
				t2 >>= 2;
			}
		}
        else {
			for (i = 0; i < 8; i++) {
				uint32_t x;

				color = ((t1 & 1) << 2) + ((t2 & 1) << 1) + ((t1 & 1) & (t2 & 1));

				x = iQLcolors[color];

				*(pixel_ptr32 + 7 - i) = x;

				t1 >>= 1;
				t2 >>= 1;
			}
		}
		pixel_ptr32 += 8;
	}
}

void QLRBRenderScreen(void) {
    if (!rb_AtomicGetScreenUpdate())
		return;

    ql_render_screen(pixel_buffer);
    
    rb_AtomicSetScreenUpdate(0);
}

/* Store the keys pressed */
unsigned int rb_keyrow[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int rb_shiftstate, rb_controlstate, rb_altstate;

static void RBQLKeyrowChg(int code, int press) {
	int j;

	if (code > -1) {
		j = 1 << (code % 8);

		if (press)
			rb_keyrow[7 - code / 8] |= j;
		else
			rb_keyrow[7 - code / 8] &= ~j;
	}
}

struct RBQLMap {
    RBVirtualKey keycode;
	int code;
	int qchar;
};

static struct RBQLMap qlmap[] = {
    { RBVK_Left, 49, 0 },
    { RBVK_Up, 50, 0 },
    { RBVK_Right, 52, 0 },
    { RBVK_Down, 55, 0 },

    { RBVK_F1, 57, 0 },
    { RBVK_F2, 59, 0 },
    { RBVK_F3, 60, 0 },
    { RBVK_F4, 56, 0 },
    { RBVK_F5, 61, 0 },

    { RBVK_Return, 48, 0 },
    { RBVK_Space, 54, 0 },
    { RBVK_Tab, 19, 0 },
    { RBVK_Escape, 51, 0 },
    { RBVK_CapsLock, 33, 0},
    { RBVK_RBracket, 40, 0 },
    { RBVK_Num5, 58, 0 },
    { RBVK_Num4, 62, 0 },
    { RBVK_Num7, 63, 0 },
    { RBVK_LBracket, 32, 0 },
    { RBVK_Z, 41, 0 },

    { RBVK_Period, 42, 0 },
    { RBVK_C, 43, 0 },
    { RBVK_B, 44, 0 },
    { RBVK_Grave, 45, 0 },

    { RBVK_M, 46, 0 },
    { RBVK_Quote, 47, 0 },
    { RBVK_BackSlash, 53, 0 },
    { RBVK_K, 34, 0 },
    { RBVK_S, 35, 0 },
    { RBVK_F, 36, 0 },
    { RBVK_Equal, 37, 0 },
    { RBVK_G, 38, 0 },
    { RBVK_SemiColon, 39, 0 },
    { RBVK_L, 24, 0 },
    { RBVK_Num3, 25, 0 },
    { RBVK_H, 26, 0 },
    { RBVK_Num1, 27, 0 },
    { RBVK_A, 28, 0 },
    { RBVK_P, 29, 0 },
    { RBVK_D, 30, 0 },
    { RBVK_J, 31, 0 },
    { RBVK_Num9, 16, 0 },
    { RBVK_W, 17, 0 },
    { RBVK_I, 18, 0 },
    { RBVK_R, 20, 0 },
    { RBVK_Dash, 21, 0 },
    { RBVK_Y, 22, 0 },
    { RBVK_O, 23, 0 },
    { RBVK_Num8, 8, 0 },
    { RBVK_Num2, 9, 0 },
    { RBVK_Num6, 10, 0 },
    { RBVK_Q, 11, 0 },
    { RBVK_E, 12, 0 },
    { RBVK_Num0, 13, 0 },
    { RBVK_T, 14, 0 },
    { RBVK_U, 15, 0 },
    { RBVK_X, 3, 0 },
    { RBVK_V, 4, 0 },
    { RBVK_Slash, 5, 0 },
    { RBVK_Underline, 5, 0 },
    { RBVK_N, 6, 0 },
    { RBVK_Comma, 7, 0 },
    
    { -1, 0, 0 } }; // -1 needed to stop loop

void QLRBProcessKey(RBEvent event, int pressed) {
	int i = 0;
	int mod = 0;
    RBVirtualKey code = event.code;
    
	// Special case backspace
	if ((code == RBVK_BackSpace) && pressed) {
		queueKey(1 << 1, 49, 0);
		return;
	}

	switch (code) {
	case RBVK_LShift:
	case RBVK_RShift:
		rb_shiftstate = pressed;
		break;
	case RBVK_LControl:
	case RBVK_RControl:
		rb_controlstate = pressed;
		break;
	case RBVK_LAlt:
	case RBVK_RAlt:
		rb_altstate = pressed;
		break;
    default:
        break;
	}

    if (event.shift) {
        rb_shiftstate = pressed;
    }
    
	mod = rb_altstate | rb_controlstate << 1 | rb_shiftstate << 2;
    
	while (qlmap[i].keycode != -1) {
		if (code == qlmap[i].keycode) {
            if (pressed) {
				queueKey(mod, qlmap[i].code, 0);
            }
            
			RBQLKeyrowChg(qlmap[i].code, pressed);
		}
        
		i++;
	}
}

static void QLRBProcessMouse(int x, int y) {
	int qlx = 0, qly = 0;
	float x_ratio, y_ratio;

    // FIXME: Refactor. Check if mouse pos nees for 2*DPI

	if (x < dest_rect.x) {
		qlx = 0;
	}
    else if (x > (dest_rect.w + dest_rect.x)) {
		qlx = qlscreen.xres - 1;
	}
    else {
		x_ratio = (float)dest_rect.w / (float)qlscreen.xres;

		x -= dest_rect.x;

		qlx = ((float)x / x_ratio);
	}

	if (y < dest_rect.y) {
		qly = 0;
	}
    else if (y > (dest_rect.h + dest_rect.y)) {
		qly = qlscreen.yres - 1;
	}
    else {
		y_ratio = (float)dest_rect.h / (float)qlscreen.yres;

		y -= dest_rect.y;

		qly = ((float)y / y_ratio);
	}

	QLMovePointer(qlx, qly);
}

static RBEvent lastEvent = {};

int QLRBPollEvent(RBEvent* event) {
    if (lastEvent.type == RBEVT_None) {
        return 0;
    }
    
    event->type = lastEvent.type;
    event->control = lastEvent.control;
    event->alt = lastEvent.alt;
    event->shift = lastEvent.shift;
    event->code = lastEvent.code;
    event->ch = lastEvent.ch;

    if (lastEvent.type == RBEVT_KeyPressed && lastEvent.control == 0 && lastEvent.alt == 0) {
        lastEvent.type = RBEVT_TextEntered;
    }
    else if (lastEvent.type == RBEVT_TextEntered) {
        lastEvent.type = RBEVT_None;
    }
    else {
        lastEvent.type = RBEVT_None;
    }
    
    return 1;
}

void QLRBSendEvent(RBEvent evt) {
    lastEvent = evt;
}

void QLRBProcessEvents(void) {
    RBEvent event;

	while (QLRBPollEvent(&event)) {
		switch (event.type) {
		case RBEVT_KeyPressed:
			QLRBProcessKey(event, 1);
			break;
		case RBEVT_KeyReleased:
			QLRBProcessKey(event, 0);
			break;
		case RBEVT_Closed:
			cleanup(0);
			break;
                
		case RBEVT_MouseMoved:
			QLRBProcessMouse(event.mouseX, event.mouseY);
			break;
		case RBEVT_MouseButtonPressed:
			QLButton(event.mouseBtn, 1);
			break;
		case RBEVT_MouseButtonReleased:
			QLButton(event.mouseBtn, 0);
			break;
		default:
			break;
		}
	}

	QLRBRenderScreen();
}

void QLRBExit(void) {
    // Shutdown screen system (not needed in iQL)
}

// FIXME: Replace after by semaphore
static int atomic_screen_update = 0;
static int atomic_poll = 0;

int rb_AtomicSetScreenUpdate(int v) {
    int old = atomic_screen_update;
    atomic_screen_update = v;
    
    return old;
}

int rb_AtomicGetScreenUpdate() {
    return atomic_screen_update;
}

int rb_AtomicSetPoll(int v) {
    int old = atomic_poll;
    atomic_poll = v;

    return old;
}

int rb_AtomicGetPoll() {
    return atomic_poll;
}
