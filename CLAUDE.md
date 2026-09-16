# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述
- **项目名称**：maritime-vision
- **项目简介**：船舶智能监控系统终端支持接入多路网络摄像头，通过智能识别算法对视频帧进行处理、分析，实现自动识别监测船员不安全行为，并触发相应的告警提醒，为船舶航行、作业过程中的安全保驾护航。
- **详细需求**：见 docs/requirements.md，涉及功能开发前请先阅读

## 技术栈
- **AI 服务**（`ai_service/`）：Python 3.8+ / YOLO（Ultralytics）/ ByteTrack / OpenCV / PyYAML，TCP + JSON 与客户端通信。推理后端预留 ONNX Runtime、TensorRT 扩展点。
- **Qt 客户端**（`qt_client/`）：C++17 / Qt 5.15.2（msvc2019_64，Widgets + Network）/ CMake + Ninja / MSVC 2022 工具链 / FFmpeg（拉流解码）/ OpenGL（渲染）。构建通过 `CMakePresets.json`（`MSVC22-Debug-x64` / `MSVC22-Release-x64`）驱动，也可用 `build.bat` / `build.bat -R`。
- **通信协议**：TCP 长连接 + JSON。

## 项目结构
```
maritime-vision/
├── ai_service/           # AI 服务（Python，无 UI）
└── video_system/         # Qt 桌面客户端（C++，不参与 AI 推理）

```

## 常用命令
```bash
# AI 服务：安装依赖并运行
cd ai_service
pip install -r requirements.txt
python main.py


# Qt 客户端构建（Debug 输出到 qt_client/bin/Debug/，Release 输出到 qt_client/bin/Release/）
cd video_system
build.bat        # Debug
build.bat -R     # Release
```




