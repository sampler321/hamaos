# Calendar hook v4: run D:\RUN.BIN from the SD card if present, otherwise RUN.BIN built into the
# firmware (LFI, via kernel calls 1:0x2c open / 1:0x30 read / 1:0x2d close). Payload gets
# a0 = OS dispatcher, a1 = its load address. Then the real calendar main runs.
        .set noreorder
        .set mips32
        .set noat
        .equ ORIGMAIN, 0x01607804

        .section .cA,"ax"              # 92 @ 0x01609400
hook:   addiu $sp,$sp,-32
        sw    $ra,28($sp)
        sw    $a0,24($sp)
        sw    $s0,20($sp)
        sw    $s1,16($sp)
        sw    $s2,12($sp)
        lui   $s1,0x8000
        ori   $s1,$s1,0x300
        lui   $a0,%hi(pathD)
        addiu $a0,$a0,%lo(pathD)
        li    $a1,1                    # fp = fopen("D:\RUN.BIN", read)
        lui   $v1,3
        jalr  $s1
        ori   $v1,$v1,4
        beqz  $v0,tryLFI
        move  $s0,$v0
        lui   $a0,1                    # malloc(64K)
        lui   $v1,1
        jalr  $s1
        ori   $v1,$v1,0x28
        j     cB
        nop

        .section .cB,"ax"              # 72 @ 0x01609cd4
cB:     beqz  $v0,closeFS
        move  $s2,$v0
        move  $a0,$s2                  # fread(buf, 64K, fp)
        lui   $a1,1
        move  $a2,$s0
        lui   $v1,3
        jalr  $s1
        ori   $v1,$v1,9
        move  $a0,$s0                  # fclose(fp)
        lui   $v1,3
        jalr  $s1
        ori   $v1,$v1,6
        j     run
        nop

        .section .cC,"ax"              # 80 @ 0x01609d60
run:    lui   $t0,0x8000               # index writeback/invalidate D, invalidate I
        lui   $t1,0x8001
1:      cache 0x01,0($t0)
        cache 0x00,0($t0)
        addiu $t0,$t0,16
        bne   $t0,$t1,1b
        nop
        sync
        move  $a0,$s1                  # payload(dispatcher, buf)
        jalr  $s2
        move  $a1,$s2
        move  $a0,$s2                  # free(buf)
        lui   $v1,1
        jalr  $s1
        ori   $v1,$v1,0x29
        j     done
        nop

        .section .cD,"ax"              # 60 @ 0x01609728
done:   lw    $a0,24($sp)
        lw    $s2,12($sp)
        lw    $s1,16($sp)
        lw    $s0,20($sp)
        lw    $ra,28($sp)
        j     ORIGMAIN
        addiu $sp,$sp,32
closeFS: move $a0,$s0
        lui   $v1,3
        jalr  $s1
        ori   $v1,$v1,6
        j     done
        nop

        .section .cE,"ax"              # 60 @ 0x01609474
tryLFI: lui   $a0,%hi(nameL)
        addiu $a0,$a0,%lo(nameL)
        move  $a1,$zero                # h = sys_open("RUN.BIN", 0)  (firmware file)
        lui   $v1,1
        jalr  $s1
        ori   $v1,$v1,0x2c
        beqz  $v0,done
        move  $s0,$v0
        lui   $a0,1                    # malloc(64K)
        lui   $v1,1
        jalr  $s1
        ori   $v1,$v1,0x28
        j     cF
        nop

        .section .cF,"ax"              # 56 @ 0x01609c1c
cF:     beqz  $v0,closeL
        move  $s2,$v0
        move  $a0,$s0                  # sys_read(h, buf, 64K)
        move  $a1,$s2
        lui   $a2,1
        lui   $v1,1
        jalr  $s1
        ori   $v1,$v1,0x30
        move  $a0,$s0                  # sys_close(h)
        lui   $v1,1
        jalr  $s1
        ori   $v1,$v1,0x2d
        j     run
        nop

        .section .cG,"ax"              # 52 @ 0x01609530
closeL: move  $a0,$s0
        lui   $v1,1
        jalr  $s1
        ori   $v1,$v1,0x2d
        j     done
        nop

        .section .str,"a"              # 32 @ 0x016093ac
pathD:  .asciz "D:\\RUN.BIN"
        .align 2
nameL:  .asciz "VD_VC1.LIB"
