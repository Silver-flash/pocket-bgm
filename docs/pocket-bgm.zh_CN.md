**简体中文** · [English](pocket-bgm.md)

# 随身 BGM 1.3 — 公开版

安装和操作见 [README](../README.zh_CN.md)。
公开版包含 3 段各 6 秒的原创合成示例，共 18 秒，不分发私人录音。
开机不播放，设置重置为 30% 音量和循环模式。
当前设备的 BSP UP/DOWN/OK 分别对应物理上/中/下，应用层统一映射，不修改共享板级驱动。

`tools/generate_bgm_examples.py` 重新生成 MIT 示例。
`tools/prepare_bgm.py` 默认打包示例，或用 `--library selection.json` 指定
含 path/title/source 的 JSON 数组；空数组会生成空曲库。
`tools/bgm_studio.py` 管理本机曲库，不自动刷机。
所有音频处理及芯片操作都在播放任务执行，不阻塞按键回调。
软限幅前最大振幅增益 5 倍，物理输出尚未验证。

字体覆盖、解码、按键映射、播放状态和增益均有主机测试。
尚未实现整机休眠、音量持久化和在线音频提取。
