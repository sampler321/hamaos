/* hamaOS launcher: terminal-style home screen. Runs apps (.BIN) from D:\APPS\ on the SD card. */
#include "hama.h"
#include "gfx.h"
#include "keys.h"
#include "sound.h"

static const char *banner[] = {
"  _                                   ___  ____  ",
" | |__   __ _ _ __ ___   __ _        / _ \\/ ___| ",
" | '_ \\ / _` | '_ ` _ \\ / _` |_____ | | | \\___ \\ ",
" | | | | (_| | | | | | | (_| |_____|| |_| |___) |",
" |_| |_|\\__,_|_| |_| |_|\\__,_|       \\___/|____/ ",
};
#ifdef MONO
int app_snake(unsigned), app_matrix(unsigned), app_aquarium(unsigned), app_shapes(unsigned), app_dvd(unsigned), app_shell(unsigned);
#endif
typedef struct { const char *label, *desc, *file; int action; } item;
enum { A_APP, A_KEYS, A_INSTALL, A_EXIT, A_DRIVES };
static const item menu[] = {
    {"snake",    "the classic. arrows steer, back quits",      "SNAKE.BIN",  A_APP},
    {"matrix",   "digital rain",                                 "MATRIX.BIN", A_APP},
    {"aquarium", "ascii fish tank",                              "AQUA.BIN",   A_APP},
    {"shapes",   "mystify-style screensaver",                    "SHAPES.BIN", A_APP},
    {"dvd",      "bouncing logo. will it hit the corner?",       "DVD.BIN",    A_APP},
    {"shell",    "type commands with the arrows (calc, echo, run)", "SHELL.BIN", A_APP},
    {"script",   "run SCRIPT.TXT",                              "SCRIPT.BIN", A_APP},
    {"keysetup", "teach the remote's buttons again",             0,            A_KEYS},
    {"drives",   "list storage drives (diagnostic)",             0,            A_DRIVES},
    {"exit",     "back to the stock Hama firmware",              0,            A_EXIT},
};
static void cat(char *d,const char *a,const char *b){ while(*a)*d++=*a++; while(*b)*d++=*b++; *d=0; }
/* copy one file; returns bytes copied or <0 */
static int copy_file(const char *from,const char *to){
    void *f=sys_fopen(from,FMODE_READ); if(!f) return -1;
    u8 *b=sys_malloc(0x10000); if(!b){ sys_fclose(f); return -2; }
    int n=sys_fread(b,0x10000,f); sys_fclose(f);
    int w=-3;
    if(n>0){ void *o=sys_fopen(to,FMODE_WRITE); if(o){ w=sys_fwrite(b,n,o); sys_fclose(o); } }
    sys_free(b); return w;
}
static const char HXD[]="0123456789abcdef";
static void hexs(char *o,u32 v,int n){ for(int i=0;i<n;i++) o[i]=HXD[(v>>(4*(n-1-i)))&15]; o[n]=0; }
static void drives(void){
    gfx_clear(TN_BG);
    int x=gfx_text(40,40,"~ $ ",TN_DIM,TN_BG); gfx_text(x,40,"lsblk",TN_GREEN,TN_BG);
    u8 buf[96]; for(int i=0;i<96;i++) buf[i]=0;
    long r=sys_call(OSCALL(3,0x2e),0x10020000,(long)buf,0,0);
    void *f=sys_fopen("D:\\DRIVES.TXT",FMODE_WRITE);
    char line[96]; hexs(line,(u32)r,8);
    x=gfx_text(60,80,"ret 0x",TN_DIM,TN_BG); gfx_text(x,80,line,TN_FG,TN_BG);
    if(f){ sys_fwrite("ret ",4,f); sys_fwrite(line,8,f); sys_fwrite("\r\n",2,f); }
    for(int e=0;e<4;e++){
        u8 *en=buf+e*20; int k=0;
        line[k++]=(en[0]>=32&&en[0]<127)?en[0]:'?'; line[k++]=':'; line[k++]=' ';
        const char *t="type "; while(*t) line[k++]=*t++;
        hexs(line+k,en[4]|en[5]<<8|en[6]<<16|en[7]<<24,8); k+=8; line[k++]=' '; line[k++]=' ';
        for(int b=0;b<20;b++){ hexs(line+k,en[b],2); k+=2; }
        line[k]=0;
        gfx_text(60,120+e*30,line,(en[4]==16)?TN_GREEN:TN_CYAN,TN_BG);
        if(f){ sys_fwrite(line,k,f); sys_fwrite("\r\n",2,f); }
    }
    if(f) sys_fclose(f);
    gfx_text(60,260,"saved to D:\\DRIVES.TXT - press any button",TN_DIM,TN_BG);
    while(keys_poll(100)<0){}
}
static void install(void){
    static const char *files[][2] = {
        {"D:\\RUN.BIN","C:\\RUN.BIN"}, {"D:\\APPS\\SNAKE.BIN","C:\\SNAKE.BIN"},
        {"D:\\APPS\\SCRIPT.BIN","C:\\SCRIPT.BIN"}, {"D:\\KEYS.TXT","C:\\KEYS.TXT"}, {"D:\\SCRIPT.TXT","C:\\SCRIPT.TXT"} };
    gfx_clear(TN_BG);
    int x=gfx_text(40,40,"~ $ ",TN_DIM,TN_BG); gfx_text(x,40,"install --target C:",TN_GREEN,TN_BG);
    int ok=0;
    for(int i=0;i<5;i++){
        int y=90+i*30; x=gfx_text(60,y,files[i][0],TN_FG,TN_BG); x=gfx_text(x,y," -> ",TN_DIM,TN_BG); x=gfx_text(x,y,files[i][1],TN_CYAN,TN_BG);
        int r=copy_file(files[i][0],files[i][1]);
        gfx_text(x+20,y, r>0?"ok": r==-1?"missing":"FAILED", r>0?TN_GREEN: r==-1?TN_ORANGE:TN_RED, TN_BG);
        if(r>0) ok++;
    }
    gfx_text(60,260, ok>=2 ? "done. hamaOS now runs without the card (Start Up Mode = Calendar)." :
                              "install failed: internal drive C: not writable?", ok>=2?TN_GREEN:TN_RED, TN_BG);
    gfx_text(60,300,"press any button",TN_DIM,TN_BG);
    while(keys_poll(100)<0){}
}
#define NMENU (int)(sizeof menu/sizeof menu[0])

