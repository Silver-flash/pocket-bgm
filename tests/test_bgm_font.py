"""Check actual generated cmap coverage, including a known missing character."""
import re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
font=(root/'assets/fonts/bgm/bgm_font.c').read_text()
cmaps=re.search(r'cmaps\[\].*?=\s*\{(.*?)\n\};',font,re.S).group(1)
available=set()
for block in re.findall(r'\{([^{}]+)\}',cmaps):
    start=int(re.search(r'\.range_start = (\d+)',block).group(1))
    length=int(re.search(r'\.range_length = (\d+)',block).group(1))
    if 'FORMAT0_TINY' in block:available.update(range(start,start+length))
    else:
        name=re.search(r'\.unicode_list = (\w+)',block).group(1)
        data=re.search(r'\b'+name+r'\[\]\s*=\s*\{(.*?)\};',font,re.S).group(1)
        offsets=[int(x,0) for x in re.findall(r'0x[0-9a-fA-F]+|\d+',data)]
        available.update(start+x for x in offsets)
text=''.join((root/p).read_text() for p in ['main/main.c','main/bgm_tracks.c'])+'我的音频'
needed={ord(c) for c in text if '\u4e00'<=c<='\u9fff'}|set(range(32,127))
assert not needed-available, ''.join(chr(x) for x in sorted(needed-available))
assert ord('龘') not in available
assert 'lv_obj_set_style_text_font(obj, &bgm_font' in text
print(f'BGM font coverage: PASS ({len(needed)} code points)')
