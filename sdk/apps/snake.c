/* Snake for hamaOS. Input via rt/keys (one press = one event). */
#include "hama.h"
#include "gfx.h"
#include "keys.h"
#include "sound.h"

#define CELL 24
#define GW (1024/CELL)
#define GH (576/CELL - 2)
static int key_up=K_UP,key_down=K_DOWN,key_left=K_LEFT,key_right=K_RIGHT,key_ok=K_OK,key_back=K_BACK;
static u32 rng=12345;
static u32 rnd(void){ rng = rng*1103515245u + 12345u; return rng>>16; }
/* action (K_*) of the next clean press, or -1 */
static int poll_key(int ticks){ int k=keys_poll(ticks); return (k>=0&&k<K_COUNT)?k:-1; }
static short *sx, *sy;
static void cell(int x,int y,color c){ gfx_fill(x*CELL+1,(y+2)*CELL+1,CELL-2,CELL-2,c); }
int main(unsigned base){
    gfx_init(); keys_init();
    rng ^= ticks_now();
    snd_init(18);
    sx=sys_malloc(GW*GH*2); sy=sys_malloc(GW*GH*2); if(!sx||!sy) return 0;
    const color BG=TN_BG, WALL=TN_DIM, SNAKE=TN_GREEN, FOOD=TN_RED;
    for(;;){                                   /* game loop; BACK quits to the calendar */
        int len=4, dx=1, dy=0, fx, fy, score=0, dead=0;
        for(int i=0;i<len;i++){ sx[i]=10-i; sy[i]=GH/2; }
        fx=rnd()%GW; fy=rnd()%GH;
        gfx_clear(BG); gfx_fill(0,0,1024,2*CELL-4,TN_BG2); gfx_fill(0,2*CELL-4,1024,2,WALL);
        int hx=gfx_text(16,14,"~/apps",TN_BLUE,TN_BG2); hx=gfx_text(hx,14," $ ",TN_DIM,TN_BG2);
        hx=gfx_text(hx,14,"./snake",TN_GREEN,TN_BG2); gfx_text(hx+30,14,"[arrows] steer  [back] quit",TN_DIM,TN_BG2);
        for(int i=0;i<len;i++) cell(sx[i],sy[i],SNAKE);
        cell(fx,fy,FOOD); gfx_present();
        int turns[2], nturn=0;
        while(!dead){
            /* fixed-rate step: drain input without blocking, queue up to 2 turns */
            for(;;){
                int k=poll_key(1);
                if(k<0) break;
                if(k>=0 && k==key_back){ snd_stop(); return 0; }
                int tdx=0,tdy=0;
                if(k==key_up){tdy=-1;} else if(k==key_down){tdy=1;} else if(k==key_left){tdx=-1;} else if(k==key_right){tdx=1;} else continue;
                int ldx=dx, ldy=dy; if(nturn){ ldx=turns[nturn-1]>>8; ldy=(signed char)(turns[nturn-1]&0xff); }
                if((tdx && ldx==0) || (tdy && ldy==0)){ if(nturn<2) turns[nturn++]=(tdx<<8)|(tdy&0xff); }
            }
            if(nturn){ dx=turns[0]>>8; dy=(signed char)(turns[0]&0xff); turns[0]=turns[1]; nturn--; }
            int nx=sx[0]+dx, ny=sy[0]+dy;
            if(nx<0||ny<0||nx>=GW||ny>=GH) dead=1;
            for(int i=0;i<len-1 && !dead;i++) if(sx[i]==nx&&sy[i]==ny) dead=1;
            if(dead) break;
            int grow = (nx==fx&&ny==fy);
            if(!grow) cell(sx[len-1],sy[len-1],BG);
            else { snd_tone(880,40,6000); if(len<GW*GH) len++; score++; fx=rnd()%GW; fy=rnd()%GH; cell(fx,fy,FOOD);
                   char sb[16]="score 000"; sb[6]='0'+score/100%10; sb[7]='0'+score/10%10; sb[8]='0'+score%10;
                   gfx_text(1024-16-9*CHAR_W,14,sb,TN_ORANGE,TN_BG2); }
            for(int i=len-1;i>0;i--){ sx[i]=sx[i-1]; sy[i]=sy[i-1]; }
            sx[0]=nx; sy[0]=ny; cell(nx,ny,SNAKE);
            gfx_present();
            sys_sleep(12 - (len>40 ? 6 : len/7));   /* ~120 ms per step, a bit faster as it grows */
        }
        snd_tone(220,120,7000); snd_tone(110,200,7000);
        gfx_fill(292,226,440,110,TN_BG2); gfx_fill(292,226,440,2,TN_RED);
        gfx_text(302,246,"process exited: snake collided",TN_RED,TN_BG2);
        gfx_text(302,286,"[ok] restart    [back] quit",TN_FG,TN_BG2);
        gfx_present();
        for(;;){ int k=poll_key(100); if(k>=0 && k==key_back){ snd_stop(); return 0; } if(k==key_ok) break; }
    }
}
