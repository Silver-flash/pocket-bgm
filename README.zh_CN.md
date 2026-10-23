**简体中文** · [English](README.md)

# 随身 BGM · AI Passport

给自己的生活手动加一段 BGM：面向 FoloToy AI Passport 的离线播放器，
深色赛博朋克界面、本地音频上传、实体按键播放，无需云账户。
维护者：[Silver-flash](https://github.com/Silver-flash)。

## 功能

- 物理上 / 中 / 下键：上一首 / 播放或停止 / 下一首。
- 长按中键进入设置，上下调音量，中键切换循环或单次播放。
- 音量 0–100% 每次 10%，之后为 200/300/400/500%，默认 30%。
- 超过 100% 使用数字增益与软限幅；500% 不代表听感响度达到五倍。
- 开机不自动播放，曲名滚动，模式与音量分栏，30 秒无操作变暗。
- 本地后台支持上传、试听、删除、恢复示例和生成固件。
- 通过 FFmpeg 导入 MP3、WAV、M4A、AAC、FLAC、OGG、OPUS、AIFF、WMA。
- 内置 3 段各 6 秒原创合成示例，可自行删除。

## 本地运行

需要 Python 3.9+、PATH 中的 FFmpeg；构建固件需要 ESP-IDF **5.5.3**，
目标芯片 ESP32-C3。详见[环境指南](docs/development/engineering/environment-setup.zh_CN.md)。
曲库后台不需要安装额外 Python 包。

```bash
git clone https://github.com/Silver-flash/pocket-bgm.git
cd pocket-bgm
python3 tools/bgm_studio.py
```

打开显示的本机地址（默认 http://127.0.0.1:8765），上传音频并点击“生成设备安装文件”。
重新生成并刷入后，设备曲库才会更新。音频在本机处理，无远程分享模式。

```bash
# 如果 ESP-IDF 安装在其他位置，请修改：
export AI_PASSPORT_IDF_ROOT="$HOME/esp/esp-idf-v5.5.3"
bash tools/build_bgm.sh
```

产物 `build/FoloToy-AI-Passport-full.bin` 是从 `0x0` 写入的合并固件。
刷写会替换旧应用，可能重置设置；启动不会自动播放。
曲库上限：24 段、总计 640 秒、单文件 32 MiB、编码后 5 MiB。
设备音频转换为 16 kHz 单声道 IMA-ADPCM，自定义上传在设备上使用编号别名。

## 验证与隐私

见[验证记录](docs/pocket-bgm-validation.zh_CN.md)和[应用说明](docs/pocket-bgm.zh_CN.md)。
上传文件及构建产物已被 Git 忽略。重新生成资源包会修改受版本管理的内嵌资源，
提交代码或分享固件前请检查是否包含自己的音频。
公开仓库不包含私人音乐、本机日志、设备标识或机器绝对路径。
新提交使用维护者的 GitHub 隐私邮箱，保留上游作者署名。

## 许可证与上游

基于 [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport)，保留其历史与署名。
代码及原创合成示例：[MIT](LICENSE)；Noto 字体：[SIL OFL](assets/fonts/bgm/OFL.txt)。
上传音频的权利归其原权利人。中键/下键映射依据当前设备的用户反馈修正，
其他硬件修订可能需要调整。
