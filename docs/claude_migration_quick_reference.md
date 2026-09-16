# Sophon MultiDecoder 迁移快速参考

## 核心变化总结

### 架构变化
```
旧：每个摄像头 → 独立 SophonVideoDecoder → 独立 sail.Decoder
新：所有摄像头 → 共享 SophonMultiVideoDecoder → 单个 sail.MultiDecoder
```

### CameraManager 变化

| 项目 | 旧实现 | 新实现 |
|------|--------|--------|
| 解码器管理 | `_decoders: Dict[str, SophonVideoDecoder]` | `_multi_decoder: SophonMultiVideoDecoder`<br>`_camera_url_map: Dict[str, str]` |
| 启动方式 | 每个摄像头创建独立解码器 | 一次性创建共享解码器并添加所有摄像头 |
| 添加摄像头 | `decoder = SophonVideoDecoder(...)`<br>`decoder.start()` | `multi_decoder.add_stream(url)` |
| 移除摄像头 | `decoder.stop()` | `multi_decoder.remove_stream(url)` |
| 获取帧 | `decoder.get_latest_frame()` | `multi_decoder.get_latest_frame_by_url(url)` |

## 关键代码片段

### 1. 初始化多路解码器（启动时）
```python
# 收集所有摄像头 URL
rtsp_urls = [cam.rtsp_url for cam in self.cameras.values()]

# 创建共享解码器
self._multi_decoder = SophonMultiVideoDecoder(
    rtsp_urls=rtsp_urls,
    dev_id=0,                    # TPU 设备 ID
    queue_size=10,               # 每路缓存10帧
    discard_mode=1,              # 缓存满时丢弃最旧帧
    frame_skip_num=0,            # 不主动丢帧
    reconnect_interval=5,        # 重连间隔5秒
    max_reconnect_attempts=-1,   # 无限重连
    rtsp_transport="tcp",        # TCP 传输
)

# 启动解码器
success = self._multi_decoder.start()

# 建立 camera_id -> URL 映射
for camera_id, camera in self.cameras.items():
    self._camera_url_map[camera_id] = camera.rtsp_url
```

### 2. 运行时添加摄像头
```python
# 添加新的 RTSP 流
success = self._multi_decoder.add_stream(camera.rtsp_url)
if success:
    self._camera_url_map[camera_id] = camera.rtsp_url
```

### 3. 运行时移除摄像头
```python
# 移除 RTSP 流
url = self._camera_url_map.get(camera_id)
if url:
    self._multi_decoder.remove_stream(url)
    del self._camera_url_map[camera_id]
```

### 4. 获取帧
```python
# 通过 camera_id 获取帧
url = self._camera_url_map.get(camera_id)
frame = self._multi_decoder.get_latest_frame_by_url(url)

# 或者通过 channel_id 获取帧
channel_id = self._multi_decoder.get_channel_id(url)
frame = self._multi_decoder.get_latest_frame(channel_id)

# 获取所有通道的帧
frames = self._multi_decoder.get_all_frames()  # Dict[channel_id, frame]
```

### 5. 监控状态
```python
# 检查解码器是否运行
is_running = self._multi_decoder.is_running()

# 获取通道 ID
channel_id = self._multi_decoder.get_channel_id(url)

# 获取最后一帧时间戳（用于超时检测）
last_time = self._multi_decoder.get_last_frame_time(channel_id)

# 检查是否超时（5秒无帧）
if time.time() - last_time > 5:
    # 摄像头离线
    pass
```

## 配置参数说明

| 参数 | 默认值 | 说明 | 调优建议 |
|------|--------|------|----------|
| `queue_size` | 10 | 每路缓存帧数 | 实时性要求高：5-10<br>稳定性要求高：15-20 |
| `discard_mode` | 1 | 缓存满时策略<br>0=丢弃新帧<br>1=丢弃旧帧 | 实时监控必须设为 1 |
| `frame_skip_num` | 0 | 主动跳帧数<br>0=不跳帧<br>1=跳过50%<br>2=跳过66% | CPU负载高时设为 1 或 2 |
| `reconnect_interval` | 5 | 重连间隔（秒） | 网络稳定：5-10秒<br>网络不稳定：15-30秒 |
| `max_reconnect_attempts` | -1 | 最大重连次数<br>-1=无限 | 测试环境：10-20次<br>生产环境：-1（无限） |
| `rtsp_transport` | "tcp" | 传输协议<br>"tcp" 或 "udp" | 有线网络：tcp<br>无线网络：udp |

