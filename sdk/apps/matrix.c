/* Matrix digital rain. Only cells that change are redrawn. */
#include "hama.h"
#include "gfx.h"
#include "keys.h"
void *memset(void *d,int c,unsigned n);

#define TOP  48
#define COLS (1024/CHAR_W)
#define ROWS ((576-TOP)/CHAR_H)
static u32 rng=0x1234567;
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
static const char GLYPHS[]="01<>|+=*:;^~ZXKHMWT7$#%&@?!LJVY[]{}/\\";
static char *cell;                 /* COLS*ROWS chars on screen */
static signed char *head, *len, *spd, *cnt;
static void put(int c,int r,char ch,color fg){
    if(r<0||r>=ROWS) return;
    cell[r*COLS+c]=ch; gfx_char(c*CHAR_W,TOP+r*CHAR_H,ch,fg,TN_BG);
}
static void reset_col(int c,int first){
    head[c]= first ? -(int)(rnd()%ROWS) : -(int)(rnd()%12);
    len[c]=6+rnd()%16; spd[c]=1+rnd()%3; cnt[c]=0;
}
int main(unsigned base){
    gfx_init(); keys_init();
    cell=sys_malloc(COLS*ROWS); head=sys_malloc(COLS*4);
    if(!cell||!head) return 0;
    len=head+COLS; spd=len+COLS; cnt=spd+COLS;
    memset(cell,' ',COLS*ROWS);
    const color HEAD=RGB(0xe0,0xff,0xe0), TRAIL=TN_GREEN, DIMG=RGB(0x3a,0x6a,0x3a);
    gfx_clear(TN_BG); header("./matrix","[back] quit");
    for(int c=0;c<COLS;c++) reset_col(c,1);
    gfx_present();
    for(int frame=1;;frame++){
        if(want_quit()) return 0;
        for(int c=0;c<COLS;c++){
            if(++cnt[c]<spd[c]) continue;
            cnt[c]=0;
            int h=head[c], l=len[c];
            if(h-1>=0 && h-1<ROWS) put(c,h-1,cell[(h-1)*COLS+c],TRAIL);
            if(h>=0 && h<ROWS) put(c,h,GLYPHS[rnd()%(sizeof(GLYPHS)-1)],HEAD);
            int f=h-l/2; if(f>=0 && f<ROWS && f<h-1) put(c,f,cell[f*COLS+c],DIMG);
            int t=h-l; if(t>=0 && t<ROWS){ cell[t*COLS+c]=' '; gfx_fill(c*CHAR_W,TOP+t*CHAR_H,CHAR_W,CHAR_H,TN_BG); }
            head[c]=h+1;
            if(h+1-l>=ROWS) reset_col(c,0);
        }
        /* glitch: a few trail glyphs mutate in place */
        for(int i=0;i<6;i++){
            int c=rnd()%COLS, r=rnd()%ROWS, h=head[c]-1;
            if(cell[r*COLS+c]==' ' || r>=h) continue;
            put(c,r,GLYPHS[rnd()%(sizeof(GLYPHS)-1)], r>h-len[c]/2 ? TRAIL : DIMG);
        }
        if(frame%50==0) gfx_present();
        sys_sleep(5);
    }
}
