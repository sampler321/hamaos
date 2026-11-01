/* Hama 00095230 (Actions AM7331) payload SDK. Calls go through the OS dispatcher at 0x80000300. */
#ifndef HAMA_H
#define HAMA_H
typedef unsigned int u32; typedef int s32; typedef unsigned char u8;
#define FMODE_READ  1
#define FMODE_WRITE 10
void *sys_fopen(const char *path, int mode);          /* 3:0x04  path like "D:\\X.TXT" */
int   sys_fclose(void *fp);                           /* 3:0x06 */
int   sys_fread(void *buf, int len, void *fp);        /* 3:0x09 */
int   sys_fwrite(const void *buf, int len, void *fp); /* 3:0x0a */
int   sys_fseek(void *fp, int off, int whence);       /* 3:0x0b (unverified) */
void *sys_malloc(int size);                           /* 1:0x28 */
void  sys_free(void *p);                              /* 1:0x29 */
void  sys_sleep(int ticks);                           /* 1:0x0f  ~10 ms per tick */
int   sys_printf(const char *fmt, ...);               /* 1:0x32  serial console */
void  gui_text(const char *s, int x, int y);          /* 2:0x04  GUI_DispStringAt */
void  lcm_update(int x, int y, int w, int h);         /* 6:0x10  push canvas region to panel */
#endif
#ifndef HAMA_CALL
#define HAMA_CALL
long sys_call(unsigned id, long a, long b, long c, long d);
#define OSCALL(mod,fn) (((mod)<<16)|(fn))
#endif
#ifndef HAMA_LFI
#define HAMA_LFI
/* read-only files built into the firmware image (LFI) */
#define lfi_open(name)          ((void*)sys_call(OSCALL(1,0x2c),(long)(name),0,0,0))
#define lfi_read(h,buf,len)     ((int)sys_call(OSCALL(1,0x30),(long)(h),(long)(buf),(len),0))
#define lfi_close(h)            sys_call(OSCALL(1,0x2d),(long)(h),0,0,0)
#endif
#ifndef HAMA_MEM
#define HAMA_MEM
void *mem_alloc(int size);
int   mem_mark(void);
void  mem_release(int mark);
#endif