## 性能调优场景

### 场景1：低延迟实时监控（推荐）
```python
queue_size=5,          # 小缓存，减少延迟
discard_mode=1,        # 丢弃旧帧
frame_skip_num=0,      # 不跳帧
```

### 场景2：高稳定性录像
```python
queue_size=20,         # 大缓存，防止丢帧
discard_mode=0,        # 保留新帧（录像完整性）
frame_skip_num=0,      # 不跳帧
```

### 场景3：高负载省资源
```python
queue_size=10,         # 中等缓存
discard_mode=1,        # 丢弃旧帧
frame_skip_num=1,      # 跳过50%帧（降低CPU负载）
```

## 故障排查清单

### 问题：多路解码器启动失败
```python
# 检查点1：SAIL SDK 是否安装
import sophon.sail as sail
print(sail.__version__)

# 检查点2：设备ID是否正确
import sophon.sail as sail
handle = sail.Handle(0)  # dev_id=0

# 检查点3：RTSP URL 格式
# 正确：rtsp://192.168.110.241:8554/video1/
# 错误：http://192.168.110.241:8554/video1/
```

### 问题：某个摄像头无法解码
```python
# 检查点1：URL 是否可达
import cv2
cap = cv2.VideoCapture("rtsp://192.168.110.241:8554/video1/")
ret, frame = cap.read()
print(ret)  # 应该是 True

# 检查点2：通道是否添加成功
channel_id = multi_decoder.get_channel_id(url)
print(channel_id)  # 应该 >= 0

# 检查点3：检查日志
# 查看 "Failed to add channel" 或 "Read failed" 错误
```

### 问题：帧率低或延迟高
```python
# 解决方案1：减小 queue_size
queue_size=5  # 从 10 降到 5

# 解决方案2：确认 discard_mode=1
discard_mode=1  # 丢弃旧帧

# 解决方案3：启用跳帧
frame_skip_num=1  # 跳过50%帧
```

### 问题：频繁重连
```python
# 解决方案1：增加重连间隔
reconnect_interval=10  # 从 5 增加到 10

# 解决方案2：检查网络稳定性
# 使用 ffplay 测试 RTSP 流是否稳定
ffplay -rtsp_transport tcp rtsp://192.168.110.241:8554/video1/

# 解决方案3：切换传输协议
rtsp_transport="udp"  # 尝试 UDP（某些场景下更稳定）
```

## 测试验证

### 单元测试
```bash
cd ai_service
python tests/test_multi_decoder.py --mode multi
```

### 集成测试
```bash
# 启动 AI 服务
python main.py

# 观察日志输出
# 应该看到：
# INFO - Sophon multi-decoder started with N cameras
# INFO - Camera xxx added to multi-decoder
```

### 性能测试
```python
import time

# 测试帧率
start = time.time()
count = 0
while time.time() - start < 10:
    frames = multi_decoder.get_all_frames()
    count += len([f for f in frames.values() if f is not None])

fps = count / 10.0
print(f"Average FPS: {fps}")  # 应该接近 25 * 摄像头数量
```

## 兼容性说明

### 向后兼容
- 保留 `SophonVideoDecoder` 单路包装器
- OpenCV 后端（`VideoDecoder`）保持不变
- 现有代码无需修改即可运行

### API 对比

| 功能 | SophonVideoDecoder (旧) | SophonMultiVideoDecoder (新) |
|------|------------------------|----------------------------|
| 单路解码 | ✓ | ✓ (通过 wrapper) |
| 多路解码 | ✗ (需要多个实例) | ✓ (单个实例) |
| 运行时添加流 | ✗ | ✓ |
| 运行时删除流 | ✗ | ✓ |
| 通过 URL 获取帧 | ✗ | ✓ |
| 获取所有帧 | ✗ | ✓ |
| 性能 | 延迟高 | 延迟低 |

## 参考文档

- **详细迁移指南**：`docs/claude_sophon_multi_decoder_migration.md`
- **迁移总结**：`docs/claude_camera_manager_migration_summary.md`
- **测试脚本**：`ai_service/tests/test_multi_decoder.py`
- **源代码**：`ai_service/core/decoder/sophon_video_decoder.py`

---

**文档版本**：1.0  
**更新时间**：2026-08-19
