/* Shared input: own message queue, KEYS.TXT button map, clean one-press-one-action events. */
#ifndef KEYS_H
#define KEYS_H
#include "hama.h"
enum { K_UP, K_DOWN, K_LEFT, K_RIGHT, K_OK, K_BACK, K_COUNT };
#define KEY_ALTS 4
extern int key_id[K_COUNT];            /* primary id per action (-1 = none) */
extern int key_alt[K_COUNT][KEY_ALTS]; /* all ids per action (remote + frame buttons) */
int  keys_init(void);          /* create queue, load KEYS.TXT; returns 1 if all arrows + ok known */
void keys_attach(void);        /* re-point the task's queue at ours (after running another app) */
int  keys_poll(int ticks);     /* next press: K_* or 100+raw id for unmapped, -1 nothing, -2 ignored */
int  keys_raw_press(int ticks);/* setup helper: raw id of one full press (waits for release), -1 timeout */
void keys_save(void);
u32  ticks_now(void);          /* OS tick counter, 10 ms per tick */
const char *key_name(int k);
#endif
