# OS call stubs. The dispatcher address lives in __disp (a word in this .text, set by start.S);
# each stub finds it PC-relatively, so stubs work however they are called (jalr/bal, any $t9/$gp).
        .set noreorder
        .set noat
        .text
        .globl __disp
__disp: .word 0
        .macro SYS name, mod, fn
        .globl \name
        .type \name,@function
\name:  move  $t0,$ra
        bal   L\@
        lui   $v1,\mod
L\@:    lw    $v0,(__disp-L\@)($ra)
        move  $ra,$t0
        jr    $v0
        ori   $v1,$v1,\fn
        .endm
        SYS sys_fopen,   3, 0x04
        SYS sys_fclose,  3, 0x06
        SYS sys_fread,   3, 0x09
        SYS sys_fwrite,  3, 0x0a
        SYS sys_fseek,   3, 0x0b
        SYS sys_malloc,  1, 0x28
        SYS sys_free,    1, 0x29
        SYS sys_sleep,   1, 0x0f
        SYS sys_printf,  1, 0x32
        SYS gui_text,    2, 0x04
        SYS lcm_update,  6, 0x10

# sys_call(id, a, b, c, d) -> OS call id=(module<<16)|fn with up to 4 args
        .globl sys_call
        .type sys_call,@function
sys_call:
        move  $v1,$a0
        move  $a0,$a1
        move  $a1,$a2
        move  $a2,$a3
        lw    $a3,16($sp)
        move  $t0,$ra
        bal   Lsc
        nop
Lsc:    lw    $v0,(__disp-Lsc)($ra)
        move  $ra,$t0
        jr    $v0
        nop
