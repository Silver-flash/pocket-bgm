[简体中文](README.zh_CN.md) · **English**

# Pocket BGM · AI Passport

An offline, button-operated soundtrack player for the FoloToy AI Passport.
Dark cyberpunk UI, local audio uploads, and manual playback with no cloud account.
Maintained by [Silver-flash](https://github.com/Silver-flash).

## Features

- Physical upper / middle / lower buttons: previous / play-stop / next.
- Hold middle to open settings. Upper/lower adjust volume; middle toggles looping.
- Volume: 0–100% in 10-point steps, then 200/300/400/500%. Default 30%.
- Above 100%, digital gain with a soft limiter; 500% is not five times perceived loudness.
- Silent startup, scrolling titles, separated mode/volume, and dimming after 30 seconds.
- Local studio: upload, preview, delete, restore examples, and build firmware.
- MP3, WAV, M4A, AAC, FLAC, OGG, OPUS, AIFF and WMA imports through FFmpeg.
- Three original six-second synthesized examples, removable from the library.

## Run locally

Requirements: Python 3.9+, FFmpeg on PATH, and ESP-IDF **5.5.3** for ESP32-C3
firmware builds. Follow the [environment guide](docs/development/engineering/environment-setup.md).
No Python package installation is required for the studio.

```bash
git clone https://github.com/Silver-flash/pocket-bgm.git
cd pocket-bgm
python3 tools/bgm_studio.py
```

Open the displayed loopback URL (default http://127.0.0.1:8765).
Upload audio and select the firmware-generation button. The device library changes only after
building and flashing. The service processes audio locally and has no remote sharing mode.

```bash
# Set this if ESP-IDF is installed somewhere else:
export AI_PASSPORT_IDF_ROOT="$HOME/esp/esp-idf-v5.5.3"
bash tools/build_bgm.sh
```

Output: `build/FoloToy-AI-Passport-full.bin`, a merged image for offset `0x0`.
Flashing replaces the previous app and may reset stored settings. It does not start playback.
Library limits: 24 clips, 640 seconds total, 32 MiB per upload and 5 MiB encoded audio.
Audio is converted to 16 kHz mono IMA-ADPCM; custom titles use numbered device aliases.

## Validation and privacy

See [validation](docs/pocket-bgm-validation.md) and [application notes](docs/pocket-bgm.md).
Uploads and build outputs are ignored by Git. Regenerating the pack changes tracked
embedded assets: review them before committing or sharing firmware.
The public repository excludes private music, local logs, device identifiers and machine paths.
New commits use the maintainer's GitHub noreply address; upstream attribution is retained.

## License and upstream

Based on [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport), preserving its history
and attribution. Code and procedural examples: [MIT](LICENSE).
Noto font: [SIL OFL](assets/fonts/bgm/OFL.txt). Uploaded audio retains its own rights.
The project's physical middle/lower key mapping follows the tested unit's user feedback;
other hardware revisions may need adjustment.
