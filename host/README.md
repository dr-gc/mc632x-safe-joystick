# PC 上位机

双摄拉流、手部跟踪、手势分类、摇杆映射、安全状态机与 Web 服务。

## 依赖

```bash
pip install mediapipe flask flask-sock pyserial pillow ai-edge-litert
```

需要系统已安装 ffmpeg。

## 文件说明

| 文件 | 职责 |
|---|---|
| web_host.py | 主程序：双摄拉流、MediaPipe 推理、Flask 服务、串口读取 |
| joystick.py | 八方向映射、死区、平滑、零点校准、四态状态机 |
| gesture_mlp.py | MLP 手势分类（加载 models/ 下的 tflite）|
| car_link.py | 摇杆状态转为车辆指令，经 Unix socket 送出 |
| tcp_ctrl_hb.py | TCP 服务端，循环 accept，0.5 秒心跳重发 |
| vehicle_udp.py | UDP 广播备用通道，20 Hz JSON |
| carctl.py | 交互式调试控制台 |
| car.sh | 单条命令快捷脚本 |
| models/ | 手势 MLP 模型与标签 |

## 启动

```bash
# 终端 1：TCP 服务端
python3 tcp_ctrl_hb.py

# 终端 2：主程序
python3 web_host.py

# 浏览器打开
firefox http://localhost:8765
```

## 配置

板端地址在 web_host.py 开头：

```python
RTSP  = "rtsp://192.168.215.201:1246"    # 人体识别视角
RTSP2 = "rtsp://192.168.215.201:1252"    # 手势识别视角
```

串口自动探测 `/dev/ttyUSB*`，无需手改。

## HTTP 接口

| 路径 | 说明 |
|---|---|
| GET / | Web 界面 |
| GET /video | 人体识别画面 MJPEG |
| GET /video1 | 手势识别画面 MJPEG |
| GET /api/state | 全量状态（摇杆、手势、板端、帧率）|
| GET /api/carlink | 车辆控制通道状态 |
| POST /api/carlink/enable/0\|1 | 暂停或恢复控制 |
| POST /api/calibrate/start/<target> | 开始零点校准 |
| POST /api/calibrate/reset/<target> | 重置零点 |

## 可调参数

| 参数 | 位置 | 默认 | 说明 |
|---|---|---|---|
| dead_zone | joystick.py | 0.08 | 死区半径，占工作区半宽比例 |
| smoothing | joystick.py | 0.4 | 输出低通滤波系数 |
| MIN_SPEED | car_link.py | 60 | 最小有效幅度（0~1000）|
| HAND_W / HAND_H | web_host.py | 480, 270 | 手部检测输入分辨率 |
| min_hand_detection_confidence | web_host.py | 0.15 | 检出阈值，再低会误检 |
| HEARTBEAT_SEC | tcp_ctrl_hb.py | 0.5 | 心跳周期（秒）|

## 调试

```bash
# 查摇杆状态
curl -s http://localhost:8765/api/state | python3 -m json.tool

# 查控制通道
curl -s http://localhost:8765/api/carlink | python3 -m json.tool

# 手动发指令
./car.sh w
./car.sh status
```

## 常见问题

| 现象 | 原因 | 解决 |
|---|---|---|
| 一直显示待机 | 手不在手势摄像头画面内 | 看左画面，把手伸进去 |
| 摇杆乱跳 | 手抖或光照变化 | 调大 dead_zone |
| 骨架不显示 | Canvas 绘制异常 | 检查浏览器控制台 window.__ov1Err |
| UART OFFLINE | 串口设备名变化 | 已自动探测，重启本程序 |
| 小车不动 | TCP 未连 | ss -tn 查看 9000 端口连接 |
| 视频卡顿 | 板端 ISP 6.55 fps 限制 | 见根目录 README 已知限制 |
