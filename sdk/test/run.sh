#!/bin/sh
# usage (inside container, /work/sdk): test/run.sh APP   -> builds TEST variant and runs it under qemu
set -e
mkdir -p build/test
APP=$1
C="mipsel-linux-gnu-gcc -Os -march=mips32 -EL -mabicalls -fpic -mno-relax-pic-calls -G0 -ffreestanding -fno-builtin -nostdlib -Irt"
mipsel-linux-gnu-gcc -march=mips32 -EL -mno-abicalls -fno-pic -DTEST -c rt/start.S -o build/test/start.o
$C -c rt/sys.S -o build/test/sys.o
$C -c rt/libc.c -o build/test/libc.o
$C -c rt/mem.c -o build/test/mem.o
$C -DTEST -c rt/gfx.c -o build/test/gfx.o
$C -DTEST -c rt/keys.c -o build/test/keys.o
$C -DTEST -c rt/sound.c -o build/test/sound.o
$C -DTEST -c apps/$APP.c -o build/test/$APP.o
$C -no-pie -static -T rt/link.ld -Wl,--emit-relocs -Wl,-e,_start -o build/test/${APP}.elf build/test/start.o build/test/sys.o build/test/libc.o build/test/mem.o build/test/gfx.o build/test/keys.o build/test/sound.o build/test/$APP.o -lgcc 2>/dev/null
python3 mkrun.py build/test/${APP}.elf build/test/${APP}.bin
mipsel-linux-gnu-gcc -O2 -static -no-pie -fno-pie -mno-abicalls -fno-pic -o test/harness test/harness.c test/disp.S 2>/dev/null
cd test && timeout ${T:-60} qemu-mipsel ./harness ../build/test/${APP}.bin
