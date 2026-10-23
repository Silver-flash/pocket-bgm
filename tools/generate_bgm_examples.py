#!/usr/bin/env python3
"""Generate original synthetic demo tones; no sampled or third-party recordings (MIT)."""
import json
import math
from pathlib import Path
import struct
import wave
ROOT = Path(__file__).resolve().parents[1]
RATE = 16000

def main():
    folder = ROOT / 'assets/music/builtin'
    folder.mkdir(parents=True, exist_ok=True)
    catalog = []
    for index, (name, notes) in enumerate([
        ('NEON', [220, 330, 440, 330, 261.63, 392, 523.25, 392]),
        ('DRIFT', [196, 246.94, 293.66, 369.99, 293.66, 246.94, 220, 196]),
        ('VICTORY', [261.63, 329.63, 392, 523.25, 392, 329.63, 392, 523.25]),
    ], 1):
        path = folder / f'example_{index:02}.wav'
        frames = bytearray()
        for sample in range(RATE * 6):
            t = sample / RATE
            beat = t % .375
            frequency = notes[int(t / .375) % len(notes)]
            envelope = min(1., beat / .012) * max(0., 1. - beat / .375) ** 2
            edge = min(1., t / .03, (6 - t) / .03)
            value = .22 * envelope * edge * (math.sin(2*math.pi*frequency*t) + .2*math.sin(4*math.pi*frequency*t))
            frames.extend(struct.pack('<h', round(value*32767)))
        with wave.open(str(path), 'wb') as output:
            output.setparams((1, 2, RATE, 0, 'NONE', 'not compressed'))
            output.writeframes(frames)
        catalog.append(dict(id=f'example-{index:02}', name=name, title=name,
                            source='Original procedural synthesis (MIT)',
                            path=str(path.relative_to(ROOT)), seconds=6, builtin=True))
    (folder/'catalog.json').write_text(json.dumps(catalog, indent=2)+'\n')

if __name__ == '__main__':
    main()
