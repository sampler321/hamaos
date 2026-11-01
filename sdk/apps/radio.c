/* hamaOS radio: an original 124 BPM chiptune house loop synthesised live, with a level meter. */
#include "hama.h"
#include "gfx.h"
#include "keys.h"
#include "sound.h"
#define BPM 124
#define STEP (SND_RATE*60/BPM/4)            /* samples per 16th note */
/* A minor house progression: Am  F  C  G  (bass roots, MIDI notes) */
static const unsigned char roots[4]={45,41,48,43};
static const signed char arp[8]={0,7,12,15,12,7,3,7};          /* minor-ish arpeggio offsets */
static const unsigned char bassline[16]={1,0,0,1, 0,0,1,0, 1,0,0,1, 0,1,0,0};
static int freq_q16(int midi){            /* phase increment (Q16 per sample) for a MIDI note */
    static const unsigned short base[12]={ 8372,8870,9397,9956,10548,11175,11840,12544,13290,14080,14917,15804 }; /* C8..B8 Hz*1 */
    int oct=midi/12, n=midi%12; int hz_x1=base[n];                 /* C8 = MIDI 108 */
    int sh=9-oct;                                                  /* 108/12=9 */
    long long inc=((long long)hz_x1<<16)/SND_RATE;
    return (int)(sh>0 ? inc>>sh : inc<<(-sh));
}
static u32 lfsr=0xace1u;
static int noise(void){ lfsr=(lfsr>>1)^(-(int)(lfsr&1)&0xb400u); return (int)(lfsr&0xffff)-32768; }

int main(unsigned base){
    gfx_init(); keys_init();
    gfx_clear(TN_BG);
    gfx_fill(0,0,1024,44,TN_BG2); gfx_fill(0,44,1024,2,TN_DIM);
    int x=gfx_text(16,12,"~/apps",TN_BLUE,TN_BG2); x=gfx_text(x,12," $ ",TN_DIM,TN_BG2); x=gfx_text(x,12,"./radio",TN_GREEN,TN_BG2);
    gfx_text(x+30,12,"[up/down] volume  [back] quit",TN_DIM,TN_BG2);
    gfx_text(60,90,"now playing: hamaOS - night drive (original, 124 bpm)",TN_FG,TN_BG);
    int vol=20;
    if(!snd_init(vol)){ gfx_text(60,140,"audio init failed",TN_RED,TN_BG); sys_sleep(200); return 0; }
    short *buf=sys_malloc(STEP*2+8); if(!buf) return 0;
    int step=0, kph=0, kenv=0, bph=0, bnote=45, lph=0, hat=0, bar_h[16]={0};
    for(;;){
        /* render one 16th note */
        int bar=(step/16)%4, s16=step%16, root=roots[bar];
        if(s16%4==0){ kenv=STEP*2; kph=0; }                        /* kick every beat */
        if(s16%4==2) hat=STEP/3;                                    /* open-ish hat on the off-beat */
        if(s16%2==1 && s16%4!=2) hat=STEP/8;                        /* closed hats */
        int bass_on=bassline[s16]; if(bass_on) bnote=root-12+(s16==6||s16==13?12:0);
        int binc=freq_q16(bnote), linc=freq_q16(root+24+arp[(step)%8]);
        int peak=0;
        for(int i=0;i<STEP;i++){
            int smp=0;
            if(kenv>0){ int f=60+ (kenv*140)/(STEP*2);              /* pitch sweep 200 -> 60 Hz */
                kph=(kph+ f*65536/SND_RATE)&0xffff;
                int tri = kph<0x8000 ? kph-0x4000 : 0xc000-kph;     /* triangle */
                smp += tri*kenv/(STEP*2)/2; kenv--; }
            if(hat>0){ smp += noise()*hat/(STEP*3)/3; hat--; }
            if(bass_on || i<STEP/2){ bph=(bph+binc)&0xffff; int sq=bph<0x8000?5000:-5000; smp+= (i<STEP*3/4)?sq:sq/3; }
            lph=(lph+linc)&0xffff; smp += ((lph<0x4000)?2200:-2200) * (STEP-i)/STEP;   /* plucky pulse arp */
            if(smp>32767) smp=32767; if(smp<-32768) smp=-32768;
            buf[i]=(short)smp; int a=smp<0?-smp:smp; if(a>peak) peak=a;
        }
        /* stream it (wait while the ring is full) */
        int sent=0, guard=0;
        while(sent<STEP && guard++<400){
            int n=snd_write(buf+sent,STEP-sent);
            if(n) sent+=n; else {
                int k=keys_poll(1);
                if(k==K_BACK || (k>=100 && key_id[K_BACK]<0)){ snd_stop(); return 0; }
                if(k==K_UP && vol<31){ vol++; sys_call(OSCALL(0x10,0x01),0x80016402,vol,0,0); }
                if(k==K_DOWN && vol>0){ vol--; sys_call(OSCALL(0x10,0x01),0x80016402,vol,0,0); }
            }
        }
        /* level meter: one bar per 16th, redraw only the bar that changed */
        int h=peak*180/32768, bx=60+s16*56, by=420;
        if(h!=bar_h[s16]){
            gfx_fill(bx,by-200,44,200,TN_BG);
            color c = s16%4==0 ? TN_PURPLE : (s16%2 ? TN_CYAN : TN_BLUE);
            gfx_fill(bx,by-h,44,h,c); bar_h[s16]=h;
        }
        char vb[16]="vol 00"; vb[4]='0'+vol/10; vb[5]='0'+vol%10; gfx_text(60,450,vb,TN_DIM,TN_BG);
        if((step&15)==0) gfx_present();
        step++;
    }
}
