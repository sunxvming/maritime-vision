# Sophon Multi-Decoder Migration Guide

## 概述

已将 `ai_service/core/decoder/sophon_video_decoder.py` 从单路 `sail.Decoder` 迁移到多路 `sail.MultiDecoder` 接口，以支持同时解码十几路 RTSP 视频流而不产生严重延迟。

## 主要变更

### 1. 新增 `SophonMultiVideoDecoder` 类

多路解码器，使用 Sophon SAIL SDK 的 `MultiDecoder` 接口实现硬件加速的多通道并发解码。

**特性：**
- 支持同时解码多路 RTSP 流（H.264/H.265）
- 使用 BM1688 VPU 硬件加速
- 每个通道独立的缓存队列（`queue_size` 参数）
- 自动重连机制（每个通道独立重连）
- 线程安全的帧访问
- 运行时动态添加/删除流（`add_stream` / `remove_stream`）

**关键参数：**
```python
SophonMultiVideoDecoder(
    rtsp_urls: List[str],          # RTSP 流 URL 列表
    dev_id: int = 0,               # TPU 设备 ID
    queue_size: int = 10,          # 每路缓存队列长度
    discard_mode: int = 1,         # 缓存满时处理：0=丢弃新帧，1=丢弃最旧帧
    frame_skip_num: int = 0,       # 主动丢帧数（0=不丢帧）
    reconnect_interval: int = 5,   # 重连间隔（秒）
    max_reconnect_attempts: int = -1,  # 最大重连次数（-1=无限）
    rtsp_transport: str = "tcp",   # 传输协议（tcp/udp）
    fps_estimate: float = 25.0,    # 估计帧率
)
```

**核心方法：**
```python
# 启动/停止
decoder.start() -> bool
decoder.stop() -> None

# 运行时添加/删除流
decoder.add_stream(rtsp_url: str) -> bool
decoder.remove_stream(rtsp_url: str) -> bool

# 获取帧（线程安全）
decoder.get_latest_frame(channel_id: int) -> Optional[np.ndarray]
decoder.get_latest_frame_by_url(url: str) -> Optional[np.ndarray]
decoder.get_all_frames() -> Dict[int, Optional[np.ndarray]]

# 查询通道信息
decoder.get_channel_id(url: str) -> Optional[int]
decoder.get_channel_url(channel_id: int) -> Optional[str]
decoder.get_active_channels() -> List[int]
decoder.get_last_frame_time(channel_id: int) -> float
```

### 2. 保留 `SophonVideoDecoder` 类（向后兼容）

原有的单路接口保持不变，内部使用 `SophonMultiVideoDecoder` 实现，确保现有代码无需修改即可运行。

```python
# 原有代码无需修改
decoder = SophonVideoDecoder(
    rtsp_url="rtsp://...",
    dev_id=0,
    buffer_size=10,
)
decoder.start()
frame = decoder.get_latest_frame()
```

## 工作原理

### MultiDecoder 轮询机制

解码线程以轮询方式读取所有通道：
```python
for channel_id, url in channels:
    ret = multi_decoder.read(channel_id, bmimg, read_mode=1)
    # read_mode=1: 阻塞等待直到获取到图像
```

每个通道有独立的：
- 缓存队列（`queue_size` 帧）
- 重连计数器
- 帧缓冲区（BGR numpy array）
- 预分配的 SAIL 对象（BMImage、Tensor，避免重复分配）

### 性能优化点

1. **对象复用**：每个通道预分配 `BMImage`、`Tensor`、`bgr_img`，避免每帧重新分配
2. **分辨率检测**：仅在分辨率变化时重新创建 SAIL 对象
3. **轮询调度**：`read_mode=1` 确保每个通道及时获取帧，避免单通道阻塞其他通道
4. **丢帧策略**：`discard_mode=1` 缓存满时丢弃最旧帧，保持实时性

## 使用示例

### 基础用法（多路解码）

