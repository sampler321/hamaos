# Flashing safely

## Replace files, never add them

Every image that only replaced existing files flashed fine. The one image that added new files to the file table (and changed the used-size field) stalled on the upgrade progress bar. After a power cycle the frame came back on its previous firmware. `tools/addfiles_EXPERIMENTAL.py` is kept for reference only.

`tools/build_image.py` keeps the layout identical to the stock image.

## Recovery without tools

- **hamaOS misbehaves:** put an 8-byte `RUN.BIN` on the card containing `08 00 e0 03 00 00 00 00` (`jr ra; nop`). The loader runs it, it returns at once, and the stock calendar opens. From there all Hama menus work.
- **Booting into hamaOS by mistake:** same card, then *Settings, Start Up Mode*.
- **Bad build in the firmware:** a card `RUN.BIN` always overrides the built-in copy, and *System upgrade* stays reachable through the Hama menus.

## What can still brick it

- A crash in `CALENDAR.APP` before the loader reads the card. The hook runs first, so a broken loader is the main risk. Test loader changes in the emulator first.
- Power loss during System upgrade.
- Changing anything outside `FIRM` (boot sections) or the used-size field.

The boot code checks the files' checksums at start-up and can restore damaged system files from a backup copy. That protects against corruption, not against valid code that misbehaves.

## Lost features

hamaOS lives in the `VD_VC1.LIB` slot, so VC-1 video no longer plays. The calendar's debug output on the serial port is gone.
