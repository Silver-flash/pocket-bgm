**简体中文** · [English](pocket-bgm-validation.md)

# 公开版验证

构建：PASS。主机测试：PASS。实机测试：公开音频资源包未运行。
此前私人曲库版本通过刷入校验和有限启动日志检查，未进行播放或音量测试。
这些启动观察不能证明屏幕效果、实体按键、音质、温升或实际响度。
完整 tools/validate.sh 已通过，包含九种上传格式、删除恢复、PCM 解码、增益及按键映射。

公开示例固件（本机构建，未刷入）：

- Full image SHA-256: `64e1b287093e716b087b965106849951b795a01742252d608debb317670d66ee`
- ELF SHA-256: `9176c96c8f456768ff200892043d52d39c60f9abee75c39d98b769f81f28a2af`
- Image: 898,272 bytes; audio: 146,688 bytes / 18 seconds.
- Archive: `build/firmware/64e1b287093e716b087b965106849951b795a01742252d608debb317670d66ee/`
