/* Direct framebuffer graphics for the Hama 00095230: 1024x576 YUYV 4:2:2, double buffered.
 * Colours are packed YUYV words (Y | U<<8 | Y<<16 | V<<24); build them with RGB(). */
#ifndef GFX_H
#define GFX_H
#include "hama.h"
typedef unsigned short u16;
typedef u32 color;
extern int gfx_w, gfx_h;
extern u32 gfx_detected;
#define CLAMP8(v) ((v)<0?0:(v)>255?255:(v))
#define RGB_Y(r,g,b) CLAMP8((( 66*(r)+129*(g)+ 25*(b)+128)>>8)+16)
#define RGB_U(r,g,b) CLAMP8(((-38*(r)- 74*(g)+112*(b)+128)>>8)+128)
#define RGB_V(r,g,b) CLAMP8(((112*(r)- 94*(g)- 18*(b)+128)>>8)+128)
#define RGB(r,g,b) ((color)(RGB_Y(r,g,b) | (RGB_U(r,g,b)<<8) | (RGB_Y(r,g,b)<<16) | ((u32)RGB_V(r,g,b)<<24)))
int  gfx_init(void);            /* 0 = canvas not found */
void gfx_fill(int x,int y,int w,int h,color c);
void gfx_clear(color c);
void gfx_present(void);
#define CHAR_W 10
#define CHAR_H 20
void gfx_char(int x,int y,char c,color fg,color bg);
int  gfx_text(int x,int y,const char *s,color fg,color bg);
/* Tokyo Night palette */
#define TN_BG     RGB(0x1a,0x1b,0x26)
#define TN_BG2    RGB(0x24,0x28,0x3b)
#define TN_FG     RGB(0xc0,0xca,0xf5)
#define TN_DIM    RGB(0x56,0x5f,0x89)
#define TN_BLUE   RGB(0x7a,0xa2,0xf7)
#define TN_CYAN   RGB(0x7d,0xcf,0xff)
#define TN_GREEN  RGB(0x9e,0xce,0x6a)
#define TN_ORANGE RGB(0xff,0x9e,0x64)
#define TN_RED    RGB(0xf7,0x76,0x8e)
#define TN_PURPLE RGB(0xbb,0x9a,0xf7)
#endif