```python
from ai_service.core.decoder import SophonMultiVideoDecoder

urls = [
    "rtsp://192.168.1.101:8554/stream1",
    "rtsp://192.168.1.102:8554/stream2",
    "rtsp://192.168.1.103:8554/stream3",
    # ... 最多支持十几路
]

decoder = SophonMultiVideoDecoder(
    rtsp_urls=urls,
    dev_id=0,
    queue_size=10,        # 每路缓存10帧
    discard_mode=1,       # 缓存满时丢弃最旧帧
    frame_skip_num=0,     # 不主动丢帧
)

with decoder:
    while decoder.is_running():
        # 获取所有通道的最新帧
        frames = decoder.get_all_frames()
        for channel_id, frame in frames.items():
            if frame is not None:
                url = decoder.get_channel_url(channel_id)
                # 处理帧 ...
        time.sleep(0.01)
```

### 运行时动态管理流

```python
# 添加新流
success = decoder.add_stream("rtsp://192.168.1.104:8554/stream4")

# 删除流
success = decoder.remove_stream("rtsp://192.168.1.101:8554/stream1")

# 查询当前活跃通道
active_channels = decoder.get_active_channels()
print(f"Active channels: {len(active_channels)}")
```

### 与 CameraManager 集成（现有架构）

`CameraManager` 现在使用 `SophonVideoDecoder`（单路包装器），无需修改现有代码：

```python
# camera_manager.py 中自动使用正确的解码器
if self._use_sophon:
    decoder = SophonVideoDecoder(  # 内部使用 MultiDecoder
        rtsp_url=camera.rtsp_url,
        dev_id=self._sophon_dev_id,
        buffer_size=10,
    )
```

## 配置说明

在 `config/config.yaml` 中配置解码器后端：

```yaml
decoder:
  backend: sophon  # 使用 Sophon 硬件加速（或 opencv）
  sophon:
    dev_id: 0      # TPU 设备 ID
  buffer_size: 10  # 缓存队列大小
  reconnect_interval: 5
  max_reconnect_attempts: -1
  rtsp_transport: tcp
```

## 性能建议

### 参数调优

1. **queue_size（缓存队列大小）**
   - 推荐值：10
   - 过小：容易丢帧
   - 过大：延迟增加

2. **discard_mode（丢帧策略）**
   - `1`（推荐）：丢弃最旧帧，保持实时性
   - `0`：丢弃新帧，可能导致画面卡顿

3. **frame_skip_num（主动丢帧）**
   - `0`（推荐）：不主动丢帧，由缓存队列自然淘汰
   - `1`：跳过50%帧，减轻负载
   - `2`：跳过66%帧

### 容量规划

BM1688 VPU 解码能力：
- H.264 1080p@30fps：约 16 路
- H.265 1080p@30fps：约 12 路
- 实际容量受分辨率、码率、GOP 影响

建议：
- 单个 `MultiDecoder` 实例控制在 10-15 路
- 超过此数量考虑多 TPU 设备或降低分辨率

## 故障排查

### 问题：部分通道延迟高

**原因**：某通道网络慢导致 `read()` 阻塞时间长
**解决**：
1. 增加 `queue_size`
2. 设置 `frame_skip_num=1` 主动丢帧
3. 检查网络带宽

### 问题：通道频繁重连

**原因**：RTSP 流不稳定或网络问题
**解决**：
1. 检查 RTSP 服务端日志
2. 尝试 `rtsp_transport="udp"`（网络较好时）
3. 增加 `reconnect_interval`

### 问题：内存持续增长

**原因**：帧未正确释放
**解决**：
- 使用 `get_latest_frame()` 返回的是拷贝，自动释放
- 避免在循环中保存大量帧引用

## 迁移检查清单

- [x] `SophonMultiVideoDecoder` 实现多路解码
- [x] `SophonVideoDecoder` 保持向后兼容
- [x] 运行时动态添加/删除流支持
- [x] 线程安全的帧访问
- [x] 每个通道独立重连机制
- [x] 对象复用优化（减少内存分配）
- [x] 更新 `__init__.py` 导出

## 参考资料

- Sophon SAIL SDK 文档：`sophon.sail.MultiDecoder`
- 原单路实现：`video_decoder.py`（OpenCV 方案）
- 集成点：`camera_manager.py`
