# CameraManager 迁移至 SophonMultiVideoDecoder 完成总结

## 迁移概述

已成功将 `CameraManager` 从使用多个独立 `SophonVideoDecoder` 实例（每路一个）迁移到使用单个共享 `SophonMultiVideoDecoder` 实例（所有摄像头共享）。

## 架构变化

### 迁移前（旧架构）
```
CameraManager
├── camera_1 → SophonVideoDecoder (独立 sail.Decoder)
├── camera_2 → SophonVideoDecoder (独立 sail.Decoder)
├── camera_3 → SophonVideoDecoder (独立 sail.Decoder)
└── ...
```
**问题**：十几路摄像头时，每个 `sail.Decoder` 独立工作，导致严重延迟。

### 迁移后（新架构）
```
CameraManager
└── SophonMultiVideoDecoder (单个 sail.MultiDecoder)
    ├── channel_0 → camera_1 (rtsp://...)
    ├── channel_1 → camera_2 (rtsp://...)
    ├── channel_2 → camera_3 (rtsp://...)
    └── ...
```
**优势**：所有摄像头共享一个硬件解码器实例，VPU 统一调度，无阻塞并发解码。

## 核心代码变更

### 1. CameraManager 初始化（新增成员变量）

```python
class CameraManager:
    def __init__(self, decoder_config: dict, camera_repository):
        # Sophon multi-decoder (shared for all cameras)
        self._multi_decoder: Optional[SophonMultiVideoDecoder] = None
        self._camera_url_map: Dict[str, str] = {}  # camera_id -> rtsp_url

        # OpenCV decoders (individual per camera)
        self._decoders: Dict[str, VideoDecoder] = {}  # camera_id -> decoder
```

### 2. start() 方法 - 启动时创建共享解码器

```python
async def start(self) -> None:
    # Load cameras from database
    all_cameras = await self.camera_repository.get_all()
    self.cameras = {cam.id: cam for cam in all_cameras if cam.enabled}

    # Initialize Sophon multi-decoder if using Sophon backend
    if self._use_sophon and self.cameras:
        rtsp_urls = [cam.rtsp_url for cam in self.cameras.values()]
        self._multi_decoder = SophonMultiVideoDecoder(
            rtsp_urls=rtsp_urls,
            dev_id=self._sophon_dev_id,
            queue_size=self.decoder_config.get("buffer_size", 10),
            discard_mode=1,  # 缓存满时丢弃最旧帧
        )

        loop = asyncio.get_event_loop()
        success = await loop.run_in_executor(None, self._multi_decoder.start)

        if success:
            # Build camera_id -> URL mapping
            for camera_id, camera in self.cameras.items():
                self._camera_url_map[camera_id] = camera.rtsp_url
    else:
        # OpenCV backend: start individual decoders
        for camera_id, camera in self.cameras.items():
            await self._start_camera(camera_id, camera)
```

### 3. _start_camera() - 动态添加摄像头

```python
async def _start_camera(self, camera_id: str, camera: CameraConfig) -> bool:
    if self._use_sophon:
        # Sophon: add stream to multi-decoder
        loop = asyncio.get_event_loop()
        success = await loop.run_in_executor(
            None, self._multi_decoder.add_stream, camera.rtsp_url
        )
        if success:
            self._camera_url_map[camera_id] = camera.rtsp_url
        return success
    else:
        # OpenCV: create individual decoder
        decoder = VideoDecoder(rtsp_url=camera.rtsp_url, ...)
        success = await loop.run_in_executor(None, decoder.start)
        if success:
            self._decoders[camera_id] = decoder
        return success
```

### 4. _stop_camera() - 动态移除摄像头

```python
async def _stop_camera(self, camera_id: str) -> None:
    if self._use_sophon:
        # Sophon: remove stream from multi-decoder
        if self._multi_decoder and camera_id in self._camera_url_map:
            url = self._camera_url_map[camera_id]
            await loop.run_in_executor(None, self._multi_decoder.remove_stream, url)
            del self._camera_url_map[camera_id]
    else:
        # OpenCV: stop individual decoder
        if camera_id in self._decoders:
            decoder = self._decoders[camera_id]
            await loop.run_in_executor(None, decoder.stop)
            del self._decoders[camera_id]
```

### 5. get_frame() - 获取帧

```python
def get_frame(self, camera_id: str) -> Optional[np.ndarray]:
    if self._use_sophon and self._multi_decoder:
        url = self._camera_url_map.get(camera_id)
        if not url:
            return None
        return self._multi_decoder.get_latest_frame_by_url(url)
    else:
        decoder = self._decoders.get(camera_id)
        if not decoder:
            return None
        return decoder.get_latest_frame()
```

### 6. _monitor_loop() - 监控所有摄像头状态

