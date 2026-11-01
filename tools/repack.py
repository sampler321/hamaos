"""Rebuild a Hama 00095230 / Actions AM7 AMAMUPUP.bin, replacing FIRM files from a folder.
usage: repack.py original.bin replacements_dir out.bin
Only files present in replacements_dir are swapped; each must fit its existing allocation."""
import struct, sys, os
def s32(b):
    b=b+b'\0'*(-len(b)%4); n=len(b)//4
    return sum(struct.unpack(f'<{n}I',b))&0xffffffff
src,repl,out=sys.argv[1:4]
d=bytearray(open(src,'rb').read())
F=0x3a800
off=F+0x200; changed=[]
while d[off]:
    name=(d[off:off+8].decode('latin1').rstrip()+'.'+d[off+8:off+11].decode('latin1').rstrip())
    sec,alloc,size,ck=struct.unpack_from('<4I',d,off+16)
    p=os.path.join(repl,name)
    if os.path.exists(p):
        blob=open(p,'rb').read()
        if len(blob)>alloc: sys.exit(f"{name}: {len(blob)} bytes > allocation {alloc}")
        a=F+sec*512
        d[a:a+alloc]=blob+b'\0'*(alloc-len(blob))
        struct.pack_into('<II',d,off+24,len(blob),s32(blob))
        changed.append(name)
    off+=32
struct.pack_into('<I',d,F+0x1c,s32(d[F+0x200:F+0x2000]))   # file-table checksum
struct.pack_into('<I',d,0x3c,s32(d[0x40:]))                  # outer image checksum
open(out,'wb').write(d)
print("replaced:",changed or "nothing")
