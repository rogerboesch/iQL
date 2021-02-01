//
//  RB_screen.h
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#ifndef _RBSCREEN_H
#define _RBSCREEN_H

void QLRBScreen(void);
void QLRBRenderScreen(void);
void QLRBProcessEvents(void);
void QLRBExit(void);
void QLRBUpdatePixelBuffer(void);
void QLRBUpdateScreenByte(uint32_t, uint8_t);
void QLRBUpdateScreenWord(uint32_t, uint16_t);
void QLRBUpdateScreenLong(uint32_t, uint32_t);


extern unsigned int rb_keyrow[8];
extern int rb_shiftstate, rb_controlstate, rb_altstate;

// FIXME: Check
extern int doPoll;
extern int rb_AtomicSetScreenUpdate(int v);
extern int rb_AtomicGetScreenUpdate(void);
extern int rb_AtomicSetPoll(int v);
extern int rb_AtomicGetPoll(void);

#endif

