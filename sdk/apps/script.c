/* Script runner: executes D:\SCRIPT.TXT line by line, logs to D:\LOG.TXT.
 *   text X Y message   draw text            show [X Y W H]  push canvas to screen
 *   wait SECONDS       pause (0.1 s steps)  loop [N]        restart from top (N times, default 1)
 *   log message        append to LOG.TXT    end             stop                                  */
#include "hama.h"

static void *logf;
static int slen(const char *s){int n=0;while(s[n])n++;return n;}
static void logs(const char *s){ if(logf) sys_fwrite(s,slen(s),logf); }
static void logn(int v){ char b[12]; int i=11; b[i]=0; int neg=v<0; if(neg)v=-v;
  do{ b[--i]='0'+v%10; v/=10; }while(v); if(neg)b[--i]='-'; logs(b+i); }
static int isws(char c){return c==' '||c=='\t';}
static const char *skip(const char *p){while(isws(*p))p++;return p;}
static const char *num(const char *p,int *out){int v=0,neg=0;p=skip(p);
  if(*p=='-'){neg=1;p++;} while(*p>='0'&&*p<='9'){v=v*10+(*p-'0');p++;}
  *out=neg?-v:v; return p;}
static int is(const char *p,const char *w){int i=0;while(w[i]){if(p[i]!=w[i])return 0;i++;}
  return p[i]==0||isws(p[i]);}

int main(unsigned base)
{
    logf = sys_fopen("D:\\LOG.TXT", FMODE_WRITE);
    logs("script runner started\r\n");
    void *fp = sys_fopen("D:\\SCRIPT.TXT", FMODE_READ);
    if(!fp) fp = sys_fopen("C:\\SCRIPT.TXT", FMODE_READ);
    if(!fp){ logs("no SCRIPT.TXT\r\n"); goto out; }
    char *buf = sys_malloc(16384);
    if(!buf){ logs("malloc failed\r\n"); sys_fclose(fp); goto out; }
    for(int i=0;i<16384;i++) buf[i]=0;
    int r = sys_fread(buf, 16383, fp);
    sys_fclose(fp);
    int n = slen(buf);                       /* don't trust the return value's meaning */
    logs("fread returned "); logn(r); logs(", text length "); logn(n); logs("\r\n");
    for(int i=0;i<n;i++) if(buf[i]=='\r'||buf[i]=='\n') buf[i]=0;   /* lines -> C strings */

    int loops_left = -1, pc = 0, steps = 0;
    while(pc < n && steps < 100000){
        const char *l = skip(buf+pc);
        int next = pc + slen(buf+pc) + 1;
        steps++;
        if(*l==0 || *l=='#'){ pc = next; continue; }
        if(is(l,"text")){ int x,y; const char *p=num(l+4,&x); p=num(p,&y); gui_text(skip(p),x,y); }
        else if(is(l,"show")){ int x=0,y=0,w=800,h=480; const char *p=l+4;
            if(*skip(p)){ p=num(p,&x); p=num(p,&y); p=num(p,&w); p=num(p,&h); }
            lcm_update(x,y,w,h); }
        else if(is(l,"wait")){ int s; num(l+4,&s); for(int k=0;k<s*10;k++) sys_sleep(10); }
        else if(is(l,"log")){ logs(skip(l+3)); logs("\r\n"); }
        else if(is(l,"end")) break;
        else if(is(l,"loop")){ int c; const char *p=skip(l+4); if(*p) num(p,&c); else c=1;
            if(loops_left<0) loops_left=c;
            if(loops_left-- > 0){ pc=0; continue; } }
        else { logs("unknown: "); logs(l); logs("\r\n"); }
        pc = next;
    }
    logs("script done, steps="); logn(steps); logs("\r\n");
    sys_free(buf);
out:
    if(logf) sys_fclose(logf);
    return 0;
}
