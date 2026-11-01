# usage: apinames.py FILE.DRV module_no  -> lists table index, address, referenced strings
import os, re,struct,sys,subprocess
f,mod=sys.argv[1],int(sys.argv[2],0)
d=open(f,'rb').read(); h=struct.unpack('<9I',d[4:40]); base=h[1]; end=max(h[2],base+h[3]); body=d[0x28:]
m=open(os.environ.get('HAMA_RAMDUMP','work/dumps/MEM-80000000-2M.bin'),'rb').read()
t=struct.unpack_from('<I',m,0x338+4*(mod-1))[0]
o=t-base; fns=[]
while o+4<=len(body):
    w=struct.unpack_from('<I',body,o)[0]
    if base<=w<end: fns.append(w); o+=4
    else: break
dis=subprocess.run(['docker','run','--rm','-v',os.path.abspath(os.environ.get('HAMA_WORK','work'))+':/work','hama-mips','sh','-c',
   f'tail -c +41 /work/extracted/{os.path.basename(f)} > /tmp/x.bin; mipsel-linux-gnu-objdump -D -b binary -m mips:isa32 -EL --adjust-vma={base:#x} /tmp/x.bin'],capture_output=True,text=True).stdout
L=[];pos={}
for l in dis.splitlines():
    mm=re.match(r'\s*([0-9a-f]+):\t[0-9a-f]{8} \t(.*)',l)
    if mm: pos[int(mm.group(1),16)]=len(L); L.append(mm.group(2))
def cstr(va):
    o=va-base
    if not 0<=o<len(body): return None
    e=body.find(b'\0',o); s=body[o:e]
    return s.decode('latin1') if 4<=len(s)<70 and all(32<=c<127 or c in (9,10,13) for c in s) else None
for i,fn in enumerate(fns):
    k=pos.get(fn); strs=[]; calls=0; hi={}
    if k is not None:
        for s in L[k:k+400]:
            mm=re.match(r'lui\t(\w+),0x([0-9a-f]+)',s)
            if mm: hi[mm.group(1)]=int(mm.group(2),16)<<16
            mm=re.match(r'addiu\t(\w+),(\w+),(-?\d+)',s)
            if mm and mm.group(2) in hi:
                c=cstr((hi[mm.group(2)]+int(mm.group(3)))&0xffffffff)
                if c and c not in strs: strs.append(c.strip())
            if s.startswith('jr\tra'): break
    print(f"{mod:#x}:{i:#04x} {fn:08x} {' | '.join(strs)[:110]}")
