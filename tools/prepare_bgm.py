#!/usr/bin/env python3
"""Build a bounded IMA-ADPCM pack from the active library or supplied files."""
import argparse
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import wave

ROOT = Path(__file__).resolve().parents[1]
RATE = 16000
BUDGET = 5 * 1024 * 1024
BLOCK = 256
SAMPLES_PER_BLOCK = 505

def wav_data(blob):
    if blob[:4]!=b'RIFF' or blob[8:12]!=b'WAVE': raise ValueError('Not a WAV container')
    pos=12; chunks={}
    while pos+8<=len(blob):
        kind=blob[pos:pos+4];size=struct.unpack_from('<I',blob,pos+4)[0]
        chunks[kind]=blob[pos+8:pos+8+size];pos+=8+size+(size%2)
    fmt=chunks[b'fmt ']
    assert struct.unpack_from('<HHI',fmt)==(17,1,RATE)
    assert struct.unpack_from('<H',fmt,12)[0]==BLOCK
    return chunks[b'data']

def encode(path):
    with tempfile.TemporaryDirectory() as tmp:
        pcm=Path(tmp)/'pcm.wav';out=Path(tmp)/'ima.wav'
        subprocess.run(['ffmpeg','-v','error','-nostdin','-i',str(path),'-t','650','-vn','-ac','1','-ar',str(RATE),
                        '-af','alimiter=limit=0.85:level=false','-c:a','pcm_s16le',str(pcm)],check=True)
        with wave.open(str(pcm)) as f: samples=f.getnframes()
        subprocess.run(['ffmpeg','-v','error','-nostdin','-i',str(pcm),'-c:a','adpcm_ima_wav','-block_size',str(BLOCK),str(out)],check=True)
        data=wav_data(out.read_bytes())
    if not samples or not data or len(data)%BLOCK:raise ValueError('Empty or invalid audio')
    return data,samples

def pack(tracks):
    converted=[]
    for track in tracks:
        data,samples=encode(Path(track['path']))
        converted.append((track,data,samples))
    total=sum(len(data) for _,data,_ in converted)
    if total>BUDGET:raise ValueError(f'Audio pack exceeds {BUDGET} bytes; shorten or remove tracks')
    dest=ROOT/'assets/music/bgm';dest.mkdir(parents=True,exist_ok=True)
    entries=[];declarations=[];metadata=[];cmake=[]
    for i,(track,data,samples) in enumerate(converted):
        filename=f'track_{i:02}.adpcm';(dest/filename).write_bytes(data)
        declarations.append(f'extern const uint8_t track_{i}[] asm("_binary_track_{i:02}_adpcm_start");')
        title=track['title']
        entries.append(f'    {{{json.dumps(title,ensure_ascii=False)}, track_{i}, {samples}u, {len(data)}u}},')
        cmake.append(f'"${{CMAKE_CURRENT_LIST_DIR}}/../assets/music/bgm/{filename}"')
        metadata.append(dict(title=title,source=track.get('source',Path(track['path']).name),file=filename,
                             samples=samples,seconds=round(samples/RATE,3),bytes=len(data),codec='IMA_ADPCM',block_size=BLOCK))
    # An empty library is valid; count zero prevents the placeholder being played.
    if not entries:entries=['    {"", NULL, 0, 0},']
    (ROOT/'main/bgm_tracks.c').write_text('#include "bgm_tracks.h"\n#include <stddef.h>\n'+'\n'.join(declarations)+
        '\nconst bgm_track_t bgm_tracks[] = {\n'+'\n'.join(entries)+f'\n}};\nconst unsigned bgm_track_count = {len(converted)};\n')
    (ROOT/'main/bgm_assets.cmake').write_text('set(BGM_ASSETS\n'+'\n'.join(cmake)+'\n)\n')
    (dest/'manifest.json').write_text(json.dumps(metadata,ensure_ascii=False,indent=2)+'\n')
    print(f'{len(converted)} tracks; {total:,} bytes; IMA-ADPCM 16 kHz mono')

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('audio',nargs='*',type=Path);ap.add_argument('--library',type=Path);args=ap.parse_args()
    if args.library:
        tracks=json.loads(args.library.read_text())
    elif args.audio:
        tracks=[dict(path=str(p.resolve()),title=f'我的音频 {i+1:02}') for i,p in enumerate(args.audio)]
    else:
        catalog=json.loads((ROOT/'assets/music/builtin/catalog.json').read_text())
        tracks=[dict(path=str(ROOT/f['path']),title=f['title'],source=f['source']) for f in catalog]
    if len(tracks)>24:ap.error('Maximum 24 tracks')
    pack(tracks)
if __name__=='__main__':main()
