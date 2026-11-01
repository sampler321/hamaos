/* Probe: dumps GUI/LCM/VRAM driver memory, logs GUI getters and task struct, records button messages. */
#include "hama.h"
static void *lf;
static int slen(const char *s){int n=0;while(s[n])n++;return n;}
static void logs(const char *s){ if(lf) sys_fwrite(s,slen(s),lf); }
static void loghex(u32 v){ char b[11]; b[0]='0'; b[1]='x'; for(int i=0;i<8;i++){int d=(v>>(28-4*i))&15; b[2+i]=d<10?'0'+d:'a'+d-10;} b[10]=0; logs(b); }
static void dump(const char *name, u32 start, u32 end, char *tmp){
    void *f = sys_fopen(name, FMODE_WRITE);
    logs("dump "); logs(name); logs(" "); loghex(start); logs("-"); loghex(end);
    if(!f){ logs(" OPEN FAILED\r\n"); return; }
    for(u32 a=start; a<end; a+=4096){
        u32 n = end-a < 4096 ? end-a : 4096;
        const u8 *src=(const u8*)a; for(u32 i=0;i<n;i++) tmp[i]=src[i];
        sys_fwrite(tmp, n, f);
    }
    sys_fclose(f); logs(" ok\r\n");
}
static char line[64];
static void show_line(int y, const char *s){ gui_text(s, 40, y); lcm_update(0,0,800,480); }
int main(unsigned base){
    lf = sys_fopen("D:\\PROBE.TXT", FMODE_WRITE);
    logs("probe start, base "); loghex(base); logs("\r\n");
    char *tmp = sys_malloc(4096);
    if(!tmp){ logs("malloc failed\r\n"); goto out; }
    u32 *task = (u32*)sys_call(OSCALL(1,0x3a),0,0,0,0);
    logs("task "); loghex((u32)task); logs("\r\n");
    /* known screen content, then dump the candidate canvas buffers */
    show_line(40,  "PROBE 2: dumping screen buffers, please wait...");
#ifndef TEST
    dump("D:\\FB_BFDFA0.BIN", 0xa0bfdfa0, 0xa0bfdfa0+0x120000, tmp);
    dump("D:\\FB_E8F000.BIN", 0xa0e8f000, 0xa0e8f000+0x120000, tmp);
    dump("D:\\FB_DFEFE0.BIN", 0xa0dfefe0, 0xa0dfefe0+0x120000, tmp);
#endif
    /* own message queue, like CALENDAR main: q = OSQCreate(buf, 20); task[3] = q */
    void **qbuf = sys_malloc(20*64);
    u32 q = qbuf && task ? (u32)sys_call(OSCALL(1,0x14),(long)qbuf,20,0,0) : 0;
    if(q) task[3] = q;
    show_line(40,  "BUTTON TEST: press each button on the frame and remote once.");
    show_line(90,  "Up, Down, Left, Right, Enter/OK, Back/Esc, Menu, Play...");
    show_line(140, "It stops after 24 presses or about 60 seconds.");
    int presses=0, y=200;
    logs("queue "); loghex(q); logs("\r\n");
    for(int it=0; it<60 && presses<24 && q; it++){
        u32 msg[8]; for(int i=0;i<8;i++) msg[i]=0;
        long r = sys_call(OSCALL(1,0x17), q, (long)msg, 100, 0);
        logs("r="); loghex(r); logs(" msg:"); for(int i=0;i<4;i++){ logs(" "); loghex(msg[i]); } logs("\r\n");
        if((msg[0]&0xff)==22){
            presses++;
            const char *h="key 0x"; int k=0; while(h[k]){line[k]=h[k];k++;}
            u32 code=msg[1]&0xffff; for(int i=0;i<4;i++){int d=(code>>(12-4*i))&15; line[k++]=d<10?'0'+d:'a'+d-10;}
            line[k]=0; gui_text(line, 40+(presses%4)*180, y+(presses/4)*40); lcm_update(0,0,800,480);
        }
    }
    show_line(440, "Button test done.");
    sys_sleep(100);
    sys_free(tmp);
out:
    logs("probe end\r\n");
    if(lf) sys_fclose(lf);
    return 0;
}
