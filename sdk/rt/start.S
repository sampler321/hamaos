# Runtime entry for C payloads (in-place). Called with a0 = OS dispatcher, a1 = load address.
# Applies the relocation table appended by mkrun.py, flushes caches, sets __disp, calls main(base).
# Table entries: type<<28|offset [, link value]   1=R_MIPS_32 2=R_MIPS_26 3=HI16(v) 4=LO16(v)
        .set noreorder
        .set mips32
        .section .text.start,"ax"
        .globl _start
_start: b     1f
        nop
        .word 0                        # +8  reloc table offset   (mkrun.py)
        .word 0                        # +12 total image size     (mkrun.py)
1:      addiu $sp,$sp,-32
        sw    $ra,28($sp)
        sw    $s1,24($sp)
        sw    $s2,20($sp)
        move  $s2,$a0                  # dispatcher
        move  $s1,$a1                  # base
        lw    $t0,8($s1)
        addu  $t8,$s1,$t0              # t8 -> table
2:      lw    $t0,0($t8)
        beqz  $t0,9f
        addiu $t8,$t8,4
        srl   $t1,$t0,28
        sll   $t2,$t0,4
        srl   $t2,$t2,4
        addu  $t2,$t2,$s1              # word to patch
        lw    $t3,0($t2)
        lui   $t6,0xffff
        li    $t4,1
        bne   $t1,$t4,3f
        li    $t4,2
        addu  $t3,$t3,$s1              # R_MIPS_32
        b     2b
        sw    $t3,0($t2)
3:      bne   $t1,$t4,4f
        li    $t4,3
        sll   $t5,$t3,6                # R_MIPS_26: target=(imm<<2)+base
        srl   $t5,$t5,4
        addu  $t5,$t5,$s1
        sll   $t5,$t5,4
        srl   $t5,$t5,6
        lui   $t7,0xfc00
        and   $t3,$t3,$t7
        or    $t3,$t3,$t5
        b     2b
        sw    $t3,0($t2)
4:      lw    $t5,0($t8)               # link value
        addiu $t8,$t8,4
        addu  $t5,$t5,$s1              # runtime value
        bne   $t1,$t4,5f
        nop
        lui   $t7,0                    # HI16: (v + 0x8000) >> 16
        addiu $t5,$t5,0x7fff
        addiu $t5,$t5,1
        srl   $t5,$t5,16
5:      andi  $t5,$t5,0xffff           # (LO16: v & 0xffff)
        and   $t3,$t3,$t6
        or    $t3,$t3,$t5
        b     2b
        sw    $t3,0($t2)
9:
#ifndef TEST
        lui   $t0,0x8000               # flush caches by index (no TLB involvement)
        lui   $t1,0x8001
6:      cache 0x01,0($t0)
        cache 0x00,0($t0)
        addiu $t0,$t0,16
        bne   $t0,$t1,6b
        nop
        sync
#endif
        bal   getw                     # ra = &main_word (already relocated)
        nop
main_word: .word main
disp_word: .word __disp
getw:   lw    $t1,4($ra)
        sw    $s2,0($t1)               # __disp = dispatcher
        lw    $t9,0($ra)
        jalr  $t9
        move  $a0,$s1
        lw    $ra,28($sp)
        lw    $s1,24($sp)
        lw    $s2,20($sp)
        jr    $ra
        addiu $sp,$sp,32
