import os, re,struct,sys,subprocess,collections
def load(app):
    d=open(os.path.join(os.environ.get('HAMA_WORK','work'),'extracted',app),'rb').read()
    base=struct.unpack('<I',d[8:12])[0]
    body=d[0x28:]; n=len(body)//4; w=struct.unpack(f'<{n}I',body[:n*4])
    stubs={}
    for i in range(n-4):
        if w[i]==0x3c028000 and w[i+1]==0x34420300 and (w[i+2]>>16)==0x3c03 and (w[i+4]>>16)==0x3463:
            stubs[base+i*4]=((w[i+2]&0xffff),(w[i+4]&0xffff))
    return base,w,stubs
