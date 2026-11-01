/* PCM audio out via ADA.DRV (module 0x10): 22050 Hz, 16-bit mono. */
#ifndef SOUND_H
#define SOUND_H
#include "hama.h"
#define SND_RATE 22050
int  snd_init(int volume);           /* volume 0..31; returns 1 on success */
int  snd_queued(void);               /* bytes still waiting in the DMA ring */
int  snd_write(const short *pcm, int samples);   /* non-blocking: returns samples accepted (0 if full) */
void snd_stop(void);
void snd_tone(int hz, int ms, int vol);           /* blocking square-wave blip (sound effects) */
#endif
