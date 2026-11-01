/* Key logger: shows raw key events + the edge filter's verdict for 40 s, logs to D:\KEYLOG.TXT, then exits. */
#include "hama.h"
#include "gfx.h"
#include "keys.h"
static const char HX[]="0123456789abcdef";
static char *hx(char *o,u32 v,int n){ for(int i=0;i<n;i++) o[i]=HX[(v>>(4*(n-1-i)))&15]; o[n]=0; return o+n; }
int main(unsigned base){
    gfx_init(); keys_init();
    u32 *task=(u32*)sys_call(OSCALL(1,0x3a),0,0,0,0);
    void *lf=sys_fopen("D:\\KEYLOG.TXT",FMODE_WRITE);
    gfx_clear(TN_BG);
    gfx_text(40,30,"keytest: press buttons (remote and frame). exits by itself after 40 s.",TN_FG,TN_BG);
    char l[96]; char *p;
    p=l; p=hx(p,ticks_now(),8); *p++=' '; p=hx(p,task?task[3]:0,8); *p=0;
    gfx_text(40,60,"ticks/queue:",TN_DIM,TN_BG); gfx_text(180,60,l,TN_DIM,TN_BG);
    if(lf){ sys_fwrite("start ",6,lf); sys_fwrite(l,17,lf); sys_fwrite("\r\n",2,lf); }
    u32 t0=ticks_now(); int row=0, polls=0;
    while(ticks_now()-t0 < 4000 && polls<4000){
        polls++;
        u32 m[8]={0};
        long r=sys_call(OSCALL(1,0x17),task?task[3]:0,(long)m,20,0);
        if(r==10) continue;
        int code=m[1]&0xffff;
        p=l; p=hx(p,ticks_now()-t0,5); *p++=' '; *p++='t'; *p++='='; p=hx(p,m[0]&0xff,2);
        *p++=' '; *p++='c'; *p++='='; p=hx(p,code,4); *p++=' '; *p++='r'; *p++='='; p=hx(p,(u32)r,2); *p=0;
        int y=100+(row%20)*22; gfx_fill(40,y,600,20,TN_BG); gfx_text(40,y,l,(m[0]&0xff)==22?TN_GREEN:TN_DIM,TN_BG); row++;
        if(lf){ int n=0; while(l[n]) n++; sys_fwrite(l,n,lf); sys_fwrite("\r\n",2,lf); }
    }
    p=l; p=hx(p,ticks_now()-t0,8); *p=0;
    if(lf){ sys_fwrite("end ",4,lf); sys_fwrite(l,8,lf); sys_fwrite("\r\n",2,lf); sys_fclose(lf); }
    gfx_text(40,560-20,"done - back to the calendar",TN_ORANGE,TN_BG);
    sys_sleep(100);
    return 0;
}
