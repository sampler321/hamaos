/* hamaOS shell: type with the arrows. up/down = change letter, right = next, left = delete,
 * ok = run line, back = leave (or clear the line). Commands: help clear echo calc uptime about run <app> */
#include "hama.h"
#include "gfx.h"
#include "keys.h"
#include "sound.h"
#ifdef MONO
int app_snake(unsigned), app_matrix(unsigned), app_aquarium(unsigned), app_shapes(unsigned), app_dvd(unsigned);
#endif
#define COLS 100
#define TOP 56
#define ROWS 22
static const char CS[]=" abcdefghijklmnopqrstuvwxyz0123456789+-*/%()=.,:!?<>_#";
#define NCS ((int)sizeof(CS)-1)
static char hist[ROWS][COLS+1]; static color hcol[ROWS]; static int nh;
static void redraw_hist(void){
    for(int i=0;i<ROWS;i++){ gfx_fill(0,TOP+i*20,1024,20,TN_BG); if(hist[i][0]) gfx_text(16,TOP+i*20,hist[i],hcol[i],TN_BG); }
}
static void out(const char *s,color c){
    if(nh==ROWS){ for(int i=1;i<ROWS;i++){ for(int j=0;j<=COLS;j++) hist[i-1][j]=hist[i][j]; hcol[i-1]=hcol[i]; } nh--; }
    int j=0; while(s[j]&&j<COLS){ hist[nh][j]=s[j]; j++; } hist[nh][j]=0; hcol[nh]=c; nh++;
    redraw_hist();
}
static int slen(const char *s){int n=0;while(s[n])n++;return n;}
static int starts(const char *s,const char *w){ int i=0; while(w[i]){ if(s[i]!=w[i]) return 0; i++; } return s[i]==0||s[i]==' '; }
static void itoa_(long v,char *o){ char b[24]; int i=0,neg=v<0; unsigned long u=neg?-v:v; do{ b[i++]='0'+u%10; u/=10; }while(u); if(neg) b[i++]='-'; int k=0; while(i) o[k++]=b[--i]; o[k]=0; }
/* tiny integer expression parser */
static const char *ep; static int eerr;
static long expr(void);
static void ws(void){ while(*ep==' ') ep++; }
static long prim(void){ ws(); long v=0;
    if(*ep=='('){ ep++; v=expr(); ws(); if(*ep==')') ep++; else eerr=1; return v; }
    if(*ep=='-'){ ep++; return -prim(); }
    if(*ep<'0'||*ep>'9'){ eerr=1; return 0; }
    while(*ep>='0'&&*ep<='9') v=v*10+(*ep++-'0'); return v; }
static long term(void){ long v=prim(); for(;;){ ws(); char o=*ep; if(o!='*'&&o!='/'&&o!='%') return v; ep++; long r=prim();
    if((o=='/'||o=='%')&&r==0){ eerr=2; return 0; } v = o=='*'?v*r : o=='/'?v/r : v%r; } }
