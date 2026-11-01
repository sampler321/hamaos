/* Probe 3: dump candidate framebuffers, then calibrate buttons into D:\KEYS.TXT */
#include "hama.h"
static void *lf;
static int slen(const char *s){int n=0;while(s[n])n++;return n;}
static void wr(void *f,const char *s){ if(f) sys_fwrite(s,slen(s),f); }
static void logs(const char *s){ wr(lf,s); }
static char *hex(u32 v,int digits,char *b){ for(int i=0;i<digits;i++){int d=(v>>(4*(digits-1-i)))&15; b[i]=d<10?'0'+d:'a'+d-10;} b[digits]=0; return b; }
static void dump(const char *name, u32 start, u32 len, char *tmp){
    char h[9]; void *f = sys_fopen(name, FMODE_WRITE);
    logs("dump "); logs(name); logs(" @"); logs(hex(start,8,h));
    if(!f){ logs(" OPEN FAILED\r\n"); return; }
    for(u32 a=start; a<start+len; a+=4096){ const u8 *s=(const u8*)a; for(int i=0;i<4096;i++) tmp[i]=s[i]; sys_fwrite(tmp,4096,f); }
    sys_fclose(f); logs(" ok\r\n");
}
static void say(const char *s,int x,int y){ gui_text(s,x,y); lcm_update(0,0,800,480); }
/* wait for a key message; returns code or -1 after `tries` timeouts of 100 ticks */
static int getkey(u32 q,int tries){
    for(int t=0;t<tries;t++){
        u32 m[8]={0}; long r=sys_call(OSCALL(1,0x17),q,(long)m,100,0);
        if(r!=10 && (m[0]&0xff)==22) return m[1]&0xffff;
    }
    return -1;
}
static void drain(u32 q){ for(int i=0;i<6;i++){ u32 m[8]; if(sys_call(OSCALL(1,0x17),q,(long)m,10,0)==10) break; } }
int main(unsigned base){
    char h[9];
    lf = sys_fopen("D:\\PROBE3.TXT", FMODE_WRITE);
    logs("probe3 start\r\n");
    char *tmp = sys_malloc(4096);
    u32 *task = (u32*)sys_call(OSCALL(1,0x3a),0,0,0,0);
    say("PROBE 3: saving screen memory (about 20 s)...", 40, 40);
#ifndef TEST
    if(tmp){
        dump("D:\\FB_BFDFA0.BIN", 0xa0bfdfa0, 0x120000, tmp);
        dump("D:\\FB_E8F000.BIN", 0xa0e8f000, 0x120000, tmp);
        dump("D:\\FB_DFEFE0.BIN", 0xa0dfefe0, 0x120000, tmp);
    }
#endif
    void *qbuf = sys_malloc(20*64);
    u32 q = (qbuf && task) ? (u32)sys_call(OSCALL(1,0x14),(long)qbuf,20,0,0) : 0;
    if(q) task[3]=q;
    logs("queue "); logs(hex(q,8,h)); logs("\r\n");
    static const char *names[] = {"UP","DOWN","LEFT","RIGHT","OK","BACK","MENU"};
    void *kf = sys_fopen("D:\\KEYS.TXT", FMODE_WRITE);
    say("BUTTON SETUP - use the remote (or frame buttons).", 40, 90);
    drain(q);
    for(int i=0;i<7 && q;i++){
        char line[64]; int k=0; const char *p="Press "; while(*p) line[k++]=*p++;
        p=names[i]; while(*p) line[k++]=*p++;
        p="  (10 s to skip)"; while(*p) line[k++]=*p++; line[k]=0;
        say(line, 40, 140+i*45);
        int c = getkey(q, 10);
        k=0; p=names[i]; while(*p) line[k++]=*p++; line[k++]=' ';
        if(c<0){ p="none"; while(*p) line[k++]=*p++; line[k]=0; }
        else { hex(c,4,line+k); }
        wr(kf,line); wr(kf,"\r\n"); logs(line); logs("\r\n");
        say(c<0 ? "skipped" : "OK", 600, 140+i*45);
        drain(q);
    }
    if(kf) sys_fclose(kf);
    say("Setup done. Bring the card back to the Mac.", 40, 470);
    sys_sleep(200);
    logs("probe3 end\r\n");
    if(lf) sys_fclose(lf);
    return 0;
}