```python
async def _monitor_loop(self) -> None:
    while self._running:
        if self._use_sophon and self._multi_decoder:
            # Sophon: check multi-decoder status for all cameras
            for camera_id in list(self._camera_url_map.keys()):
                url = self._camera_url_map.get(camera_id)
                channel_id = self._multi_decoder.get_channel_id(url)
                last_frame_time = self._multi_decoder.get_last_frame_time(channel_id)

                # Check timeout (5 seconds)
                if last_frame_time > 0 and (time.time() - last_frame_time) > 5:
                    self._camera_status[camera_id] = CameraStatus(
                        camera_id=camera_id, online=False
                    )
                else:
                    self._camera_status[camera_id] = CameraStatus(
                        camera_id=camera_id, online=True, fps=self._multi_decoder.get_fps()
                    )
        else:
            # OpenCV: check individual decoders
            for camera_id, decoder in list(self._decoders.items()):
                # ... (same logic per decoder)
```

## 关键设计决策

### 1. 双后端架构
- **Sophon 后端**：单个 `SophonMultiVideoDecoder` + `_camera_url_map`（camera_id → URL 映射）
- **OpenCV 后端**：多个 `VideoDecoder` + `_decoders`（camera_id → decoder 映射）

### 2. 运行时动态管理
- `add_stream()` / `remove_stream()` 支持运行时添加/删除摄像头
- 无需重启整个解码器，热插拔支持

### 3. 线程安全
- 所有阻塞 SAIL 调用通过 `asyncio.run_in_executor()` 在线程池执行
- `SophonMultiVideoDecoder` 内部使用 `threading.Lock` 保护帧缓冲区

### 4. 向后兼容
- 保留 `SophonVideoDecoder` 单路包装器用于测试和向后兼容
- `CameraManager` 现在只使用 `SophonMultiVideoDecoder`

## 性能优化要点

### 1. 缓存队列配置
```python
queue_size=10,       # 每路缓存10帧
discard_mode=1,      # 缓存满时丢弃最旧帧（保持实时性）
frame_skip_num=0,    # 不主动丢帧（由缓存队列自然淘汰）
```

### 2. 对象复用
- 每个通道预分配 `BMImage`、`Tensor`、`bgr_img` 对象
- 仅在分辨率变化时重新创建

### 3. 轮询调度
- `read_mode=1`：阻塞等待帧到达，避免 CPU 空转
- Round-robin 方式遍历所有通道

## 测试验证

### 单元测试
```bash
cd ai_service
python tests/test_multi_decoder.py --mode multi   # 测试多路解码
python tests/test_multi_decoder.py --mode single  # 测试单路包装器
```

### 集成测试
启动 AI 服务后观察日志：
```
INFO - Sophon multi-decoder started with 12 cameras
INFO - Camera cam_001 added to multi-decoder
INFO - Camera cam_002 added to multi-decoder
...
```

## 故障排查

### 问题1：部分摄像头无法解码
**原因**：RTSP URL 格式错误或网络不可达
**解决**：检查数据库中的 `rtsp_url` 字段，确保格式正确（如 `rtsp://192.168.110.241:8554/video1/`）

### 问题2：解码器频繁重连
**原因**：网络不稳定或 RTSP 服务器负载过高
**解决**：调整 `reconnect_interval` 和 `max_reconnect_attempts` 参数

### 问题3：帧延迟仍然存在
**原因**：`queue_size` 过大或 `discard_mode=0`（丢弃新帧）
**解决**：设置 `discard_mode=1` 并适当降低 `queue_size`（如5-10）

## 文件清单

### 修改的文件
- `ai_service/core/camera/camera_manager.py` - 重构为使用 `SophonMultiVideoDecoder`
- `ai_service/core/decoder/sophon_video_decoder.py` - 新增 `SophonMultiVideoDecoder` 类

### 新增的文件
- `docs/claude_sophon_multi_decoder_migration.md` - 解码器迁移指南
- `docs/claude_camera_manager_migration_summary.md` - 本文件（迁移总结）
- `ai_service/tests/test_multi_decoder.py` - 多路解码器测试脚本

### 未修改的文件（无缝兼容）
- `ai_service/main.py` - 无需修改
- `ai_service/core/inference/inference_manager.py` - 无需修改
- `ai_service/core/api/tcp_server.py` - 无需修改

## 预期效果

### 性能提升
- **延迟降低**：从多个独立解码器的累积延迟（可能数秒）降低到统一调度的毫秒级延迟
- **资源利用**：VPU 统一调度，避免硬件竞争
- **帧率稳定**：所有通道共享缓存策略，帧率更均衡

### 可扩展性
- 支持运行时动态添加/删除摄像头
- 支持最多 10-15 路并发解码（受 BM1688 VPU 硬件限制）

### 兼容性
- OpenCV 后端保持不变
- 向后兼容原有 `SophonVideoDecoder` 接口

## 后续建议

1. **监控指标**：在生产环境添加 Prometheus 指标监控多路解码器性能
2. **动态配置**：将 `queue_size`、`discard_mode` 等参数暴露到配置文件
3. **健康检查**：添加 `/health` 端点检查多路解码器状态
4. **日志优化**：为每个通道添加独立日志流，便于调试

---

**迁移完成时间**：2026-08-19  
**技术栈版本**：Sophon SAIL SDK（BM1688）/ Python 3.8+
