#!/usr/bin/env python3
# ELF (PIC/abicalls, statically linked at 0 with --emit-relocs) -> RUN.BIN:
# image + .bss zeros + relocation table (offsets of words that need +base: GOT entries and R_MIPS_32/REL32).
import subprocess, struct, sys, re
elf, out = sys.argv[1], sys.argv[2]
run = lambda *a: subprocess.run(['mipsel-linux-gnu-'+a[0], *a[1:]], capture_output=True, text=True).stdout
secs = {}
for l in run('readelf', '-SW', elf).splitlines():
    m = re.match(r'\s*\[\s*\d+\]\s+(\S+)\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)', l)
    if m: secs[m.group(1)] = (int(m.group(3),16), int(m.group(5),16))
keep = [n for n in secs if n in ('.text', '.rodata', '.data', '.got', '.bss') or n.startswith('.text')]
img_end = max(secs[n][0] + secs[n][1] for n in keep)
subprocess.run(['mipsel-linux-gnu-objcopy', '-O', 'binary', '-j', '.text', '-j', '.rodata', '-j', '.data', '-j', '.got',
                elf, '/tmp/img.bin'], check=True)
img = bytearray(open('/tmp/img.bin', 'rb').read())
img += b'\0' * (img_end - len(img)); img += b'\0' * (-len(img) % 4)
fix = set()
# GOT: every entry except the 2 reserved ones holds an absolute link-time address (page or symbol)
if '.got' in secs:
    ga, gs = secs['.got']
    for off in range(ga + 8, ga + gs, 4): fix.add(off)
skip_syms = ('_gp_disp',)
bad = set(); cur = None
for l in run('readelf', '-rW', elf).splitlines():
    m = re.match(r"Relocation section '\.rel(\S+)'", l)
    if m: cur = m.group(1); continue
    m = re.match(r'([0-9a-f]{8})\s+[0-9a-f]+\s+(R_MIPS_\w+)\s*[0-9a-f]*\s*(.*)$', l)
    if not m or cur is None or cur == '.got': continue
    off, typ, sym = int(m.group(1), 16), m.group(2), m.group(3).strip()
    if typ in ('R_MIPS_32', 'R_MIPS_REL32'): fix.add(off)
    elif typ in ('R_MIPS_GOT16', 'R_MIPS_CALL16', 'R_MIPS_GPREL16', 'R_MIPS_GPREL32', 'R_MIPS_PC16',
                 'R_MIPS_JALR', 'R_MIPS_NONE', 'R_MIPS_GOT_PAGE', 'R_MIPS_GOT_OFST', 'R_MIPS_GOT_DISP'): pass
    elif typ in ('R_MIPS_HI16', 'R_MIPS_LO16') and any(s in sym for s in skip_syms): pass
    elif typ == 'R_MIPS_LO16' and cur.startswith('.text'):
        # LO16 paired with a GOT16 page load (local symbol) is relative to the GOT page: no fix needed
        pass
    else: bad.add(f"{typ} {sym} in {cur}")
if bad: sys.exit("unsupported relocations:\n  " + "\n  ".join(sorted(bad)))
tab_off = len(img)
img += b''.join(struct.pack('<I', (1 << 28) | o) for o in sorted(fix)) + b'\0\0\0\0'
struct.pack_into('<II', img, 8, tab_off, len(img))
if len(img) > 0x10000: sys.exit(f"RUN.BIN too big: {len(img)} > 65536")
open(out, 'wb').write(img)
print(f"{out}: {len(img)} bytes, {len(fix)} words relocated")
