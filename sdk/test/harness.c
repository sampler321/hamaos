/* Emulates the frame's OS calls under qemu-mipsel so RUN.BIN payloads can be tested on the Mac. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <ucontext.h>
static unsigned char *g_buf;
static void segv(int sig, siginfo_t *si, void *uc){ ucontext_t *u=uc; unsigned long pc=u->uc_mcontext.pc; printf("SEGV at pc=%#lx (payload offset %#lx) addr=%p ra=%#llx\n",pc,pc-(unsigned long)g_buf,si->si_addr,(unsigned long long)u->uc_mcontext.gregs[31]); exit(3);} 
static void *xalloc(long n){ return mmap(0,(n+0xfff)&~0xfff,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0); }
unsigned g_id;
extern int disp(void);
static void path(const char *p, char *out){ const char *q=p; const char *root="sd";
  if((q[0]=='C'||q[0]=='c')&&q[1]==':'){ root="internal"; q+=3; } else if((q[0]=='D'||q[0]=='d')&&q[1]==':') q+=3;
  sprintf(out,"%s/%s",root,q); for(char*c=out;*c;c++) if(*c=='\\')*c='/'; }
long handle(long a0,long a1,long a2,long a3){
  unsigned m=g_id>>16,f=g_id&0xffff; char p[512];
  switch(g_id){
  case 0x30004: path((char*)a0,p); { FILE*fp=fopen(p,a1==1?"rb":"wb"); printf("  fopen(%s,%ld) -> %p\n",(char*)a0,a1,(void*)fp); return (long)fp; }
  case 0x30006: return fclose((FILE*)a0);
  case 0x30009: return fread((void*)a0,1,a1,(FILE*)a2);
  case 0x3000a: return fwrite((void*)a0,1,a1,(FILE*)a2);
  case 0x10028: { static long live=0; void*r=xalloc(a0); live+=a0; printf("  malloc(%ld) -> %p\n",a0,r); return (long)r; }
  case 0x10029: return 0;
  case 0x1000f: return 0;
  case 0x2002c: case 0x20036: case 0x2003a: case 0x2003c: case 0x2004d: case 0x20058: return 0x1234;
  case 0x10014: return 0x777;
  case 0x7f0001: { static int n=0; char fn[64]; sprintf(fn,"shots/shot%03d.ppm",n++); FILE*o=fopen(fn,"wb");
      if(o){ unsigned char*fb=(unsigned char*)a0; fprintf(o,"P6 %ld %ld 255\n",a1,a2);
        for(long i=0;i<a1*a2;i++){ int Y=fb[2*i]-16, U=fb[(2*i&~3)+1]-128, V=fb[(2*i&~3)+3]-128;
          int r=(298*Y+409*V+128)>>8, g=(298*Y-100*U-208*V+128)>>8, b=(298*Y+516*U+128)>>8;
          fputc(r<0?0:r>255?255:r,o); fputc(g<0?0:g>255?255:g,o); fputc(b<0?0:b>255?255:b,o);} fclose(o);} return 0; }
  case 0x3002e: { unsigned char*b=(unsigned char*)a1; b[0]='C'; b[4]=2; b[20]='D'; b[24]=16; return 0; }
  case 0x1002c: { char p[512]; sprintf(p,"lfi/%s",(char*)a0); FILE*fp=fopen(p,"rb"); printf("  lfi_open(%s) -> %p\n",(char*)a0,(void*)fp); return (long)fp; }
  case 0x10030: return fread((void*)a1,1,a2,(FILE*)a0);
  case 0x1002d: return fclose((FILE*)a0);
  case 0x100009: case 0x100001: return 0;
  case 0x100006: { static long tot=0; tot+=a1; if(tot%220500<a1) printf("  [audio] %ld bytes streamed\n",tot); return a1; }
  case 0x100008: return 0;
  case 0x10011: { static unsigned tk=0; tk+=3; return tk; }
  case 0x1003a: { static unsigned t[16]={0,1,2,0xabc}; return (long)t; }
  case 0x10017: { static int k=0; unsigned *m=(unsigned*)a1; k++;
      if(k==20){m[0]=22;m[1]=0x40|0x1d;return 1;}            /* DOWN  -> matrix */
      if(k==25){m[0]=22;m[1]=0x40|0x1f;return 1;}            /* OK    -> run it */
      if(k==120){m[0]=22;m[1]=0x40|0x19;return 1;}           /* BACK  -> quit app */
      if(k==140){m[0]=22;m[1]=0x40|0x1f;return 1;}           /* OK    -> run again */
      if(k==260){m[0]=22;m[1]=0x40|0x19;return 1;}
      return 10; }
  case 0x10032: printf("  [printf] %s",(char*)a0); return 0;
  case 0x20004: printf("  [TEXT %ld,%ld] %s\n",a1,a2,(char*)a0); return 0;
  case 0x60010: printf("  [SHOW %ld,%ld %ldx%ld]\n",a0,a1,a2,a3); return 0;
  }
  printf("  !! unhandled OS call %x:%x\n",m,f); exit(2);
}
int main(int argc,char**argv){ setvbuf(stdout,0,_IONBF,0);
  mmap((void*)0x100000,0x110000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0);
  mmap((void*)0x700000,0x10000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0);
  FILE*f=fopen(argv[1],"rb"); unsigned char *raw=xalloc(0x30000); size_t n=fread(raw,1,0x30000-0x2000,f); fclose(f);
  unsigned char *buf=(unsigned char*)(((unsigned long)raw+0xfff0)&~3UL);   /* deliberately unaligned load address */
  memmove(buf,raw,n); g_buf=buf;
  { struct sigaction sa={0}; sa.sa_sigaction=segv; sa.sa_flags=SA_SIGINFO; sigaction(SIGSEGV,&sa,0); }
  printf("loaded %zu bytes at %p\n",n,buf);
  int (*entry)(void*,void*)=(void*)buf;
  int r=entry((void*)disp,buf);
  printf("payload returned %d\n",r); return 0;
}
