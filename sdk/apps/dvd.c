/* Bouncing disc-badge screensaver. Colour changes on every bounce; corner hits are counted. */
#include "hama.h"
#include "gfx.h"
#include "keys.h"

#define TOP 48
#define BW  184
#define BH  112
#define R   12
#define CS  8                         /* letter cell size */
static int want_quit(void){
    for(;;){ int k=keys_poll(1); if(k==-1) return 0;
        if(k==K_BACK || (k>=100 && key_id[K_BACK]<0)) return 1; }
}
static void header(const char *name,const char *hint){
    gfx_fill(0,0,1024,44,TN_BG2); gfx_fill(0,44,1024,2,TN_DIM);
    int x=gfx_text(16,14,"~/apps",TN_BLUE,TN_BG2); x=gfx_text(x,14," $ ",TN_DIM,TN_BG2);
    x=gfx_text(x,14,name,TN_GREEN,TN_BG2); gfx_text(x+30,14,hint,TN_DIM,TN_BG2);
}
static const unsigned char LET_D[7]={0x1e,0x11,0x11,0x11,0x11,0x11,0x1e};
static const unsigned char LET_V[7]={0x11,0x11,0x11,0x11,0x0a,0x0a,0x04};
/* quarter-circle insets for the rounded corners, R rows */
static const unsigned char INSET[R]={8,6,4,3,2,2,1,1,0,0,0,0};
static void letter(int x,int y,const unsigned char *g,color c){
    for(int j=0;j<7;j++){ int i=0;
        while(i<5){ if(!(g[j]&(0x10>>i))){ i++; continue; }
            int s=i; while(i<5 && (g[j]&(0x10>>i))) i++;
            gfx_fill(x+s*CS,y+j*CS,(i-s)*CS,CS,c); } }
}
static void badge(int x,int y,color c){
    for(int j=0;j<R;j++){ int in=INSET[j];
        if(in){ gfx_fill(x,y+j,in,1,TN_BG); gfx_fill(x+BW-in,y+j,in,1,TN_BG);
                gfx_fill(x,y+BH-1-j,in,1,TN_BG); gfx_fill(x+BW-in,y+BH-1-j,in,1,TN_BG); }
        gfx_fill(x+in,y+j,BW-2*in,1,c); gfx_fill(x+in,y+BH-1-j,BW-2*in,1,c);
    }
    gfx_fill(x,y+R,BW,BH-2*R,c);
    int lx=x+20, ly=y+14;
    letter(lx,ly,LET_D,TN_BG); letter(lx+52,ly,LET_V,TN_BG); letter(lx+104,ly,LET_D,TN_BG);
    gfx_fill(x+36,y+76,BW-72,24,TN_BG);
    gfx_text(x+BW/2-25,y+78,"VIDEO",c,TN_BG);
}
static void num(char *p,int v,int n){ for(int i=n-1;i>=0;i--){ p[i]='0'+v%10; v/=10; } }
int main(unsigned base){
    gfx_init(); keys_init();
    color pal[7]={TN_BLUE,TN_GREEN,TN_ORANGE,TN_RED,TN_PURPLE,TN_CYAN,RGB(0xe0,0xaf,0x68)};
    gfx_clear(TN_BG); header("./dvd","[back] quit");
    int x=100, y=TOP+60, dx=4, dy=4, ci=0, corners=0, bounces=0;
    const int MAXX=1024-BW, MINY=TOP, MAXY=576-BH;
    char st[]="bounces 0000  corners 000";
    gfx_text(1024-16-25*CHAR_W,14,st,TN_ORANGE,TN_BG2);
    badge(x,y,pal[ci]);
    for(int frame=1;;frame++){
        if(want_quit()) return 0;
        int nx=x+dx, ny=y+dy, hx=0, hy=0;
        if(nx<=0){ nx=0; dx=-dx; hx=1; } else if(nx>=MAXX){ nx=MAXX; dx=-dx; hx=1; }
        if(ny<=MINY){ ny=MINY; dy=-dy; hy=1; } else if(ny>=MAXY){ ny=MAXY; dy=-dy; hy=1; }
        /* erase only the strips the badge uncovers */
        if(nx>x) gfx_fill(x,y,nx-x,BH,TN_BG); else if(nx<x) gfx_fill(nx+BW,y,x-nx,BH,TN_BG);
        if(ny>y) gfx_fill(x,y,BW,ny-y,TN_BG); else if(ny<y) gfx_fill(x,ny+BH,BW,y-ny,TN_BG);
        x=nx; y=ny;
        if(hx||hy){
            ci=(ci+1+(frame%3==0))%7; bounces++;
            if(hx&&hy) corners++;
            num(st+8,bounces,4); num(st+22,corners,3);
            gfx_text(1024-16-25*CHAR_W,14,st,hx&&hy?TN_GREEN:TN_ORANGE,TN_BG2);
        }
        badge(x,y,pal[ci]);
        if(frame%50==0) gfx_present();
        sys_sleep(4);
    }
}
