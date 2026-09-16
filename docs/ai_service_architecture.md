# AI Service 架构详解

> 本文档详细介绍 `ai_service` 的代码逻辑和架构设计。针对 Python 初学者，关键概念会额外说明。

## 目录

- [1. 整体架构](#1-整体架构)
- [2. 一帧图像的完整旅程](#2-一帧图像的完整旅程)
- [3. 模块详解](#3-模块详解)
- [4. 配置文件详解](#4-配置文件详解)
- [5. TCP 通信协议](#5-tcp-通信协议)
- [6. HTTP API 接口](#6-http-api-接口)
- [7. 关键 Python 概念说明](#7-关键-python-概念说明)

---

## 1. 整体架构

### 1.1 系统组成

AI Service 同时运行三个服务：

| 服务 | 端口 | 用途 |
|------|------|------|
| 主处理循环 | — | 视频采集 → 推理 → 报警 |
| TCP Server | 8888 | 实时推送检测/报警数据给 Qt 客户端 |
| HTTP API | 8889 | 摄像头管理 CRUD（供 Qt 客户端调用）|

### 1.2 数据流向

```
RTSP 摄像头
    │
    ▼
VideoDecoder (OpenCV 解码)
    │ BGR 图像帧 (numpy array)
    ▼
MultiModelInference (5个YOLO模型并行推理)
    │ 检测框列表 [Detection]
    ▼
ByteTracker (跨帧目标跟踪)
    │ 带 TrackID 的检测框
    ▼
EventEngine (连续帧判断)
    │ 触发报警的检测框
    ▼
AlarmManager (冷却去重)
    │ AlarmEvent
    ▼
TCPServer → Qt 客户端
```

### 1.3 目录结构

```
ai_service/
├── main.py                       # 主程序，启动入口
├── requirements.txt              # 依赖包列表
├── config/
│   └── config.yaml               # 所有配置项
├── core/                         # 核心业务逻辑
│   ├── models.py                 # 数据结构定义（纯数据，无逻辑）
│   ├── alarm/
│   │   └── alarm_manager.py      # 报警生成和冷却管理
│   ├── api/                      # HTTP API (FastAPI)
│   │   ├── app.py                # FastAPI 应用工厂
│   │   ├── dependencies.py       # 依赖注入
│   │   ├── schemas.py            # 请求/响应数据格式
│   │   └── routes/
│   │       ├── cameras.py        # 摄像头 CRUD 接口
│   │       └── health.py         # 健康检查接口
│   ├── camera/
│   │   └── camera_manager.py     # 摄像头连接管理
│   ├── database/
│   │   ├── connection.py         # SQLite 数据库连接
│   │   ├── models.py             # ORM 数据库表结构
│   │   └── camera_repository.py  # 摄像头数据增删改查
│   ├── decoder/
│   │   └── video_decoder.py      # RTSP 视频流解码
│   ├── event_engine/
│   │   └── event_engine.py       # 报警触发逻辑
│   ├── inference/
│   │   ├── inference_manager.py  # 单模型推理封装
│   │   └── multi_model_inference.py  # 多模型并行推理
│   ├── tcp/
│   │   └── tcp_server.py         # 异步 TCP 服务器
│   ├── tracker/
│   │   └── byte_tracker.py       # ByteTrack 目标跟踪
│   └── utils/
│       ├── config_loader.py      # YAML 配置加载
│       ├── encryption.py         # 密码加密工具
│       └── logger.py             # 日志封装
├── models/                       # YOLO 模型文件（不入 git）
├── data/
│   └── cameras.db                # SQLite 数据库文件
├── logs/                         # 日志文件
├── tests/                        # 测试脚本
└── tools/
    └── download_models.py        # 自动下载模型脚本
```

---

## 2. 一帧图像的完整旅程

理解这个流程是理解整个系统的关键。

### 步骤 1：解码视频帧

`VideoDecoder` 用 OpenCV 连接 RTSP 摄像头，每隔一段时间读取一帧图像。

- 图像格式：**BGR**（蓝绿红），这是 OpenCV 默认格式，与常见的 RGB 顺序相反
- 数据类型：**numpy array**，形状是 `(高度, 宽度, 3)`，例如 `(1080, 1920, 3)`
- 帧存入摄像头的 `buffer`（缓冲队列），主循环来取

### 步骤 2：多模型并行推理

`MultiModelInferenceManager` 把这一帧图像同时送给 5 个 YOLO 模型。

- 使用 `ThreadPoolExecutor`（线程池）并发执行，不是一个个串行跑
- 每个模型返回一批 `Detection` 对象（检测结果）
- 最后把所有模型的结果合并成一个列表

每个 `Detection` 包含：

```python
Detection(
    track_id=0,           # 初始为 0，Tracker 会赋予真正的 ID
    label="smoking",      # 类别名
    confidence=0.92,      # 置信度 0~1
    bbox=BoundingBox(     # 归一化坐标（相对于图像宽高，0~1 之间）
        x=0.5,            # 中心点 x
        y=0.3,            # 中心点 y
        w=0.2,            # 宽度
        h=0.4             # 高度
    ),
    alarm_type="smoking"  # 对应的报警类型
)
```

> **什么是归一化坐标？** 例如图像宽 1920，检测框中心 x=960，则归一化后 x=960/1920=0.5。用归一化坐标的好处是不受分辨率影响。

### 步骤 3：目标跟踪

`ByteTracker` 把当前帧的检测框与上一帧的检测框做关联，为每个目标赋予持久的 `track_id`。

- 同一个人在连续帧中会保持相同的 `track_id`（比如 ID=3）
- 如果某帧漏检，跟踪器会尝试用上一帧的轨迹预测位置
- 目标消失超过一定帧数后，该 ID 被释放

**为什么需要跟踪？** 因为报警逻辑要求"同一个人连续 N 帧都被检测到"，没有稳定的 ID 就无法实现这个需求。

### 步骤 4：事件引擎判断

`EventEngine` 记录每个 `(camera_id, track_id, alarm_type)` 组合的**连续帧计数**。

```
第1帧：track_id=3 检测到 smoking → 计数=1
第2帧：track_id=3 检测到 smoking → 计数=2
...
第15帧：track_id=3 检测到 smoking → 计数=15 ≥ 阈值(15) → 触发报警！
```

如果中途某帧没检测到，计数归零重新开始。

**为什么要连续帧才触发？** 防止单帧误检触发大量假报警。

### 步骤 5：报警管理

`AlarmManager` 生成 `AlarmEvent` 对象，并执行冷却机制：

- 同一个 `track_id` 触发报警后，**30 秒内不再重复报警**
- 报警缓存最多保留 1000 条

### 步骤 6：通过 TCP 推送

`TCPServer` 将检测结果和报警事件序列化为 JSON，通过 TCP 广播给所有已连接的 Qt 客户端。

---

## 3. 模块详解

### 3.1 main.py — 主程序入口

**文件位置：** `ai_service/main.py`

`AIService` 类是整个服务的"总指挥"。它在 `__init__` 方法中创建所有子模块，在 `run()` 方法中启动所有服务。

关键方法：

| 方法 | 作用 |
|------|------|
| `__init__()` | 加载配置，初始化所有模块 |
| `initialize()` | 异步初始化（数据库、摄像头） |
| `run()` | 启动主循环 + TCP服务器 + HTTP API（三者并发） |
| `_process_loop()` | 主处理循环，每帧执行推理→跟踪→事件→报警 |
| `_process_camera()` | 处理单个摄像头的一帧 |

启动流程（`run()` 内部）：

```python
# 伪代码说明启动过程
async def run():
    await initialize()          # 初始化数据库、加载摄像头
    await asyncio.gather(       # 同时启动三个任务（并发，不是并行）
        _process_loop(),        # 主处理循环
        tcp_server.start(),     # TCP 服务器
        http_api.start(),       # HTTP API 服务器
    )
```

> **asyncio.gather 说明：** 这是 Python 异步编程中同时运行多个"协程"的方式。三个任务交替执行，不会互相阻塞。

---

### 3.2 core/models.py — 数据结构定义

**文件位置：** `ai_service/core/models.py`

这个文件只定义数据的"形状"，不包含业务逻辑。使用 Python 的 `dataclass` 装饰器，类似 C++ 的 struct。

```python
@dataclass
class BoundingBox:
    x: float   # 中心点 x（归一化，0~1）
    y: float   # 中心点 y（归一化，0~1）
    w: float   # 宽度（归一化，0~1）
    h: float   # 高度（归一化，0~1）

@dataclass
class Detection:
    track_id: int          # 跟踪ID，0 表示未分配
    label: str             # 类别名，如 "smoking"
    confidence: float      # 置信度，0~1
    bbox: BoundingBox      # 边界框
    alarm_type: str = ""   # 报警类型，如 "smoking"

@dataclass
class AlarmEvent:
    id: str                # UUID，唯一标识
    camera_id: str         # 摄像头ID
    camera_name: str       # 摄像头名称
    alarm_type: str        # 报警类型
    description: str       # 文字描述，如 "检测到人员吸烟"
    timestamp: float       # Unix 时间戳
    track_id: int          # 触发报警的目标 ID
    bbox: BoundingBox      # 目标位置
    confidence: float      # 置信度
```

---

### 3.3 core/decoder/video_decoder.py — 视频解码

**文件位置：** `ai_service/core/decoder/video_decoder.py`

`VideoDecoder` 负责从 RTSP URL 拉取视频流并解码成图像帧。

**关键流程：**

```
connect() → 用 OpenCV 连接 RTSP URL
    ↓
read_loop() → 在独立线程中循环读帧（不能用 async，OpenCV 是同步的）
    ↓
帧存入 buffer（collections.deque，最大 10 帧）
    ↓
主循环调用 get_frame() 取最新帧
```

**断线重连逻辑：**

```python
# 伪代码
while True:
    frame = cap.read()          # 读一帧
    if frame is None:           # 读失败（网络断了）
        reconnect_attempts += 1
        wait(reconnect_interval)  # 等待几秒
        cap = reconnect()       # 重新连接
    else:
        buffer.append(frame)    # 正常：存入缓冲
```

**配置项说明：**

| 配置项 | 说明 | 默认值 |
|--------|------|--------|
| `buffer_size` | 最多缓存多少帧 | 10 |
| `reconnect_interval` | 断线后等待几秒重连 | 5 |
| `max_reconnect_attempts` | 最多重试几次（-1=无限） | -1 |
| `rtsp_transport` | RTSP 传输协议 | tcp |

---

### 3.4 core/camera/camera_manager.py — 摄像头管理

**文件位置：** `ai_service/core/camera/camera_manager.py`

`CameraManager` 管理所有摄像头，相当于一个"摄像头池"。

**主要功能：**

- 程序启动时，从sqlite数据库，为每个摄像头创建 `VideoDecoder`
- 提供 `get_frame(camera_id)` 接口，主循环通过它获取帧
- HTTP API 添加/删除摄像头时，实时调用 `add_camera()` / `remove_camera()`

**内部数据结构：**

```python
# 两个字典，key 都是 camera_id 字符串
self._decoders: dict[str, VideoDecoder]       # 解码器实例
self._camera_configs: dict[str, CameraConfig] # 摄像头配置
```

---

### 3.5 core/inference/ — AI 推理

#### 3.5.1 inference_manager.py — 单模型推理

**文件位置：** `ai_service/core/inference/inference_manager.py`

定义了推理的抽象接口和 YOLO 具体实现。

```python
class Detector(ABC):  # 抽象基类，类似 C++ 纯虚类
    def detect(self, frame) -> list[Detection]:
        raise NotImplementedError

class YOLODetector(Detector):  # YOLO 实现
    def __init__(self, model_path, confidence_threshold, device, ...):
        self.model = YOLO(model_path)  # 加载模型

    def detect(self, frame) -> list[Detection]:
        results = self.model(frame)    # 执行推理
        return self._parse_results(results)  # 解析结果
```

#### 3.5.2 multi_model_inference.py — 多模型并行推理

**文件位置：** `ai_service/core/inference/multi_model_inference.py`

这是推理模块的核心，管理所有检测模型。

**加载的模型（按 config.yaml 配置）：**

| 模型键名 | 模型文件 | 检测目标 |
|---------|---------|---------|
| fire_smoke | fire_smoke_yolov10.pt | 明火、明烟 |
| smoking | smoking_yolov11.pt | 人员吸烟 |
| phone | phone_yolov8.pt | 使用手机 |
| ppe | ppe_yolov8.pt | 安全帽、救生衣、工作服 |
| person | yolov8n.pt | 人员（用于离岗检测） |

**并行推理实现：**

```python
# 伪代码说明
def detect_all(frame) -> list[Detection]:
    with ThreadPoolExecutor() as pool:
        # 同时提交 5 个推理任务
        futures = [pool.submit(detector.detect, frame)
                   for detector in self.detectors]
        # 等待所有结果
        all_detections = []
        for future in futures:
            all_detections.extend(future.result())
    return all_detections
```

> **ThreadPoolExecutor 说明：** Python 中 CPU 密集型任务（如神经网络推理）可以用线程池让多个任务"同时"进行，充分利用多核 CPU。

**Label 映射机制：**

不同模型输出的类别名可能不同，需要统一。例如 PPE 模型可能输出 `"no-helmet"`，系统需要的是 `"no_helmet"`。配置文件中可以设置映射规则。

---

### 3.6 core/tracker/byte_tracker.py — 目标跟踪

**文件位置：** `ai_service/core/tracker/byte_tracker.py`

实现 ByteTrack 算法，核心思想是用两套匹配策略：
1. 先用高置信度检测框匹配现有轨迹
2. 再用低置信度检测框拯救可能丢失的轨迹

**优先使用 `boxmot` 库：**

```python
try:
    from boxmot import ByteTracker   # 第三方实现，功能更完整
except ImportError:
    # 回退到内置的简单 IOU 匹配实现
```

**核心接口：**

```python
def update(detections: list[Detection], frame_shape) -> list[Detection]:
    # 输入：当前帧的检测结果
    # 输出：带有 track_id 的检测结果
    # 内部：将检测框与历史轨迹匹配，分配持久 ID
```

---

### 3.7 core/event_engine/event_engine.py — 事件引擎

**文件位置：** `ai_service/core/event_engine/event_engine.py`

`EventEngine` 是防误报的核心机制，采用"连续帧计数"策略。

**内部状态存储：**

```python
# 字典，key = (camera_id, track_id, alarm_type)，value = 计数器
_frame_counts: dict[tuple, int] = {}

# 示例：
# ("cam1", 3, "smoking") → 12   表示 ID=3 的人已连续12帧被检测到吸烟
# ("cam1", 5, "no_helmet") → 3  表示 ID=5 的人已连续3帧未戴安全帽
```

**触发逻辑：**

```python
def process(camera_id, detections) -> list[Detection]:
    alarm_detections = []
    for det in detections:
        key = (camera_id, det.track_id, det.alarm_type)
        _frame_counts[key] += 1  # 计数加1
        threshold = get_threshold(det.alarm_type)  # 从配置读阈值
        if _frame_counts[key] >= threshold:
            alarm_detections.append(det)  # 达到阈值，加入报警列表
    # 清除本帧没有出现的 key（目标消失）
    return alarm_detections
```

**各类报警的连续帧阈值（25fps 下）：**

| 报警类型 | 连续帧数 | 对应时长 |
|---------|---------|---------|
| fire（明火）| 5 | 0.2 秒 |
| smoke（明烟）| 5 | 0.2 秒 |
| smoking（吸烟）| 15 | 0.6 秒 |
| phone_use（手机）| 15 | 0.6 秒 |
| no_helmet（无安全帽）| 75 | 3 秒 |
| no_lifejacket（无救生衣）| 75 | 3 秒 |
| no_workwear（无工作服）| 75 | 3 秒 |
| absence（离岗）| 150 | 6 秒 |

---

### 3.8 core/alarm/alarm_manager.py — 报警管理

**文件位置：** `ai_service/core/alarm/alarm_manager.py`

`AlarmManager` 负责生成最终的 `AlarmEvent` 并去重。

**冷却机制：**

```python
# 字典记录每个目标的最近报警时间
# key = (camera_id, track_id, alarm_type)，value = 上次报警的时间戳
_cooldown_map: dict[tuple, float] = {}

def generate_alarm(camera_id, det) -> AlarmEvent | None:
    key = (camera_id, det.track_id, det.alarm_type)
    now = time.time()
    last_alarm_time = _cooldown_map.get(key, 0)

    if now - last_alarm_time < cooldown_seconds:  # 30秒内
        return None  # 冷却中，不重复报警

    _cooldown_map[key] = now  # 记录本次报警时间
    return AlarmEvent(...)    # 生成报警事件
```

**报警缓存：**

使用 `collections.deque(maxlen=1000)`（双端队列），最多保留 1000 条报警记录，超出后自动丢弃最旧的。

---

### 3.9 core/tcp/tcp_server.py — TCP 服务器

**文件位置：** `ai_service/core/tcp/tcp_server.py`

基于 Python `asyncio` 的异步 TCP 服务器，向所有连接的客户端广播数据。

**协议设计：**

- 每条消息是一个 JSON 对象，末尾加 `\n`（换行符作为分隔符）
- 客户端按换行符切割消息，避免粘包问题

**消息推送时机：**

| 触发条件 | 消息类型 |
|---------|---------|
| 每帧有检测结果 | `detection` |
| 报警触发 | `alarm` |
| 摄像头状态变化 | `camera_status` |
| 每 5 秒 | `heartbeat` |

**内部客户端管理：**

```python
_clients: set[asyncio.StreamWriter] = set()

async def handle_client(reader, writer):
    _clients.add(writer)      # 客户端连接时加入集合
    try:
        await reader.read()   # 等待客户端断开
    finally:
        _clients.discard(writer)  # 断开时从集合移除

async def broadcast(message: dict):
    data = json.dumps(message) + "\n"
    for writer in _clients:
        writer.write(data.encode())  # 向所有客户端发送
```

---

### 3.10 core/api/ — HTTP API

**文件位置：** `ai_service/core/api/`

使用 FastAPI 框架提供 RESTful API，供 Qt 客户端管理摄像头配置。

#### 接口列表

| 方法 | 路径 | 功能 |
|------|------|------|
| POST | `/api/v1/ipc` | 添加摄像头 |
| GET | `/api/v1/ipc` | 获取所有摄像头 |
| GET | `/api/v1/ipc/{id}` | 获取单个摄像头 |
| PUT | `/api/v1/ipc/{id}` | 更新摄像头配置 |
| DELETE | `/api/v1/ipc/{id}` | 删除摄像头 |
| PUT | `/api/v1/ipc/{id}/position` | 更新摄像头位置 |
| GET | `/api/v1/health` | 健康检查 |

#### 统一响应格式

```json
{
    "code": 0,
    "message": "success",
    "data": { ... }
}
```

`code` 为 0 表示成功，非 0 表示错误。

#### 添加摄像头请求示例

```json
POST /api/v1/ipc
{
    "id": "cam_001",
    "name": "甲板摄像头1",
    "rtsp_url": "rtsp://admin:admin123@192.168.1.101:554/stream1",
    "enabled": true
}
```

---

### 3.11 core/database/ — 数据库层

**文件位置：** `ai_service/core/database/`

使用 SQLite + SQLAlchemy（异步模式）持久化摄像头配置。

**数据库表结构（ipc_info 表）：**

| 字段 | 类型 | 说明 |
|------|------|------|
| id | String | 主键，摄像头唯一ID |
| name | String | 摄像头名称 |
| rtsp_url | String | RTSP 地址 |
| username | String | 账号（可选）|
| password | String | 密码（加密存储）|
| enabled | Boolean | 是否启用 |
| position | String | 安装位置描述 |
| created_at | DateTime | 创建时间 |
| updated_at | DateTime | 更新时间 |

**密码加密：** 使用 `cryptography.fernet` 对称加密，密钥从配置文件读取（或自动生成）。数据库中存储的是加密后的密文。

---

## 4. 配置文件详解

**文件位置：** `ai_service/config/config.yaml`

### 4.1 数据库配置

```yaml
database:
  enabled: true      # true=从SQLite加载摄像头；false=只从yaml加载
  path: "data/cameras.db"
```

### 4.2 API 和 TCP 端口

```yaml
api:
  host: "0.0.0.0"   # 监听所有网卡
  port: 8889         # HTTP API 端口

tcp:
  host: "0.0.0.0"
  port: 8888         # TCP 推送端口
  heartbeat_interval: 5  # 心跳间隔（秒）
```

### 4.3 摄像头配置（database.enabled=false 时使用）

```yaml
cameras:
  - id: "1"
    name: "甲板摄像头1"
    rtsp_url: "rtsp://127.0.0.1:8554/video1"
    enabled: true
```

### 4.4 解码器配置

```yaml
decoder:
  buffer_size: 10            # 帧缓冲大小
  reconnect_interval: 5      # 断线重连间隔（秒）
  max_reconnect_attempts: -1 # -1 表示无限重试
  rtsp_transport: "tcp"      # tcp 比 udp 更稳定
```

### 4.5 推理配置

```yaml
inference:
  device: "cpu"         # "cpu" 或 "cuda"（有 NVIDIA GPU 时用 cuda 更快）
  iou_threshold: 0.45   # 非极大值抑制阈值（去掉重叠框）
  max_det: 300          # 每帧最多检测多少个目标
  imgsz: 640            # 输入图像尺寸（像素）
  half: true            # FP16 半精度（仅 CUDA 有效，加速推理）

  models:
    - name: "fire_smoke"
      enabled: true
      model_path: "models/fire_smoke_yolov10.pt"
      confidence_threshold: 0.45  # 低于此置信度的检测框丢弃
    - name: "smoking"
      enabled: true
      model_path: "models/smoking_yolov11.pt"
      confidence_threshold: 0.5
    # ...（其他模型类似）
```

### 4.6 报警规则配置

```yaml
event_engine:
  alarm_rules:
    - alarm_type: "smoking"
      label: "smoking"         # 触发该规则的检测标签
      consecutive_frames: 15   # 需要连续多少帧（25fps下=0.6秒）
    - alarm_type: "fire"
      label: "fire"
      consecutive_frames: 5
    # ...
```

### 4.7 报警管理配置

```yaml
alarm_manager:
  max_alarm_cache: 1000   # 最多缓存多少条报警
  alarm_cooldown: 30      # 同一目标的报警冷却时间（秒）
```

---

## 5. TCP 通信协议

Qt 客户端与 AI Service 通过 TCP 长连接通信，端口 8888。

### 消息格式

每条消息是一行 JSON，以 `\n` 结尾：

```
{"type":"heartbeat","timestamp":1720598400}\n
{"type":"detection","camera_id":"1","detections":[...]}\n
```

### 消息类型详解

#### detection — 检测结果

每帧有检测目标时发送：

```json
{
    "type": "detection",
    "camera_id": "1",
    "timestamp": 1720598400.123,
    "detections": [
        {
            "track_id": 3,
            "label": "smoking",
            "confidence": 0.92,
            "bbox": [0.5, 0.3, 0.2, 0.4],
            "alarm_type": "smoking"
        }
    ]
}
```

`bbox` 格式：`[中心x, 中心y, 宽, 高]`，均为 0~1 的归一化值。

#### alarm — 报警事件

触发报警时发送：

```json
{
    "type": "alarm",
    "id": "550e8400-e29b-41d4-a716-446655440000",
    "camera_id": "1",
    "camera_name": "甲板摄像头1",
    "alarm_type": "smoking",
    "description": "检测到人员吸烟 (置信度: 92.00%)",
    "timestamp": 1720598400.123,
    "track_id": 3,
    "bbox": [0.5, 0.3, 0.2, 0.4],
    "confidence": 0.92
}
```

#### camera_status — 摄像头状态

摄像头上线/下线时发送：

```json
{
    "type": "camera_status",
    "camera_id": "1",
    "online": true,
    "fps": 25.3
}
```

#### heartbeat — 心跳

每 5 秒发送，用于检测连接是否存活：

```json
{
    "type": "heartbeat",
    "timestamp": 1720598400.123
}
```

---

## 6. HTTP API 接口

HTTP API 运行在 8889 端口，供 Qt 客户端管理摄像头。

### 统一响应格式

```json
{
    "code": 0,          // 0=成功，其他=错误
    "message": "success",
    "data": { ... }     // 响应数据，可能是对象或数组
}
```

### 摄像头管理接口

#### 添加摄像头

```
POST /api/v1/ipc
Content-Type: application/json

{
    "id": "cam_001",
    "name": "甲板前部摄像头",
    "rtsp_url": "rtsp://192.168.1.101:554/stream1",
    "enabled": true,
    "position": "甲板前部"
}
```

#### 获取摄像头列表

```
GET /api/v1/ipc

响应：
{
    "code": 0,
    "message": "success",
    "data": [
        { "id": "cam_001", "name": "...", "rtsp_url": "...", "online": true }
    ]
}
```

#### 更新摄像头

```
PUT /api/v1/ipc/cam_001
{ "name": "新名称", "enabled": false }
```

#### 删除摄像头

```
DELETE /api/v1/ipc/cam_001
```

#### 健康检查

```
GET /api/v1/health

响应：
{
    "status": "ok",
    "cameras_loaded": 3,
    "cameras_online": 2
}
```

---

## 7. 关键 Python 概念说明

本节解释代码中大量使用的 Python 特性，帮助理解代码结构。

### 7.1 async/await — 异步编程

Python 中的 `async def` 定义一个"协程"（coroutine），相当于一个可以暂停执行的函数。`await` 表示"等待某个操作完成，期间可以去做别的事"。

```python
# 同步：一个一个等
def sync_read():
    data1 = read_camera1()  # 等 100ms
    data2 = read_camera2()  # 再等 100ms
    # 总耗时 200ms

# 异步：同时等
async def async_read():
    data1, data2 = await asyncio.gather(
        read_camera1(),   # 同时发起
        read_camera2(),   # 同时发起
    )
    # 总耗时约 100ms（取决于最慢的那个）
```

AI Service 的 TCP 服务器、数据库操作、HTTP API 都使用异步模式。

### 7.2 dataclass — 数据类

`@dataclass` 是一个"装饰器"，自动为类生成 `__init__`、`__repr__` 等方法，用来定义纯数据结构。

```python
@dataclass
class Point:
    x: float
    y: float

# 相当于手写了：
# def __init__(self, x: float, y: float):
#     self.x = x
#     self.y = y
```

### 7.3 Type Hints — 类型注解

Python 是动态类型语言，但代码中大量使用类型注解提高可读性：

```python
def detect(self, frame: np.ndarray) -> list[Detection]:
    #  参数类型 ↑                    返回类型 ↑
```

这只是"注释"，不强制执行，但 IDE 可以根据它做代码补全和检查。

### 7.4 dict / list / deque — 常用容器

- `dict`：字典（映射），类似 C++ `std::map`，如 `{"key": value}`
- `list`：列表（可变数组），如 `[1, 2, 3]`
- `collections.deque(maxlen=N)`：双端队列，当长度超过 N 时自动丢弃最旧的元素，用于实现固定大小的缓冲区

### 7.5 Abstract Base Class — 抽象基类

```python
from abc import ABC, abstractmethod

class Detector(ABC):  # 抽象基类，不能直接实例化
    @abstractmethod
    def detect(self, frame) -> list[Detection]:
        pass  # 子类必须实现这个方法
```

类似 C++ 的纯虚函数，用于定义接口规范。

### 7.6 with 语句 — 上下文管理器

```python
with ThreadPoolExecutor() as pool:
    futures = [pool.submit(task) for task in tasks]
# with 块结束时自动释放线程池资源，即使出了异常也会执行
```

用于确保资源（文件、连接、锁等）在使用完后被正确释放。

### 7.7 threading 与 asyncio 的混用

AI Service 中存在两种并发机制：

| 机制 | 用途 | 原因 |
|------|------|------|
| `asyncio` | TCP服务器、HTTP API、数据库 | 这些是 I/O 密集型，用异步更高效 |
| `threading.Thread` | RTSP 视频解码 | OpenCV 的 API 是同步的，必须放在线程里 |
| `ThreadPoolExecutor` | 多模型并行推理 | AI 推理是 CPU 密集型，用线程池提升吞吐量 |

---

*文档生成时间：2026-07-30*

