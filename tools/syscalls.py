# catalogue OS call stubs (lui v1,M / ori v1,v1,F around jr/j 0x80000300) per KTCA app/drv
import struct, sys, glob, os, collections
use=collections.defaultdict(set)
for p in sorted(glob.glob(sys.argv[1]+'/*')):
    d=open(p,'rb').read()
    if d[:4]!=b'KTCA': continue
    code=d[0x28:]; n=len(code)//4
    w=struct.unpack(f'<{n}I',code[:n*4])
    for i in range(n-1):
        if (w[i]>>16)==0x3c03 and (w[i+1]>>16)==0x3463 or ((w[i]>>16)==0x3c03 and i+3<n and (w[i+3]>>16)==0x3463):
            hi=w[i]&0xffff
            lo=(w[i+1]&0xffff) if (w[i+1]>>16)==0x3463 else (w[i+3]&0xffff)
            use[(hi,lo)].add(os.path.basename(p))
mods=collections.Counter(h for h,_ in use)
print("modules:",{hex(k):v for k,v in sorted(mods.items())})
for (h,l),apps in sorted(use.items()):
    if len(apps)>=6: print(f"{h:#06x}:{l:#05x} used by {len(apps):2d}")
