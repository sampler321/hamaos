        .set noreorder
        .text
        .globl disp
        .ent disp
disp:   lui   $t0,%hi(g_id)
        sw    $v1,%lo(g_id)($t0)
        lui   $t9,%hi(handle)
        addiu $t9,$t9,%lo(handle)
        jr    $t9
        nop
        .end disp
