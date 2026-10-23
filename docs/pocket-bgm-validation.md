[简体中文](pocket-bgm-validation.zh_CN.md) · **English**

# Public edition validation

Build: PASS. Host tests: PASS. Device tests: NOT RUN for the public audio pack.
The earlier private audio variant passed flash verification and a bounded startup-log check.
No playback or volume test was performed. Do not treat those startup observations as
validation of screen appearance, physical keys, sound quality, thermal behavior or loudness.
Complete tools/validate.sh passed, including nine upload formats, deletion/restoration, PCM decoding, gain and input mapping.

Validated public example image (local build, not flashed):

- Full image SHA-256: `64e1b287093e716b087b965106849951b795a01742252d608debb317670d66ee`
- ELF SHA-256: `9176c96c8f456768ff200892043d52d39c60f9abee75c39d98b769f81f28a2af`
- Image: 898,272 bytes; audio: 146,688 bytes / 18 seconds.
- Archive: `build/firmware/64e1b287093e716b087b965106849951b795a01742252d608debb317670d66ee/`
