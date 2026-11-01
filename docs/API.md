# OS interface

Everything here was found by disassembly and tested on the device unless marked *untested*.

## Calling convention

Every OS call jumps to `0x80000300` with `v1 = (module << 16) | function` and arguments in `a0..a3` (more on the stack, o32 style). The dispatcher indexes a module table at `0x80000338` (entry `module - 1`), then the module's function table. Apps and drivers run in kernel mode.

| Module | Owner |
|---|---|
| 1, 7 | kernel (uC/OS-II plus Actions services) |
| 2 | GUI.DRV (emWin based) |
| 3 | FS.DRV |
| 4 | A_CODEC.DRV (decoders) |
| 5 | KEY.DRV |
| 6 | LCM.DRV (display) |
| 8 | SDCARD.DRV |
| 0xc | VRAM.DRV |
| 0xd | COM_UI.DRV |
| 0xe | MNAVI.DRV |
| 0x10 | ADA.DRV (DAC) |
| 0x12 | MLANG.DRV |
| 0x13 | CHARGE.DRV |
| 0x15 | CARDDET.DRV |
| 0x20 | IIC.DRV |

## Kernel

| Call | Meaning |
|---|---|
| 1:0f(ticks) | sleep, 10 ms per tick |
| 1:11() | tick counter (OSTimeGet) |
| 1:14(buf, n) | create message queue (OSQCreate) |
| 1:17(q, &msg, ticks) | wait for a message; returns 10 on timeout |
| 1:1b(n) | create semaphore |
| 1:28(size) | malloc |
| 1:29(p) | free |
| 1:2c(name, 0) | open a file inside the firmware (read only) |
| 1:2d(h) | close it |
| 1:2e(h, 0, off) | seek |
| 1:30(h, buf, len) | read |
| 1:32(fmt, ...) | printf to the serial port |
| 1:3a() | current task block; word 3 is the task's message queue |

## Files (FS.DRV)

| Call | Meaning |
|---|---|
| 3:04(path, mode) | open; `"D:\\FILE.TXT"`; mode 1 read, 10 or 11 write |
| 3:06(fp) | close |
| 3:09(buf, len, fp) | read, returns bytes |
| 3:0a(buf, len, fp) | write |
| 3:2e(0x10020000, buf) | list drives: 4 entries of 20 bytes, letter at +0, type at +4 (16 = card) |

Only `D:` (the card) exists for apps. Writing to `C:` hangs. Opening an existing file for writing overwrites without truncating.

## Display

The visible picture is the video layer: two 1024x576 YUYV 4:2:2 buffers at `0xa0bfdfa0` and `0xa0dfefe0` (uncached). Bytes per pixel pair: Y0 U Y1 V. Drawing into both buffers shows immediately; no flip call is needed.

Hama's menus also use an OSD overlay on top. `6:19(layer)` clears an overlay layer (0 and 1). `2:04(text, x, y)` (GUI_DispStringAt) draws on the overlay; `6:10(x, y, w, h)` pushes the overlay to the panel.

## Input

Create your own queue with `1:14`, store it in task word 3 (`1:3a()[3]`), then wait with `1:17`. Key messages have type 22 (`msg[0] & 0xff`) and the code in `msg[1] & 0xffff`: `id = code & 0x3f`, `flags = code & 0xc0`.

| Source | Events per press |
|---|---|
| IR remote | `0x40\|id`, `id`, `0xc0\|id`, then an auto-repeat burst `0x80\|id`, `id`, ..., `0xc0\|id` on release |
| frame buttons | `0x40\|id` only |

A press is exactly one event with flags `0x40`. Codes on the author's frame:

| Action | Remote | Frame |
|---|---|---|
| Up | 1b | 05 |
| Down | 1d | 06 |
| Left | 00 | 03 |
| Right | 20 | 04 |
| OK | 1f | 08 |
| Back / Menu | 19 | 02 |

## Audio (untested on the device)

PCM through ADA.DRV, taken from the firmware's own WAV player:

| Call | Meaning |
|---|---|
| 10:09(buf, len, 0) | set the DMA ring buffer |
| 10:01(0x80016406, &{0,0,0,1}) | select the DAC source |
| 10:01(0x80016401, &{rate, 4, mono, 16bit}) | format; rate index 11 = 22050 Hz |
| 10:01(0x80016402, vol) | volume 0..31 |
| 10:06(pcm, len) | queue samples; returns len, or 0 if full |
| 10:08() | bytes still queued |
| 10:01(0x40046415, 0) | stop |

The radio app streams a synthesised loop this way but nothing was audible. File playback through A_CODEC (`4:03`, `4:04(h, 22, path)`, `4:04(h, 1, 0)`) is untested.
