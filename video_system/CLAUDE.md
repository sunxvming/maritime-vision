# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 提供在本仓库中处理代码时的指导。

## 项目概述

这是一个基于 Qt/C++ 的智能视频监控系统。



## 构建系统

### CMake 构建（推荐）

**快速开始：**
```bash
# Windows - 使用构建脚本
build.bat          # Debug 构建
build.bat -R       # Release 构建
```


**CMake 结构：**
```
video_system/
├── CMakeLists.txt              # 主配置（Qt检测、编译器标志、功能定义）
├── CMakePresets.json           # MSVC预设配置
├── core_*/
│   └── CMakeLists.txt         # 每个核心模块（静态库）
└── video_system/
    └── CMakeLists.txt         # 主应用（链接所有模块）
```



## 项目结构

### 核心模块（core_*）

`core_*` 目录包含可复用的、与架构无关的组件：

- **core_audio**：音频播放/录制、音频电平可视化
- **core_common**：通用工具（日志、加密、无边框窗口）
- **core_control**：自定义 UI 控件（开关、设备按钮、颜色组合框）
- **core_dataout**：数据导出（Excel/PDF）、打印功能
- **core_db**：数据库管理、分页、自动清理线程
- **core_form**：用户登录/注销、用户管理、数据库配置 UI
- **core_onvif**：ONVIF 协议实现（设备搜索、PTZ 控制）
- **core_qui**：通用 UI 框架（消息框、对话框、样式）
- **core_video**：视频播放核心（解码器抽象、控件基类）
- **core_videobase**：视频线程/控件的基类
- **core_videoffmpeg**：FFmpeg 解码器实现（默认）
- **core_videoopengl**：OpenGL 渲染（YUV/NV12）
- **core_videosave**：视频/音频文件保存
- **core_webview**：浏览器控件封装（WebKit/WebEngine/miniblink）

### 应用模块

- **class/**：业务逻辑（设备通信、视频管理）
  - `app/`：全局配置、初始化、样式
  - `devicevideo/`：视频相关辅助（ONVIF 集成、地图管理）
  - `usercontrol/`：自定义控件（PTZ 仪表盘）

- **ui/**：按功能组织的所有 UI 窗体
  - `frmmain/`：主窗口、登录/注销
  - `frmvideo/`：视频监控、回放
  - `frmconfig/`：系统设置、摄像头/NVR 管理
  - `frmdata/`：日志查询
  - `frmipc/`：摄像头控制（PTZ、预置位）
  - `frmmodule/`：可停靠的子模块

- **video_system/**：主应用程序入口


## Qt UI Development Rules
所有 Qt 界面必须采用 Qt Designer（.ui）开发。禁止用纯 C++ 拼接复杂界面。
大部分窗口应对应三个文件,比如：1. xx.ui 2. xx.h 3. xx.cpp

### 代码风格
- 类命名：小写加下划线（`video_thread`）
- UI 文件：`frm` 前缀（`frmmain`、`frmvideo`）
- 核心模块：`core_` 前缀
- 信号/槽遵循 Qt 惯例
