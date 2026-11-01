/* Tracked allocations so the launcher can reclaim everything an app allocated. */
#include "hama.h"
#define MAXA 64
static void *list[MAXA]; static int n;
void *mem_alloc(int size){
    void *p=sys_malloc(size);
    if(p && n<MAXA) list[n++]=p;
    return p;
}
int  mem_mark(void){ return n; }
void mem_release(int mark){ while(n>mark){ n--; if(list[n]) sys_free(list[n]); list[n]=0; } }
