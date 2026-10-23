[简体中文](pocket-bgm-validation.zh_CN.md) · **English**

# Current 1.4: persistent volume

Build: PASS — full validation gate and archived image verification.
Host tests: PASS — all volume levels survive simulated reboot, playback remains stopped, and missing/corrupt data plus initialization/read/write/commit failures are covered.
Device tests: NOT RUN for 1.4.
Unverified: real power-cycle persistence, audio/rendering behavior. No audible tests performed.
Changed volume commits immediately in the player worker. Normal power-off retains it; merged flashing may reset NVS. Only volume is stored.

- Full image SHA-256: `254fafbc78aed3c7f976745019e0a9437eb958b992a2fa598009791a929844e1`
- ELF SHA-256: `0fb5e4bbf1d9d037bfcfc13893fe8866a7bd7c7da08fbdad30362647f096967d`
- Full image bytes: 921360
- Archive: `build/firmware/254fafbc78aed3c7f976745019e0a9437eb958b992a2fa598009791a929844e1/`

## Earlier validation records

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

The eleven-track 1.4 variant was flashed with explicit authorization. Written hash verified; startup identified 1.4.0, matching ELF prefix `fc857efba`, Ready with 219,000 bytes free heap, and verified codec low power. No playback, injected buttons, volume adjustments or power-cycle persistence tests were performed. The three-example public image itself was not flashed.
