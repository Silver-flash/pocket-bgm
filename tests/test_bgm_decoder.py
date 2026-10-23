"""Compare the exact C decoder to FFmpeg across every embedded block and chunk boundary."""
import ctypes,json,subprocess,tempfile,struct,sys
from pathlib import Path
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
    lib=Path(tmp)/'decoder.so'
    subprocess.run(['cc','-shared','-fPIC','-Wall','-Wextra','-Werror','-Imain','main/bgm_decoder.c','-o',str(lib)],cwd=root,check=True)
    dec=ctypes.CDLL(str(lib)).bgm_decode
    dec.argtypes=[ctypes.c_void_p,ctypes.c_size_t,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_void_p,ctypes.c_size_t];dec.restype=ctypes.c_bool
    for track in json.loads((root/'assets/music/bgm/manifest.json').read_text()):
        raw=(root/'assets/music/bgm'/track['file']).read_bytes();data=ctypes.create_string_buffer(raw)
        fmt=struct.pack('<HHIIHHHH',17,1,16000,8110,256,4,2,505)
        chunks=b'fmt '+struct.pack('<I',len(fmt))+fmt+b'data'+struct.pack('<I',len(raw))+raw
        wav=Path(tmp)/'ima.wav';wav.write_bytes(b'RIFF'+struct.pack('<I',len(chunks)+4)+b'WAVE'+chunks)
        reference=subprocess.check_output(['ffmpeg','-v','error','-i',str(wav),'-f','s16le','-'])[:track['samples']*2]
        actual=bytearray()
        for cursor in range(0,track['samples'],320):
            count=min(320,track['samples']-cursor);out=(ctypes.c_int16*count)()
            assert dec(data,len(raw),track['samples'],cursor,out,count)
            actual.extend(bytes(out))
        assert actual==reference,track['file']
        out=(ctypes.c_int16*1)()
        assert not dec(data,len(raw),track['samples'],track['samples'],out,1)
        assert not dec(data,0,track['samples'],0,out,1)
    bad=ctypes.create_string_buffer(b'\0\0\xff\0'+b'\0'*252)
    assert not dec(bad,256,505,0,(ctypes.c_int16*1)(),1)
print('BGM decoder: PASS (all samples match FFmpeg, boundaries and malformed header)')
