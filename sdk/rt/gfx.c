#include "gfx.h"
#include "font.h"
#define FB_W 1024
#define FB_H 576
int gfx_w=FB_W, gfx_h=FB_H;
u32 gfx_detected;
static u8 *buf[2]; static int nbuf;
#ifndef TEST
static const u32 fb_cand[] = {0xa0bfdfa0u, 0xa0dfefe0u};
#endif
int gfx_init(void){
#ifndef TEST
    sys_call(OSCALL(6,0x19),0,0,0,0);   /* clear the OSD overlay layers (stock menu haze / GUI text) */
    sys_call(OSCALL(6,0x19),1,0,0,0);
#endif
#ifdef TEST
    buf[0]=sys_malloc(FB_W*FB_H*2); nbuf=1; gfx_detected=(u32)buf[0]; return 1;
#else
    /* both candidates showed the menu in YUYV; draw into every buffer that holds a valid frame */
    nbuf=0;
    for(int c=0;c<2;c++){ u8 *b=(u8*)fb_cand[c]; if(b[1]!=0 || b[3]!=0) buf[nbuf++]=b; }
    if(!nbuf){ buf[0]=(u8*)fb_cand[0]; nbuf=1; }
    gfx_detected=(u32)buf[0];
    return 1;
#endif
}
static inline void put(int x,int y,color c){
    for(int i=0;i<nbuf;i++){ u8 *p=buf[i]+2*(y*FB_W+x); p[0]=(u8)c; p[1]=(x&1)?(u8)(c>>24):(u8)(c>>8); }
}
void gfx_fill(int x,int y,int w,int h,color c){
    if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
    if(x+w>FB_W) w=FB_W-x; if(y+h>FB_H) h=FB_H-y;
    if(w<=0||h<=0) return;
    for(int j=0;j<h;j++){
        int xx=x, n=w;
        if(xx&1){ put(xx,y+j,c); xx++; n--; }
        for(int i=0;i<nbuf;i++){ u32 *q=(u32*)(buf[i]+2*((y+j)*FB_W+xx)); for(int k=0;k<n/2;k++) q[k]=c; }
        if(n&1) put(xx+n-1,y+j,c);
    }
}
void gfx_clear(color c){ gfx_fill(0,0,FB_W,FB_H,c); }
void gfx_present(void){
#ifdef TEST
    sys_call(0x7f0001,(long)buf[0],FB_W,FB_H,1);   /* emulator: save screenshot (YUYV) */
#endif
    /* no lcm_update: we draw straight into the scanned-out buffers */
}
void gfx_char(int x,int y,char ch,color fg,color bg){
    if(ch<32||ch>126) ch='?';
    const unsigned short *g=font_bits[ch-32];
    for(int j=0;j<FONT_H;j++){ int yy=y+j; if(yy<0||yy>=FB_H) continue; unsigned short b=g[j];
        for(int i=0;i<FONT_W;i++){ int xx=x+i; if(xx>=0&&xx<FB_W) put(xx,yy,(b&(0x8000>>i))?fg:bg); } }
}
int gfx_text(int x,int y,const char *s,color fg,color bg){ while(*s){ gfx_char(x,y,*s++,fg,bg); x+=FONT_W; } return x; }
