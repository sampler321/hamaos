/* ASCII aquarium: fish swim both ways, bubbles rise, seaweed + sand at the bottom. */
#include "hama.h"
#include "gfx.h"
#include "keys.h"

#define TOP     48
#define SURF    (TOP+4)
#define WEED_Y  (576-5*CHAR_H)       /* seaweed rows start here; sand is the last row */
#define NFISH   7
#define NBUB    10
static u32 rng=0xbeef;
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
/* right-facing art; left-facing is mirrored at draw time */
static const char *const ART[][5]={
    {"><>",0},
    {"  .--.  ", "><  * >", "  `--'  ",0},
    {"   _/|_  ", ">=(  o )>", "   ~\\|~  ",0},
    {"      __    ", "\\\\___/  \\__ ", " >==     o >", "//```\\__/`` ",0},
    {"  ,  ", "<(o)>", "  '  ",0},
    {"   .:.   ", "}-{(  @)>", "   ':'   ",0},
};
#define NART 6
static int slen(const char *s){ int n=0; while(s[n]) n++; return n; }
static void art_size(int a,int *w,int *h){ int mw=0,i=0; for(;ART[a][i];i++){ int l=slen(ART[a][i]); if(l>mw) mw=l; } *w=mw; *h=i; }
static char flip(char c){
    switch(c){ case '(':return ')'; case ')':return '('; case '<':return '>'; case '>':return '<';
    case '/':return '\\'; case '\\':return '/'; case '[':return ']'; case ']':return '[';
    case '{':return '}'; case '}':return '{'; case '`':return '\''; case '\'':return '`'; }
    return c;
}
typedef struct { int x, v, y, art, w, h, drawn, dirty; color c; } Fish;   /* x,v in 1/16 px */
typedef struct { int x, y, v, live; char ch; } Bub;
static Fish *fish; static Bub *bub;
static color pal[6];
static void draw_fish(Fish *f,int px){
    int left=f->v<0;
    for(int j=0;j<f->h;j++){ const char *s=ART[f->art][j]; int l=slen(s);
        for(int i=0;i<f->w;i++){
            char ch = left ? (f->w-1-i<l ? flip(s[f->w-1-i]) : ' ') : (i<l ? s[i] : ' ');
            int cx=px+i*CHAR_W; if(cx<=-CHAR_W||cx>=1024) continue;
            gfx_char(cx,f->y+j*CHAR_H,ch, (ch=='o'||ch=='@'||ch=='*')?TN_FG:f->c, TN_BG);
        } }
}
static void spawn_fish(Fish *f,int any){
    f->art=rnd()%NART; art_size(f->art,&f->w,&f->h);
    int pw=f->w*CHAR_W;
    int sp=8+rnd()%40;                         /* 0.5 .. 3 px per frame */
    f->v = (rnd()&1) ? sp : -sp;
    int span=WEED_Y-8-(SURF+30)-f->h*CHAR_H;
    f->y=SURF+30+(rnd()%span)/2*2;
    int px = any ? (int)(rnd()%1024) : (f->v>0 ? -pw : 1024);
    f->x=(px&~1)<<4; f->c=pal[rnd()%6]; f->drawn=-99999; f->dirty=1;
}
static int hit(int ax,int ay,int aw,int ah,int bx,int by,int bw,int bh){
    return ax<bx+bw && bx<ax+aw && ay<by+bh && by<ay+ah;
}
static void spawn_bub(Bub *b){
    if(rnd()%2){ Fish *f=&fish[rnd()%NFISH]; int px=(f->x>>4)&~1;
        b->x = f->v>0 ? px+f->w*CHAR_W : px-CHAR_W; b->y=f->y; }
    else { b->x=(rnd()%100)*CHAR_W; b->y=WEED_Y-CHAR_H; }
    b->x&=~1; if(b->x<0||b->x>1024-CHAR_W){ b->live=0; return; }
    b->v=1+rnd()%2; b->ch=".oO"[rnd()%3]; b->live=1;
}
int main(unsigned base){
    gfx_init(); keys_init();
    fish=sys_malloc(sizeof(Fish)*NFISH); bub=sys_malloc(sizeof(Bub)*NBUB);
    if(!fish||!bub) return 0;
    pal[0]=TN_ORANGE; pal[1]=TN_CYAN; pal[2]=TN_RED; pal[3]=TN_PURPLE; pal[4]=TN_BLUE; pal[5]=RGB(0xe0,0xaf,0x68);
    gfx_clear(TN_BG); header("./aquarium","[back] quit");
    for(int x=0;x<1024;x+=CHAR_W) gfx_char(x,SURF,(x/CHAR_W)%3==1?'-':'~',TN_BLUE,TN_BG);
    /* static seaweed + sand */
    for(int c=0;c<102;c++){
        gfx_char(c*CHAR_W,576-CHAR_H,".,_.:'_."[rnd()%8],RGB(0x8a,0x76,0x50),TN_BG);
        if(rnd()%6) continue;
        int hgt=1+rnd()%4;
        for(int j=0;j<hgt;j++) gfx_char(c*CHAR_W,576-CHAR_H*(2+j),(j+c)&1?'(':')',j==hgt-1?TN_GREEN:RGB(0x5a,0x8a,0x40),TN_BG);
    }
    for(int i=0;i<NFISH;i++) spawn_fish(&fish[i],1);
    for(int i=0;i<NBUB;i++) bub[i].live=0;
    for(int frame=1;;frame++){
        if(want_quit()) return 0;
        /* erase bubbles; any fish they touched needs a repaint */
        for(int i=0;i<NBUB;i++){ Bub *b=&bub[i]; if(!b->live) continue;
            gfx_fill(b->x,b->y,CHAR_W,CHAR_H,TN_BG);
            for(int k=0;k<NFISH;k++){ Fish *f=&fish[k];
                if(hit(b->x,b->y,CHAR_W,CHAR_H,f->drawn,f->y,f->w*CHAR_W,f->h*CHAR_H)) f->dirty=1; } }
        for(int i=0;i<NFISH;i++){ Fish *f=&fish[i];
            f->x+=f->v; int px=(f->x>>4)&~1, pw=f->w*CHAR_W;
            if((f->v>0 && px>=1024) || (f->v<0 && px<=-pw)){
                gfx_fill(f->drawn,f->y,pw,f->h*CHAR_H,TN_BG); spawn_fish(f,0); continue; }
            if(px==f->drawn && !f->dirty) continue;
            int d=px-f->drawn;
            if(d>0 && d<pw) gfx_fill(f->drawn,f->y,d,f->h*CHAR_H,TN_BG);
            else if(d<0 && -d<pw) gfx_fill(px+pw,f->y,-d,f->h*CHAR_H,TN_BG);
            draw_fish(f,px); f->drawn=px; f->dirty=0;
        }
        for(int i=0;i<NBUB;i++){ Bub *b=&bub[i];
            if(!b->live){ if(rnd()%20==0) spawn_bub(b); continue; }
            b->y-=b->v*2; if(frame%9==0) b->x+=(rnd()%3-1)*2;
            if(b->y<SURF+CHAR_H+2 || b->x<0 || b->x>1024-CHAR_W){ b->live=0; continue; }
            int behind=0;
            for(int k=0;k<NFISH;k++){ Fish *f=&fish[k];
                if(hit(b->x,b->y,CHAR_W,CHAR_H,f->drawn,f->y,f->w*CHAR_W,f->h*CHAR_H)) behind=1; }
            if(!behind) gfx_char(b->x,b->y,b->ch,TN_CYAN,TN_BG);
        }
        if(frame%50==0) gfx_present();
        sys_sleep(4);
    }
}
