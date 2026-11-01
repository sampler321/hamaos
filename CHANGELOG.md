# Changelog

Firmware versions are the strings shown in *Settings, Version*. hamaOS versions are shown on the launcher banner.

## hamaOS 0.8 (firmware 20261102-1)
- New shell app: type with the arrows, run `help`, `clear`, `echo`, `calc`, `uptime`, `about`, `run <app>`.
- Radio app removed from the menu (audio does not play on the device yet).
- Memory used by an app is released when it exits, so apps can be opened repeatedly.

## hamaOS 0.7.1 (firmware 20261101-9)
- Built-in apps start directly; the launcher no longer probes the firmware for app files.

## hamaOS 0.7
- Input rewritten from a measured key log: a press is exactly one event with flag `0x40`. Remote auto-repeat bursts no longer cause double presses.
- Remote and frame buttons both mapped by default.
- Keysetup has two rounds (remote, then frame buttons), shows each code, rejects buttons already in use, and never leaves OK unmapped.

## hamaOS 0.6
- Back no longer exits the launcher; only the *exit* item does.
- Input is ignored for half a second after start, so the press that opened the Calendar is not seen as a command.

## hamaOS 0.5
- Audio library (PCM through the DAC driver) and the radio app. Not audible on the device.

## hamaOS 0.4 (firmware 20261101-7)
- All apps linked into one binary with one copy of the font.
- hamaOS stored inside the firmware in the `VD_VC1.LIB` slot; runs without a card.
- Clears Hama's overlay layer on start.
- Matrix, aquarium, shapes and dvd added.

## Firmware 20261101-5 (withdrawn)
- Added new files to the firmware's file table. System upgrade hung on the progress bar; a power cycle returned to the previous firmware. Never add files; replace existing ones.

## hamaOS 0.1 to 0.3
- Launcher with ASCII banner and Tokyo Night colours.
- Direct YUYV framebuffer drawing, own 10x20 bitmap font.
- Snake.

## Firmware 20261101-3
- Calendar loader v2: loads `D:\RUN.BIN` into heap memory and runs it. First code executed from the SD card.
- Loader v1 crashed because it loaded into memory the calendar task was already using.

## Firmware 20261101-1
- First repacked image (stock 2012 firmware with the version string raised). Proved the update file format and checksums.
