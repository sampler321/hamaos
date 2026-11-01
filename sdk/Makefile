CC=mipsel-linux-gnu-gcc
CFLAGS=-Os -march=mips32 -EL -mabicalls -fpic -mno-relax-pic-calls -G0 -ffreestanding -fno-builtin -nostdlib -Wall -Irt
APP?=script
all: build/$(APP).bin
build/start.o: rt/start.S
	@mkdir -p build; $(CC) -march=mips32 -EL -mno-abicalls -fno-pic $(DEFS) -c $< -o $@
build/sys.o: rt/sys.S
	@mkdir -p build; $(CC) $(CFLAGS) -c $< -o $@
build/$(APP).o: $(wildcard apps/$(APP).c apps/diag/$(APP).c) rt/hama.h
	@mkdir -p build; $(CC) $(CFLAGS) -c $< -o $@
build/libc.o: rt/libc.c
	@mkdir -p build; $(CC) $(CFLAGS) -c $< -o $@
rt/font.h:
	@echo "rt/font.h is generated: on macOS run  swift tools/mkfont.swift Menlo-Bold 15 10 20 > rt/font.h"; exit 1
build/gfx.o: rt/gfx.c rt/gfx.h rt/font.h
	@mkdir -p build; $(CC) $(CFLAGS) $(FBDEFS) -c $< -o $@
build/keys.o: rt/keys.c rt/keys.h
	@mkdir -p build; $(CC) $(CFLAGS) -c $< -o $@
build/mem.o: rt/mem.c
	@mkdir -p build; $(CC) $(CFLAGS) -c $< -o $@
build/sound.o: rt/sound.c rt/sound.h
	@mkdir -p build; $(CC) $(CFLAGS) -c $< -o $@
build/$(APP).elf: build/start.o build/sys.o build/libc.o build/gfx.o build/keys.o build/sound.o build/mem.o build/$(APP).o
	$(CC) $(CFLAGS) -no-pie -static -T rt/link.ld -Wl,--emit-relocs -Wl,-e,_start -o $@ $^ -lgcc
build/$(APP).bin: build/$(APP).elf
	python3 mkrun.py $< $@
clean:
	rm -rf build

MONO_APPS=snake matrix aquarium shapes dvd shell
mono: build/start.o build/sys.o build/libc.o build/gfx.o build/keys.o build/sound.o build/mem.o
	@for a in $(MONO_APPS); do $(CC) $(CFLAGS) -DMONO -Dmain=app_$$a -Dsys_malloc=mem_alloc -c apps/$$a.c -o build/m_$$a.o || exit 1; done
	$(CC) $(CFLAGS) -DMONO -c apps/launcher.c -o build/m_launcher.o
	$(CC) $(CFLAGS) -no-pie -static -T rt/link.ld -Wl,--emit-relocs -Wl,-e,_start -o build/hamaos.elf build/start.o build/sys.o build/libc.o build/gfx.o build/keys.o build/sound.o build/mem.o build/m_launcher.o $(addprefix build/m_,$(addsuffix .o,$(MONO_APPS))) -lgcc
	python3 mkrun.py build/hamaos.elf build/hamaos.bin
