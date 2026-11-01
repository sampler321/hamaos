#include "keys.h"
int key_alt[K_COUNT][KEY_ALTS] = {          /* defaults from KEYLOG: remote id, frame-button id; KEYS.TXT overrides */
    {0x1b,0x05,-1,-1},{0x1d,0x06,-1,-1},{0x00,0x03,-1,-1},{0x20,0x04,-1,-1},{0x1f,0x08,-1,-1},{0x19,0x02,-1,-1} };
int key_id[K_COUNT];
static u32 q, *task;
static const char *names[K_COUNT] = {"UP","DOWN","LEFT","RIGHT","OK","BACK"};
const char *key_name(int k){ return (k>=0&&k<K_COUNT)?names[k]:"?"; }
u32 ticks_now(void){ return (u32)sys_call(OSCALL(1,0x11),0,0,0,0); }
static void sync_primary(void){ for(int k=0;k<K_COUNT;k++) key_id[k]=key_alt[k][0]; }
static int hexv(char c){ return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1; }
void keys_attach(void){ if(task&&q) task[3]=q; }
static void parse(char *p){
    while(*p){
        char *nm=p; while(*p&&*p!=' '&&*p!='\r'&&*p!='\n') p++;
        if(*p!=' '){ while(*p&&*p!='\n')p++; if(*p)p++; continue; }
        *p++=0;
        int k=-1; for(int j=0;j<K_COUNT;j++){ const char *a=names[j]; int i=0; while(a[i]&&a[i]==nm[i]) i++; if(!a[i]&&!nm[i]) k=j; }
        int n=0;
        if(k>=0) for(int i=0;i<KEY_ALTS;i++) key_alt[k][i]=-1;
        while(*p&&*p!='\r'&&*p!='\n'){
            while(*p==' ') p++;
            int v=0,ok=0; while(hexv(*p)>=0){ v=v*16+hexv(*p); p++; ok=1; }
            if(ok && k>=0 && n<KEY_ALTS) key_alt[k][n++]=v&0x3f;
            while(*p&&*p!=' '&&*p!='\r'&&*p!='\n') p++;
        }
        while(*p&&*p!='\n') p++; if(*p) p++;
    }
}
int keys_init(void){
    if(!q){
        task=(u32*)sys_call(OSCALL(1,0x3a),0,0,0,0);
        void *qb=sys_malloc(20*64);
        q = (task&&qb) ? (u32)sys_call(OSCALL(1,0x14),(long)qb,20,0,0) : 0;
        char b[256]; for(int i=0;i<256;i++) b[i]=0;
        void *f=sys_fopen("D:\\KEYS.TXT",FMODE_READ);
        if(f){ sys_fread(b,255,f); sys_fclose(f); parse(b); }
    }
    keys_attach(); sync_primary();
    return q && key_id[K_UP]>=0 && key_id[K_DOWN]>=0 && key_id[K_OK]>=0;
}
/* --- one press = one event ------------------------------------------------------------
 * Measured (KEYLOG): a remote press sends 0x40|id, id, 0xC0|id, then an auto-repeat burst
 * 0x80|id, id ... 0xC0|id on release. Frame buttons send only 0x40|id.
 * So: a press is exactly an event with flags 0x40; everything else is ignored.          */
static int next_event(int ticks,int *id,int *flags){
    u32 m[8]; long r=sys_call(OSCALL(1,0x17),q,(long)m,ticks,0);
    if(r==10 || (m[0]&0xff)!=22) return 0;
    int code=m[1]&0xffff; *id=code&0x3f; *flags=code&0xc0; return 1;
}
static int edge(int id,int flags){ (void)id; return flags==0x40; }
int keys_poll(int ticks){
    if(!q) return -1;
    int id,fl;
    if(!next_event(ticks,&id,&fl)) return -1;
    if(!edge(id,fl)) return -2;
    for(int k=0;k<K_COUNT;k++) for(int i=0;i<KEY_ALTS;i++) if(key_alt[k][i]==id) return k;
    if(key_id[K_OK]<0) return K_OK;                        /* never lock the user out */
    return 100+id;
}
int keys_raw_press(int ticks){
    u32 start=ticks_now(); int id,fl;
    while(ticks_now()-start < (u32)ticks){
        if(next_event(10,&id,&fl) && fl==0x40){
            u32 t=ticks_now();                       /* swallow this press's echo/repeat burst */
            while(ticks_now()-t < 25){ int i2,f2; if(next_event(5,&i2,&f2) && f2==0x40 && i2!=id) break; }
            return id;
        }
    }
    return -1;
}
void keys_save(void){
    void *f=sys_fopen("D:\\KEYS.TXT",FMODE_WRITE); if(!f) return;
    static const char HX[]="0123456789abcdef";
    for(int k=0;k<K_COUNT;k++){
        char l[40]; int n=0; const char *a=names[k]; while(*a) l[n++]=*a++;
        int any=0;
        for(int i=0;i<KEY_ALTS;i++) if(key_alt[k][i]>=0){ l[n++]=' '; l[n++]=HX[(key_alt[k][i]>>4)&15]; l[n++]=HX[key_alt[k][i]&15]; any=1; }
        if(!any){ l[n++]=' '; l[n++]='-'; }
        while(n<24) l[n++]=' ';                 /* pad: the FS overwrites without truncating */
        l[n++]='\r'; l[n++]='\n'; sys_fwrite(l,n,f);
    }
    sys_fclose(f);
}
