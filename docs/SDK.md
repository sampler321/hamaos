# Writing apps

Apps are position-independent MIPS32 C (`-mabicalls -fpic`). `mkrun.py` turns the linked ELF into a `.bin`: the image, zeroed bss and a table of words to relocate. `rt/start.S` relocates the image wherever it was loaded, flushes the caches and calls `main`.

## Runtime

| File | Purpose |
|---|---|
| `rt/start.S` | entry, relocation, cache flush |
| `rt/sys.S` | OS call stubs (`sys_fopen`, `sys_malloc`, ...) and `sys_call(id, a, b, c, d)` |
| `rt/gfx.c` | YUYV drawing: `gfx_fill`, `gfx_text`, `gfx_char`, `RGB()`, Tokyo Night colours `TN_*` |
| `rt/keys.c` | input: `keys_poll(ticks)` returns `K_UP`..`K_BACK`; `KEYS.TXT` mapping |
| `rt/mem.c` | tracked `mem_alloc` so the launcher can free what an app used |
| `rt/sound.c` | PCM audio (not audible yet) |
| `rt/libc.c` | memset, memcpy, memmove, memcmp |

Rules that keep apps working on this hardware:

- Redraw only what changed. A full-screen redraw is slow.
- No large static arrays: bss is stored in the file. Use `sys_malloc`.
- Quit on `K_BACK`.
- The calendar task stack is about 8 KB.

## Build

```sh
docker build -t hama-mips toolchain/
cd sdk
docker run --rm -v "$PWD":/work -w /work hama-mips make APP=snake   # one app -> build/snake.bin
docker run --rm -v "$PWD":/work -w /work hama-mips make mono        # everything -> build/hamaos.bin
```

`rt/font.h` is rendered from a system font on macOS: `swift tools/mkfont.swift Menlo-Bold 15 10 20 > rt/font.h`.

A single app's `.bin` copied to the card as `RUN.BIN` runs when the Calendar opens.

## Emulator

`test/run.sh APP` builds a test variant and runs it under `qemu-mipsel` with `test/harness.c`, which fakes the OS calls: files map to `test/sd/`, keys come from a script inside the harness, and every screen update is saved to `test/shots/*.ppm`. Set `T=seconds` to change the timeout.

```sh
docker run --rm -v "$PWD":/work -w /work hama-mips sh -c 'T=20 test/run.sh shell'
```

## Research tools

`tools/lfi.py` lists and extracts the firmware files, `tools/apinames.py` names a driver's functions by the debug strings they use, `tools/syscalls.py` catalogues OS calls used by each module. They expect the extracted firmware in `work/extracted` and a RAM dump in `work/dumps`; `sdk/apps/diag/` has the probes that produced the dumps and key logs.