static long expr(void){ long v=term(); for(;;){ ws(); char o=*ep; if(o!='+'&&o!='-') return v; ep++; long r=term(); v = o=='+'?v+r:v-r; } }
static void run_line(char *line){
    char buf[COLS+8];
    { int k=0; const char *p="~ $ "; while(*p) buf[k++]=*p++; for(int i=0;line[i];i++) buf[k++]=line[i]; buf[k]=0; out(buf,TN_FG); }
    char *a=line; while(*a==' ') a++;
    if(!*a) return;
    if(starts(a,"help")){ out("help clear echo <text> calc <expr> uptime about run <app>",TN_CYAN);
                          out("apps: snake matrix aquarium shapes dvd   (or type an expression: 6*7)",TN_CYAN); return; }
    if(starts(a,"clear")){ for(int i=0;i<ROWS;i++) hist[i][0]=0; nh=0; redraw_hist(); return; }
    if(starts(a,"echo")){ out(a[4]?a+5:"",TN_FG); return; }
    if(starts(a,"about")){ out("hamaOS on a 2009 Hama 00095230 frame - Actions AM7331 MIPS, uC/OS-II",TN_PURPLE); return; }
    if(starts(a,"uptime")){ u32 t=ticks_now()/100; char n[24]; itoa_(t/60,n); int k=slen(n);
        buf[0]=0; int j=0; for(int i=0;n[i];i++) buf[j++]=n[i]; const char *m=" min "; while(*m) buf[j++]=*m++;
        itoa_(t%60,n); for(int i=0;n[i];i++) buf[j++]=n[i]; buf[j++]=' '; buf[j++]='s'; buf[j]=0; (void)k; out(buf,TN_GREEN); return; }
    if(starts(a,"run")){
#ifdef MONO
        char *n=a+3; while(*n==' ') n++;
        int (*f)(unsigned)=0;
        if(starts(n,"snake")) f=app_snake; else if(starts(n,"matrix")) f=app_matrix; else if(starts(n,"aquarium")) f=app_aquarium;
        else if(starts(n,"shapes")) f=app_shapes; else if(starts(n,"dvd")) f=app_dvd;
        if(!f){ out("run: unknown app (try: help)",TN_RED); return; }
        int mk=mem_mark(); f(0); snd_stop(); mem_release(mk); keys_init(); gfx_init();
        gfx_clear(TN_BG); out("returned to shell",TN_DIM); return;
#else
        out("run: only in the built-in hamaOS",TN_RED); return;
#endif
    }
    if(starts(a,"calc")) a+=4;
    ep=a; eerr=0; long v=expr(); ws();
    if(!eerr && *ep==0){ char n[24]; itoa_(v,n); buf[0]='='; buf[1]=' '; int j=2; for(int i=0;n[i];i++) buf[j++]=n[i]; buf[j]=0; out(buf,TN_GREEN); return; }
    if(eerr==2){ out("calc: division by zero",TN_RED); return; }
    out("unknown command (try: help)",TN_RED);
}
static void draw_input(const char *line,int len,int cur){
    int y=TOP+ROWS*20+12;
    gfx_fill(0,y-4,1024,28,TN_BG2);
    int x=gfx_text(16,y,"~ $ ",TN_BLUE,TN_BG2);
    for(int i=0;i<len;i++){ gfx_char(x,y,line[i],TN_FG,TN_BG2); x+=CHAR_W; }
    gfx_char(x,y,CS[cur],TN_BG,TN_ORANGE);            /* the letter being chosen */
    gfx_text(1024-16-10*42,y,"[up/dn] letter [>] next [<] del [ok] run",TN_DIM,TN_BG2);
    gfx_present();
}
int main(unsigned base){
    gfx_init(); keys_init();
    gfx_clear(TN_BG);
    gfx_fill(0,0,1024,44,TN_BG2); gfx_fill(0,44,1024,2,TN_DIM);
    int x=gfx_text(16,12,"~/apps",TN_BLUE,TN_BG2); x=gfx_text(x,12," $ ",TN_DIM,TN_BG2); x=gfx_text(x,12,"./shell",TN_GREEN,TN_BG2);
    gfx_text(x+30,12,"[back] on empty line quits",TN_DIM,TN_BG2);
    nh=0; for(int i=0;i<ROWS;i++) hist[i][0]=0;
    out("hamaOS shell. type 'help' and press ok.",TN_DIM);
    char line[COLS]; int len=0, cur=1;          /* start on 'a' */
    draw_input(line,len,cur);
    for(;;){
        int k=keys_poll(100);
        if(k<0) continue;
        if(k==K_UP){ cur=(cur+1)%NCS; }
        else if(k==K_DOWN){ cur=(cur+NCS-1)%NCS; }
        else if(k==K_RIGHT){ if(len<COLS-8) line[len++]=CS[cur]; }
        else if(k==K_LEFT){ if(len>0){ len--; for(int i=0;i<NCS;i++) if(CS[i]==line[len]) cur=i; } }
        else if(k==K_OK){ if(cur!=0 && len<COLS-8) line[len++]=CS[cur]; line[len]=0; run_line(line); len=0; cur=1; }
        else if(k==K_BACK){ if(len==0) return 0; len=0; cur=1; }
        draw_input(line,len,cur);
    }
}
