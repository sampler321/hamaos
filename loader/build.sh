#!/bin/sh
# Assemble the calendar loader into its code chunks (run from the repo root; needs the hama-mips image).
set -e
docker run --rm -v "$PWD":/work -w /work/loader hama-mips sh -c '
  mipsel-linux-gnu-as -EL -mips32 -non_shared -o loader5.o loader5.S
  mipsel-linux-gnu-ld -EL -T loader.ld -o loader5.elf loader5.o
  mkdir -p out
  for s in cA cB cC cD cE cF cG str; do mipsel-linux-gnu-objcopy -O binary -j .$s loader5.elf out/$s.bin; done
  rm -f loader5.o loader5.elf'
echo "loader chunks in loader/out/"
