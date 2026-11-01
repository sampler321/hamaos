#!/usr/bin/env python3
"""Build a flashable hamaOS image from the stock Hama 00095230 firmware update (20121130-1).

usage: build_image.py STOCK_AMAMUPUP.bin HAMAOS.bin OUT_AMAMUPUP.bin [VERSION]

* patches CALENDAR.APP with the loader (loader/out/*.bin, from loader/build.sh)
* stores hamaOS in the VD_VC1.LIB slot (VC-1 video decoder; VC-1 playback is lost)
* replaces the version string "20121130-1" (10 chars) in EASY.RES
The file layout stays identical to the stock image: only existing files are replaced.
Adding new files to the image made System upgrade hang (see docs/SAFETY.md)."""
import struct, sys, os, subprocess, tempfile, shutil
here = os.path.dirname(os.path.abspath(__file__)); root = os.path.dirname(here)
stock, hamaos, out = sys.argv[1:4]
version = (sys.argv[4] if len(sys.argv) > 4 else '20261102-1').encode()
assert len(version) == 10, "version must be 10 characters, e.g. 20261102-1"
tmp = tempfile.mkdtemp()
subprocess.run([sys.executable, '-I', os.path.join(here, 'lfi.py'), stock, tmp + '/x'], check=True, stdout=subprocess.DEVNULL)
os.makedirs(tmp + '/r')
# --- calendar hook --------------------------------------------------------------
c = bytearray(open(tmp + '/x/CALENDAR.APP', 'rb').read()); base = 0x01600000
chunks = {'cA': (0x01609400, 92), 'cB': (0x01609cd4, 72), 'cC': (0x01609d60, 80), 'cD': (0x01609728, 60),
          'cE': (0x01609474, 60), 'cF': (0x01609c1c, 56), 'cG': (0x01609530, 52), 'str': (0x016093ac, 32)}
for name, (va, lim) in chunks.items():
    blob = open(os.path.join(root, 'loader', 'out', name + '.bin'), 'rb').read()
    assert len(blob) <= lim, (name, len(blob))
    o = va - base + 0x28; c[o:o + len(blob)] = blob
o = 0x0160a9ec - base + 0x28                          # app descriptor: main -> hook
assert struct.unpack_from('<I', c, o + 4)[0] == 0x01607804, "unexpected CALENDAR.APP (wrong stock firmware?)"
struct.pack_into('<I', c, o + 4, 0x01609400)
o = 0x01608f4c - base + 0x28                          # printf stub -> return (its strings now hold code)
assert struct.unpack_from('<I', c, o)[0] == 0x3c028000
struct.pack_into('<II', c, o, 0x03e00008, 0x00001021)
open(tmp + '/r/CALENDAR.APP', 'wb').write(c)
open(tmp + '/r/EASY.RES', 'wb').write(open(tmp + '/x/EASY.RES', 'rb').read().replace(b'20121130-1', version))
shutil.copy(hamaos, tmp + '/r/VD_VC1.LIB')
subprocess.run([sys.executable, '-I', os.path.join(here, 'repack.py'), stock, tmp + '/r', out], check=True)
shutil.rmtree(tmp)
print(f"wrote {out}  (version {version.decode()}). Copy it to the SD card root as AMAMUPUP.bin.")
