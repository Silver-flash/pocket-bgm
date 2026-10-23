[简体中文](pocket-bgm.zh_CN.md) · **English**

# Pocket BGM 1.4 — public edition

See the [README](../README.md) for setup and controls.
The public edition includes three original six-second synthesized examples, 18 seconds total.
Private recordings are not distributed. Startup is silent; saved volume is restored (30% on first use); loop mode resets on startup.
Physical UP/DOWN/OK BSP events map to upper/middle/lower respectively on the user's unit.
The application adapts these events without changing shared board drivers.

`tools/generate_bgm_examples.py` regenerates the MIT examples.
`tools/prepare_bgm.py` packages defaults, or `--library selection.json` packages an explicit
JSON array with path/title/source fields. An empty array builds an empty library.
`tools/bgm_studio.py` manages the local library and never flashes automatically.
All audio processing and codec operations run in the player worker, not key callbacks.
Firmware gain is limited to 5x amplitude before soft limiting; physical output is unverified.

Noto font coverage, decoded audio, input mapping, state transitions and gain have host tests.
Whole-device sleep, online audio extraction are not implemented.

Volume persistence: each changed level is committed to NVS from the player worker before being applied to the codec. Missing settings default to 30%; invalid or unreadable settings use 30% and display a settings error. Initialization failures never erase shared storage. Normal power-off retains volume and startup stays silent; flashing a merged full image can still reset saved data.