static void cache_sync(void){
#ifndef TEST
    for(u32 a=0x80000000u; a<0x80010000u; a+=16)
        __asm__ volatile(".set push\n.set mips32\ncache 0x01,0(%0)\ncache 0x00,0(%0)\n.set pop" :: "r"(a));
    __asm__ volatile("sync");
#endif
}
extern u32 __disp;
static int run_builtin(const char *file){
#ifdef MONO
    int (*f)(unsigned)=0;
    if(file[0]=='S'&&file[1]=='N') f=app_snake; else if(file[0]=='M') f=app_matrix; else if(file[0]=='A') f=app_aquarium;
    else if(file[0]=='S'&&file[1]=='H'&&file[2]=='A') f=app_shapes; else if(file[0]=='D') f=app_dvd; else if(file[0]=='S'&&file[1]=='H'&&file[2]=='E') f=app_shell;
    if(f){ int mk=mem_mark(); f(0); snd_stop(); mem_release(mk); keys_attach(); gfx_init(); return 0; }
#endif
    return -1;
}
static int run_app(const char *file){
#ifdef MONO
    { int r=run_builtin(file); if(r!=-1) return r; }      /* built-in apps first */
#endif
    char path[64]; cat(path,"D:\\APPS\\",file);
    u8 *buf=sys_malloc(0x10000); if(!buf) return -2;
    void *f=sys_fopen(path,FMODE_READ);
    if(f){ sys_fread(buf,0x10000,f); sys_fclose(f); }
    else { sys_free(buf); return -1; }
    if(((u32*)buf)[0]==0){ sys_free(buf); return -3; }
    cache_sync();
    int (*entry)(u32,u8*)=(int(*)(u32,u8*))buf;
    int r=entry(__disp,buf);
    sys_free(buf);
    keys_attach();
    return r;
}
static int owner(int id){ for(int k=0;k<K_COUNT;k++) for(int i=0;i<KEY_ALTS;i++) if(key_alt[k][i]==id) return k; return -1; }
static void add_alt(int k,int id){ for(int i=0;i<KEY_ALTS;i++) if(key_alt[k][i]<0){ key_alt[k][i]=id; return; } }
static const char HXK[]="0123456789abcdef";
/* one setup step: returns 1 if mapped, 0 skipped */
static int setup_step(int y,const char *who,int action,const char *label){
    int x=gfx_text(60,y,"> press ",TN_DIM,TN_BG); x=gfx_text(x,y,label,TN_CYAN,TN_BG); x=gfx_text(x,y," on the ",TN_DIM,TN_BG); gfx_text(x,y,who,TN_FG,TN_BG);
    for(;;){
        gfx_fill(560,y,460,20,TN_BG); gfx_text(560,y,"waiting... (15 s = skip)",TN_DIM,TN_BG);
        int id=keys_raw_press(1500);
        gfx_fill(560,y,460,20,TN_BG);
        if(id<0){ gfx_text(560,y,"skipped",TN_ORANGE,TN_BG); return 0; }
        char c[8]="0x00"; c[2]=HXK[(id>>4)&15]; c[3]=HXK[id&15];
        int o=owner(id);
        if(o>=0 && o!=action){
            int xx=gfx_text(560,y,c,TN_RED,TN_BG); xx=gfx_text(xx,y," already = ",TN_RED,TN_BG); gfx_text(xx,y,key_name(o),TN_RED,TN_BG);
            sys_sleep(120); continue;
        }
        if(o<0) add_alt(action,id);
        int xx=gfx_text(560,y,c,TN_GREEN,TN_BG); gfx_text(xx,y,"  ok",TN_GREEN,TN_BG);
        return 1;
    }
}
static void keysetup(void){
    static const int acts[6]={K_UP,K_DOWN,K_LEFT,K_RIGHT,K_OK,K_BACK};
    static const char *rl[6]={"UP","DOWN","LEFT","RIGHT","OK","BACK"};
    static const char *fl[6]={"UP","DOWN","LEFT","RIGHT","OK","MENU"};   /* frame MENU acts as BACK */
    for(int k=0;k<K_COUNT;k++) for(int i=0;i<KEY_ALTS;i++) key_alt[k][i]=-1;
    for(int round=0;round<2;round++){
        gfx_clear(TN_BG);
        int x=gfx_text(40,40,"~ $ ",TN_DIM,TN_BG); gfx_text(x,40,round?"keysetup --frame-buttons":"keysetup --remote",TN_GREEN,TN_BG);
        gfx_text(40,80, round?"now the buttons on the frame itself (not power). press, then let go."
                             :"press each button on the remote ONCE, then let go.",TN_FG,TN_BG);
        gfx_text(40,104,"wait 15 s to skip a button.",TN_DIM,TN_BG);
        for(int i=0;i<6;i++) setup_step(150+i*40, round?"frame":"remote", acts[i], round?fl[i]:rl[i]);
        sys_sleep(80);
    }
    /* safety: never leave OK/arrows empty - fall back to the remote defaults */
    static const int def[6]={0x1b,0x1d,0x00,0x20,0x1f,0x19};
    for(int k=0;k<K_COUNT;k++) if(key_alt[k][0]<0 && owner(def[k])<0) key_alt[k][0]=def[k];
    keys_save(); keys_init();
    gfx_text(60,420,"saved to KEYS.TXT",TN_GREEN,TN_BG);
    sys_sleep(120);
}
static void draw(int sel,const char *status){
    gfx_clear(TN_BG);
    for(int i=0;i<5;i++) gfx_text(40,30+i*20,banner[i],i<2?TN_BLUE:i<4?TN_CYAN:TN_PURPLE,TN_BG);
    gfx_text(560,110,"v0.8  AM7331 / uC-OS  1024x576",TN_DIM,TN_BG);
    gfx_fill(40,150,944,1,TN_DIM);
    int x=gfx_text(40,170,"~/apps",TN_BLUE,TN_BG); x=gfx_text(x,170," $ ",TN_DIM,TN_BG); gfx_text(x,170,"ls",TN_FG,TN_BG);
    for(int i=0;i<NMENU;i++){
        int y=190+i*30;
        color bg = i==sel ? TN_BG2 : TN_BG;
        if(i==sel) gfx_fill(40,y-6,944,32,TN_BG2);
        gfx_text(52,y, i==sel?">":" ",TN_ORANGE,bg);
        gfx_text(76,y,menu[i].label, i==sel?TN_GREEN:TN_FG,bg);
        gfx_text(220,y,menu[i].desc,TN_DIM,bg);
    }
    gfx_fill(0,546,1024,30,TN_BG2);
    gfx_text(16,551,"[up/down] select   [ok] run",TN_DIM,TN_BG2);
    if(status) gfx_text(1024-16-10*30,551,status,TN_ORANGE,TN_BG2);
    gfx_present();
}
int main(unsigned base){
    gfx_init();
    keys_init();
    { u32 t0=ticks_now(); for(int i=0;i<15 && ticks_now()-t0<50;i++) keys_poll(5); }   /* swallow the press that opened us */
    int sel=0; const char *status=0;
    draw(sel,status);
    for(;;){
        int k=keys_poll(100);
        if(k<0) continue;
        status=0;
        if(k==K_UP) sel=(sel+NMENU-1)%NMENU;
        else if(k==K_DOWN) sel=(sel+1)%NMENU;
        else if(k==K_BACK) status="use 'exit' to leave hamaOS";
        else if(k==K_OK){
            int a=menu[sel].action;
            if(a==A_EXIT) return 0;
            else if(a==A_KEYS) keysetup();
            else if(a==A_DRIVES) drives();
            else if(a==A_INSTALL) status="install disabled for now";
            else { int r=run_app(menu[sel].file); if(r==-1) status="app not found"; }
        }
        draw(sel,status);
    }
}
