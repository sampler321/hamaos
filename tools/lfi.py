import struct, sys, os
def s32(b):
    b=b+b'\0'*(-len(b)%4); n=len(b)//4
    return sum(struct.unpack(f'<{n}I',b))&0xffffffff
d=open(sys.argv[1],'rb').read()
F=0x3a800; L=d[F:F+0x012e0000]
ents=[]
off=0x200
while True:
    e=L[off:off+32]
    if e[0]==0: break
    name=e[:8].decode('latin1').rstrip()+'.'+e[8:11].decode('latin1').rstrip()
    sec,alloc,size,ck=struct.unpack('<4I',e[16:32])
    ents.append((name,sec,alloc,size,ck)); off+=32
out=sys.argv[2]; os.makedirs(out,exist_ok=True)
ok=0
for name,sec,alloc,size,ck in ents:
    blob=L[sec*512:sec*512+size]
    open(os.path.join(out,name),'wb').write(blob)
    m = s32(blob)==ck
    ok+=m
    print(f"{name:14s} sec={sec:6d} size={size:9d} ck={ck:08x} sum32={'OK' if m else 'no'}")
print(len(ents),"entries;",ok,"match sum32; dir ends at",hex(off))
