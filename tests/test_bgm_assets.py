"""Check block layout, declared sample capacity and aggregate Flash budget."""
import json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
folder=root/'assets/music/bgm'
tracks=json.loads((folder/'manifest.json').read_text())
assert len(tracks)<=24
size=0
for t in tracks:
    data=(folder/t['file']).read_bytes()
    assert len(data)==t['bytes'] and len(data)>0 and len(data)%256==0
    assert 0<t['samples']<=len(data)//256*505
    assert t['codec']=='IMA_ADPCM'
    assert all(data[i+2]<=88 and data[i+3]==0 for i in range(0,len(data),256))
    size+=len(data)
assert size<=5*1024*1024
print(f'BGM assets: PASS ({len(tracks)} tracks, {size} bytes)')
