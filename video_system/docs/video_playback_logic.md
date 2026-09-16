# Qt 视频播放逻辑详解

## 1. 整体架构

视频播放系统采用多线程架构，主要包含以下核心组件：

```
FFmpegThread (主解码线程)
    ├── FFmpegSync (视频同步线程)
    ├── FFmpegSync (音频同步线程)
    └── OpenGL渲染控件 (YuvWidget/Nv12Widget/RgbWidget)
```

## 2. 核心类职责

### 2.1 FFmpegThread (core_videoffmpeg/ffmpegthread.cpp)
主解码线程，负责：
- 初始化 FFmpeg 解码器和格式上下文
- 从视频源读取数据包（av_read_frame）
- 将数据包分发给视频/音频同步线程
- 管理解码资源和线程生命周期

### 2.2 FFmpegSync (core_videoffmpeg/ffmpegsync.cpp)
音视频同步线程（实例化两个，一个处理视频，一个处理音频），负责：
- 维护数据包队列
- **计算音视频同步时间（PTS）**
- 控制播放节奏
- 触发解码并发送帧数据

### 2.3 YuvWidget/Nv12Widget (core_videoopengl/)
OpenGL 渲染控件，负责：
- 接收解码后的 YUV/NV12 数据
- 通过 OpenGL shader 转换为 RGB 并渲染
- 刷新画面显示

## 3. 视频播放流程

### 3.1 初始化阶段

```
1. FFmpegThread::openVideo()
   ├─ initOption()          // 设置 FFmpeg 选项（超时、缓存、协议等）
   ├─ initInput()           // 打开输入源，创建 AVFormatContext
   ├─ initVideo()           // 初始化视频解码器
   ├─ initAudio()           // 初始化音频解码器
   └─ initOther()           // 分配帧内存、创建图像转换上下文
       └─ videoSync->start()   // 启动视频同步线程
       └─ audioSync->start()   // 启动音频同步线程
```

### 3.2 运行阶段（主循环）

**FFmpegThread::run()** 主循环逻辑：

```cpp
while (!stopped) {
    // 1. 检查暂停状态
    if (isPause || changePosition) {
        msleep(1);
        continue;
    }
    
    // 2. 检查同步队列是否过载（防止内存溢出）
    if (videoSync->getPacketCount() >= 100 || audioSync->getPacketCount() >= 100) {
        msleep(1);  // ⚠️ 卡顿点1：队列满时等待
        continue;
    }
    
    // 3. 读取一帧数据包
    int result = av_read_frame(formatCtx, packet);
    
    // 4. 分发数据包
    if (index == videoIndex) {
        decodeVideo0(packet);  // 视频包 → 视频同步队列
    } else if (index == audioIndex) {
        decodeAudio0(packet);  // 音频包 → 音频同步队列
    }
    
    msleep(1);  // ⚠️ 卡顿点2：主循环固定延时1ms
}
```

### 3.3 同步线程处理（FFmpegSync::run）

```cpp
while (!stopped) {
    if (packets.size() > 0) {
        AVPacket *packet = packets.first();
        
        // 1. 计算 PTS 时间
        ptsTime = FFmpegHelper::getPtsTime(thread->formatCtx, packet);
        
        // 2. 检查是否到播放时间（音视频同步核心）
        if (!checkPtsTime()) {
            msleep(1);  // ⚠️ 卡顿点3：未到播放时间，等待1ms
            continue;
        }
        
        // 3. 解码并显示
        if (type == 1) {  // 视频
            thread->decodeVideo1(packet);  // 解码
            // → decodeVideo2()
            // → emit receiveFrame()  // 发送帧数据到渲染控件
        }
        
        // 4. 移除已处理的包
        packets.removeFirst();
    }
    
    msleep(1);  // ⚠️ 卡顿点4：同步线程固定延时1ms
}
```

### 3.4 PTS 同步算法 (FFmpegSync::checkPtsTime)

```cpp
bool FFmpegSync::checkPtsTime() {
    if (ptsTime > 0) {
        // 计算当前播放偏移时间
        offsetTime = (av_gettime() - startTime) * thread->speed + bufferTime;
        
        // 判断是否到达播放时间
        int offset = (type == 0 ? 1000 : 5000);  // 音频1ms，视频5ms
        if ((offsetTime <= ptsTime && ptsTime - offsetTime <= offset) 
            || (offsetTime > ptsTime)) {
            return true;  // 可以播放
        }
    }
    return false;  // 未到播放时间
}
```

### 3.5 渲染阶段

```
receiveFrame 信号
    ↓
YuvWidget::updateFrame()
    ↓
YuvWidget::update()         // 触发 Qt 重绘
    ↓
YuvWidget::paintGL()        // OpenGL 渲染
    ├─ glTexImage2D()       // 上传 YUV 纹理到 GPU
    └─ glDrawArrays()       // 绘制到屏幕
```
