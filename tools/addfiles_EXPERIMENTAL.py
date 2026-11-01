"""Add new files to the FIRM/LFI file table of an AMAMUPUP.bin.
usage: addfiles.py in.bin out.bin NAME.EXT=path [NAME.EXT=path ...]   (8.3 names, replaced if they exist)"""
import struct, sys
def s32(b):
    b = b + b'\0' * (-len(b) % 4); n = len(b) // 4
    return sum(struct.unpack(f'<{n}I', b)) & 0xffffffff
src, out, *specs = sys.argv[1:]
d = bytearray(open(src, 'rb').read())
F = 0x3a800
firm_len, used = struct.unpack_from('<II', d, 0x114)
assert struct.unpack_from('<I', d, 0x110)[0] == F
# existing entries
off = F + 0x200; ents = {}
while d[off]:
    name = d[off:off+8].decode('latin1').rstrip() + '.' + d[off+8:off+11].decode('latin1').rstrip()
    ents[name] = off; off += 32
end_dir = off
next_sec = used // 512
for spec in specs:
    name, path = spec.split('=', 1); name = name.upper()
    blob = open(path, 'rb').read()
    base, ext = name.split('.')
    assert len(base) <= 8 and len(ext) <= 3
    alloc = (len(blob) + 511) // 512 * 512
    if name in ents:
        e = ents[name]; sec, al, sz, ck = struct.unpack_from('<4I', d, e + 16)
        if len(blob) > al: sys.exit(f"{name}: grows beyond its allocation")
    else:
        e = end_dir; end_dir += 32; assert end_dir <= F + 0x2000, "file table full"
        sec, al = next_sec, alloc; next_sec += alloc // 512
        if next_sec * 512 > firm_len: sys.exit(f"out of LFI space adding {name}: need {next_sec*512:#x} > {firm_len:#x}")
        d[e:e+16] = base.ljust(8).encode() + ext.ljust(3).encode() + b'\0' * 5
        ents[name] = e
    a = F + sec * 512
    d[a:a+al] = blob + b'\0' * (al - len(blob))
    struct.pack_into('<4I', d, e + 16, sec, al, len(blob), s32(blob))
    print(f"{name}: sector {sec}, {len(blob)} bytes")
struct.pack_into('<I', d, 0x118, max(used, next_sec * 512))
struct.pack_into('<I', d, F + 0x1c, s32(d[F + 0x200:F + 0x2000]))
struct.pack_into('<I', d, 0x3c, s32(d[0x40:]))
open(out, 'wb').write(d)
print(f"LFI used {max(used, next_sec*512):#x} of {firm_len:#x} ({firm_len - max(used, next_sec*512)} bytes free)")
