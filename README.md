# hamaOS

Custom software for the Hama 00095230 digital photo frame (15.6", 2009). It runs on the stock kernel and drivers, and replaces what you see and use: a terminal-style launcher, games, screensavers and a small shell, all driven by the IR remote or the frame's own buttons.

```
  _                                   ___  ____
 | |__   __ _ _ __ ___   __ _        / _ \/ ___|
 | '_ \ / _` | '_ ` _ \ / _` |_____ | | | \___ \
 | | | | (_| | | | | | | (_| |_____|| |_| |___) |
 |_| |_|\__,_|_| |_| |_|\__,_|       \___/|____/
```

## Hardware

| Part | Value |
|---|---|
| SoC | Actions Semiconductor AM7331, MIPS32 little-endian |
| OS | uC/OS-II with Actions' module system (`.APP` / `.DRV` / `.LIB`) |
| Panel | 1366x768 LVDS; the video layer is 1024x576 YUYV 4:2:2, double buffered |
| Storage | NAND (firmware, read-only to apps) and one SD slot (`D:`) |
| Input | IR remote and 6 front buttons |

## Apps

| App | What it does |
|---|---|
| snake | the game, with your remote or the frame buttons |
| matrix | digital rain |
| aquarium | ASCII fish tank |
| shapes | mystify-style screensaver |
| dvd | bouncing badge, counts corner hits |
| shell | type with the arrows: `help`, `calc 6*7`, `echo`, `uptime`, `run snake` |
| keysetup | map remote and frame buttons, saved to `KEYS.TXT` on the card |
| script | runs `SCRIPT.TXT` from the card (text, wait, loop) |

## How it runs

The firmware's Calendar app gets a small loader. When the Calendar opens:

1. If `D:\RUN.BIN` exists on the SD card, the loader runs it. Development builds go here and override everything.
2. Otherwise it runs the copy of hamaOS built into the firmware.
3. When hamaOS exits, the stock calendar continues.

Set *Settings, Start Up Mode* to *Calendar* and the frame boots into hamaOS.

## Install

You need the stock firmware update file for the 00095230 (version 20121130-1, `AMAMUPUP.bin`). It is not included here.

```sh
docker build -t hama-mips toolchain/
swift sdk/tools/mkfont.swift Menlo-Bold 15 10 20 > sdk/rt/font.h      # macOS
(cd sdk && docker run --rm -v "$PWD":/work -w /work hama-mips make mono)
./loader/build.sh
python3 tools/build_image.py AMAMUPUP-stock.bin sdk/build/hamaos.bin AMAMUPUP.bin
```

Copy `AMAMUPUP.bin` to the root of a FAT32 SD card, insert it, then *Settings, System settings, System upgrade, Yes*. Read [docs/SAFETY.md](docs/SAFETY.md) first.

## Docs

- [docs/FIRMWARE.md](docs/FIRMWARE.md): update file format, checksums, how the loader is patched in
- [docs/API.md](docs/API.md): OS calls, display, input, files, audio
- [docs/SDK.md](docs/SDK.md): writing apps, the emulator, the build
- [docs/SAFETY.md](docs/SAFETY.md): what is safe to flash, recovery
- [CHANGELOG.md](CHANGELOG.md)

## Status

Works on the author's frame: launcher, games, screensavers, shell, key mapping, booting into hamaOS without a card. Audio output is reverse engineered but did not play on the device yet (`sdk/apps/radio.c`).

## Legal

This repository contains only original code and documentation. It does not include Hama or Actions firmware, extracted modules or memory dumps. Hama is a trademark of Hama GmbH & Co KG; this project is not affiliated with them. Flashing modified firmware can damage your device and is at your own risk.

License: MIT.
