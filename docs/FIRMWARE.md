# Firmware update format

Applies to `AMAMUPUP.bin` for the Hama 00095230, version 20121130-1 (19,993,088 bytes). The frame's own firmware may be newer (the author's shipped with 20140903-1); flashing the 2012 base worked fine.

## Outer container

| Offset | Content |
|---|---|
| 0x00 | `ActionsFirmware`, then `EZToolVer:v1.06` |
| 0x20 | `CHECKSUM` |
| 0x3c | u32 checksum: sum of all 32-bit LE words from 0x40 to the end |
| 0x40 | section table, 32 bytes per entry: name[16], offset, length, load address, extra |

Sections: `ADECadfus`, `ADFUadfus` (USB recovery), `HWSChwsc`, `F648fwsc`, `F648mbrec`, `F648brec` (boot code), `FIRM`.

The `FIRM` entry's third word (offset 0x118) holds the used size of the file area. Changing it made the update hang.

## FIRM: the file area (LFI)

`FIRM` starts at 0x3a800. Header magic `55 aa f2 0f`, then strings `actions` and `media player`, and `Fw:2012-11-30` at +0xf0.

| Offset in FIRM | Content |
|---|---|
| +0x1c | u32 checksum of the file table (sum of words in +0x200..+0x2000) |
| +0x200 | file table, 32 bytes per file, ends at the first zero entry |

File table entry:

| Bytes | Field |
|---|---|
| 0..7 | name, space padded |
| 8..10 | extension |
| 11..15 | zero |
| 16 | u32 start sector (512 bytes, relative to FIRM) |
| 20 | u32 allocated bytes |
| 24 | u32 file size |
| 28 | u32 checksum: sum of the file's 32-bit LE words |

The stock image has 91 files: kernel init, drivers (`*.DRV`), apps (`*.APP`), codecs (`*.LIB`), the UI resources (`EASY.RES`, 16 MB) and config.

All three checksums are plain 32-bit word sums. No signatures. `tools/repack.py` swaps files and recomputes everything; rebuilding without changes is byte-identical.

## Module files (KTCA)

Every `.APP`, `.DRV` and `.LIB` starts with a 40-byte header:

| Offset | Field |
|---|---|
| 0 | `KTCA` |
| 4 | entry point |
| 8 | load address |
| 12 | end address (including bss) |
| 16 | code size (file size minus 40) |
| 24 | bss start |
| 28 | bss size |
| 32 | drivers: offset of the exported function table |

An app's entry returns a descriptor: `"app\0"`, pointer to main, 0, stack size and priority.

## Version check

The upgrade path only checks that `D:\AMAMUPUP.BIN` exists. The version shown in Settings comes from `EASY.RES` (16 copies of `20121130-1`). `tools/build_image.py` replaces them with a 10-character string of your choice.

## The calendar hook

`CALENDAR.APP` keeps about 400 bytes of debug strings that are only ever passed to `printf`. The build replaces the `printf` stub with a return, puts the loader into those string areas (7 chunks) and points the app descriptor's main at the loader. See `loader/loader5.S`.

The loader:

1. opens `D:\RUN.BIN` (FS call 3:04); if that fails, opens the firmware file `VD_VC1.LIB` (kernel call 1:2c)
2. allocates 64 KB (1:28) and reads the file into it
3. writes back and invalidates the caches by index over kseg0
4. calls it with `a0` = OS dispatcher, `a1` = load address
5. frees the buffer and jumps to the original calendar main
