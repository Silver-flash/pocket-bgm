[简体中文](pocket-bgm.zh_CN.md) · **English**

# Pocket BGM 1.3 — public edition

See the [README](../README.md) for setup and controls.
The public edition includes three original six-second synthesized examples, 18 seconds total.
Private recordings are not distributed. Startup is silent; settings reset to 30% and loop mode.
Physical UP/DOWN/OK BSP events map to upper/middle/lower respectively on the user's unit.
The application adapts these events without changing shared board drivers.

`tools/generate_bgm_examples.py` regenerates the MIT examples.
`tools/prepare_bgm.py` packages defaults, or `--library selection.json` packages an explicit
JSON array with path/title/source fields. An empty array builds an empty library.
`tools/bgm_studio.py` manages the local library and never flashes automatically.
All audio processing and codec operations run in the player worker, not key callbacks.
Firmware gain is limited to 5x amplitude before soft limiting; physical output is unverified.

Noto font coverage, decoded audio, input mapping, state transitions and gain have host tests.
Whole-device sleep, persistent volume and online audio extraction are not implemented.
