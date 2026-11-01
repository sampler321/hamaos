/* Mystify-style screensaver: two bouncing quads with fading trails. */
#include "hama.h"
#include "gfx.h"
#include "keys.h"

#define TOP   48
#define NPOLY 2
#define NV    4
#define TR    9                      /* 8 trail copies + the current one */
static u32 rng=0x5eed;
static u32 rnd(void){ rng=rng*1103515245u+12345u; return rng>>16; }
static int want_quit(void){
    for(;;){ int k=keys_poll(1); if(k==-1) return 0;
        if(k==K_BACK || (k>=100 && key_id[K_BACK]<0)) return 1; }
}
static void header(const char *name,const char *hint){
    gfx_fill(0,0,1024,44,TN_BG2); gfx_fill(0,44,1024,2,TN_DIM);
    int x=gfx_text(16,14,"~/apps",TN_BLUE,TN_BG2); x=gfx_text(x,14," $ ",TN_DIM,TN_BG2);
    x=gfx_text(x,14,name,TN_GREEN,TN_BG2); gfx_text(x+30,14,hint,TN_DIM,TN_BG2);
}
static void line(int x0,int y0,int x1,int y1,color c){
    int dx=x1-x0, dy=y1-y0, sx=dx<0?-1:1, sy=dy<0?-1:1;
    if(dx<0) dx=-dx;
    if(dy<0) dy=-dy;
    int err=dx-dy;
    for(;;){
        gfx_fill(x0&~1,y0,2,2,c);
        if(x0==x1&&y0==y1) break;
        int e2=2*err;
        if(e2>-dy){ err-=dy; x0+=sx; }
        if(e2< dx){ err+=dx; y0+=sy; }
    }
}
/* hue 0..767 -> bright pastel-ish colour */
static color hue(int h){
    h%=768; int r,g,b,t=h%256;
    if(h<256){ r=255-t; g=t; b=0; } else if(h<512){ r=0; g=255-t; b=t; } else { r=t; g=0; b=255-t; }
    r=80+r*175/255; g=80+g*175/255; b=80+b*175/255;
    return RGB(r,g,b);
}
typedef struct { short x[NV], y[NV]; } Quad;
static Quad *hist;                    /* [TR][NPOLY] */
static short *col;                     /* hue per history slot and poly */
static int px[NPOLY][NV], py[NPOLY][NV], vx[NPOLY][NV], vy[NPOLY][NV];
static void draw(Quad *q,color c){
    for(int i=0;i<NV;i++){ int j=(i+1)%NV; line(q->x[i],q->y[i],q->x[j],q->y[j],c); }
}
int main(unsigned base){
    gfx_init(); keys_init();
    hist=sys_malloc(sizeof(Quad)*TR*NPOLY); col=sys_malloc(2*TR*NPOLY);
    if(!hist||!col) return 0;
    gfx_clear(TN_BG); header("./shapes","[back] quit");
    for(int p=0;p<NPOLY;p++) for(int i=0;i<NV;i++){
        px[p][i]=40+rnd()%940; py[p][i]=TOP+20+rnd()%480;
        vx[p][i]=(6+rnd()%10)*((rnd()&1)?1:-1); vy[p][i]=(5+rnd()%9)*((rnd()&1)?1:-1);
    }
    int filled=0, h=0;
    for(int frame=1;;frame++){
        if(want_quit()) return 0;
        int slot=frame%TR;
        if(filled>=TR) for(int p=0;p<NPOLY;p++) draw(&hist[slot*NPOLY+p],TN_BG);   /* erase oldest */
        else filled++;
        h+=3;
        for(int p=0;p<NPOLY;p++){
            Quad *q=&hist[slot*NPOLY+p];
            for(int i=0;i<NV;i++){
                px[p][i]+=vx[p][i]; py[p][i]+=vy[p][i];
                if(px[p][i]<0){ px[p][i]=-px[p][i]; vx[p][i]=-vx[p][i]; }
                if(px[p][i]>1022){ px[p][i]=2044-px[p][i]; vx[p][i]=-vx[p][i]; }
                if(py[p][i]<TOP+2){ py[p][i]=2*(TOP+2)-py[p][i]; vy[p][i]=-vy[p][i]; }
                if(py[p][i]>574){ py[p][i]=1148-py[p][i]; vy[p][i]=-vy[p][i]; }
                q->x[i]=px[p][i]; q->y[i]=py[p][i];
            }
            col[slot*NPOLY+p]=h+p*384;
            draw(q,hue(h+p*384));
        }
        /* repaint the newer copies lightly: only the previous one, which the erase most likely nicked */
        if(filled>=TR){ int prev=(slot+TR-1)%TR;
            for(int p=0;p<NPOLY;p++) draw(&hist[prev*NPOLY+p],hue(col[prev*NPOLY+p])); }
        if(frame%50==0) gfx_present();
        sys_sleep(6);
    }
}
