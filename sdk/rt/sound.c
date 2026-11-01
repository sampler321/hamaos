#include "sound.h"
#define RING 16384
static int ok;
int snd_init(int volume){
    if(ok) return 1;
    void *buf=mem_alloc(RING+32); if(!buf) return 0;
    sys_call(OSCALL(0x10,0x09),(long)buf,RING,0,0);
    static int src[4]={0,0,0,1};  sys_call(OSCALL(0x10,0x01),0x80016406,(long)src,0,0);
    static int fmt[4]={11,4,1,1}; sys_call(OSCALL(0x10,0x01),0x80016401,(long)fmt,0,0);   /* 22050, mono, 16-bit */
    sys_call(OSCALL(0x10,0x01),0x80016402,volume,0,0);
    ok=1; return 1;
}
int snd_queued(void){ return ok ? (int)sys_call(OSCALL(0x10,0x08),0,0,0,0) : 0; }
int snd_write(const short *pcm,int samples){
    if(!ok) return 0;
    int len=(samples*2)&~3;
    if(len<=0) return 0;
    return sys_call(OSCALL(0x10,0x06),(long)pcm,len,0,0)==len ? len/2 : 0;
}
void snd_stop(void){
    if(!ok) return;
    for(int i=0;i<50 && snd_queued()>100;i++) sys_sleep(2);
    sys_call(OSCALL(0x10,0x01),0x40046415,0,0,0);
    ok=0;
}
void snd_tone(int hz,int ms,int vol){
    static short b[1024];
    if(!ok) return;
    int n=SND_RATE*ms/1000, ph=0, step=hz*65536/SND_RATE;
    while(n>0){
        int k=n<1024?n:1024;
        for(int i=0;i<k;i++){ ph=(ph+step)&0xffff; int env=vol*(n-i)/ (n>0?n:1); b[i]=(ph<0x8000)?env:-env; }
        for(int t=0;t<100 && !snd_write(b,k);t++) sys_sleep(1);
        n-=k;
    }
}
