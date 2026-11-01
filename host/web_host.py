#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
安全虚拟摇杆 上位机 (Web 版)
============================
架构：
  Flask (后端)  ── HTTP/WebSocket ──  浏览器 (前端)
    ├─ RTSP 拉流解码
    ├─ MediaPipe 手部检测
    ├─ 摇杆映射
    └─ 串口板端数据

启动: python3 web_host.py
访问: http://localhost:8765
"""
import json
import os
import queue
import re
import subprocess
import threading
import time

from flask import Flask, Response, jsonify, render_template_string
from flask_sock import Sock

HERE = os.path.dirname(os.path.abspath(__file__))
import sys
sys.path.insert(0, HERE)

from joystick import DualJoystick
from gesture_mlp import classify_mlp, get_classifier
from vehicle_udp import get_broadcaster, VehicleBroadcaster
from car_link import get_carlink

RTSP = "rtsp://192.168.215.201:1246"      # CAM-1 -> 人体识别（板端 NPU）
RTSP2 = "rtsp://192.168.215.201:1252"     # CAM-2 -> 手势识别（PC MediaPipe）
def _find_uart():
    """自动探测 CH340 串口（编号会随 USB 口变化，不能写死）"""
    import glob
    cands = sorted(glob.glob("/dev/ttyUSB*"))
    if cands:
        return cands[0]
    return "/dev/ttyUSB0"


UART = _find_uart()
BAUD = 115200
STREAM_W, STREAM_H = 960, 540       # 推流分辨率
HAND_W, HAND_H = 480, 270          # 手部检测分辨率（MediaPipe 最佳区间）

app = Flask(__name__)
sock = Sock(app)

# ============ 全局状态 ============
class G:
    # ── 视频：直接存 ffmpeg 输出的 MJPEG 帧（不做二次编码）
    mjpeg_q0 = None           # queue.Queue，CAM-0 的 JPEG 帧
    mjpeg_q1 = None           # queue.Queue，CAM-1 的 JPEG 帧
    frame_jpg = None          # CAM-0 最新 JPEG（给浏览器兜底/单帧）
    frame_jpg1 = None
    frame_lock = threading.Lock()
    cam1_ok = False
    # ── 手部检测用的低分辨率 RGB 帧
    hand_frame = None
    hand_frame_lock = threading.Lock()
    hand_seq = 0              # 递增序号，避免重复处理
    joy = None                # JoystickState
    hands = []
    gesture = "NONE"
    board = {"find": 0, "num": -1, "conf": 0.0, "fps": 0.0,
             "bb": None, "connected": False, "state": "LOCK"}
    fps_stat = {"capture": 0.0, "hand": 0.0}
    handedness = []           # 每只手的左右标注


mapper = DualJoystick()          # 左半屏 / 右半屏 各一个摇杆
G.joy = mapper.right.state       # 兼容旧引用


# ============ 板端串口 ============
ROD_RE = re.compile(
    r"\[ROD\]\s+(UNLOCK|LOCK)\s+x=(-?\d+)\s+y=(-?\d+)\s+xs=(-?\d+)\s+ys=(-?\d+)"
    r"\s+find=(\d+)\s+rz=(\w+)(?:\s+bb=(-?\d+),(-?\d+),(-?\d+),(-?\d+))?")
BOX_RE = re.compile(r"\[BOX\]\s+num=(\d+)(?:\s+maxconf=([\d.]+))?")
FPS_RE = re.compile(r"Det FPS:\s*([\d.]+)")


def uart_reader():
    try:
        import serial
    except ImportError:
        print("[UART] pyserial 缺失")
        return
    while True:
        try:
            ser = serial.Serial(UART, BAUD, timeout=0.2)
            print(f"[UART] 已连接 {UART}")
            break
        except Exception as e:
            print(f"[UART] 重试: {e}")
            time.sleep(3)

    buf = ""
    last = 0.0
    while True:
        try:
            data = ser.read(8192)
            if not data:
                if time.time() - last > 3:
                    G.board["connected"] = False
                continue
            buf += data.decode("utf-8", "replace")
            if len(buf) > 65536:
                buf = buf[-8192:]

            while "\n" in buf:
                line, buf = buf.split("\n", 1)
                line = line.strip()
                if not line:
                    continue
                m = ROD_RE.search(line)
                if m:
                    G.board["state"] = m.group(1)
                    G.board["find"] = int(m.group(6))
                    if m.group(8):
                        G.board["bb"] = (int(m.group(8)), int(m.group(9)),
                                         int(m.group(10)), int(m.group(11)))
                    G.board["connected"] = True
                    last = time.time()
                    continue
                m = BOX_RE.search(line)
                if m:
                    G.board["num"] = int(m.group(1))
                    G.board["conf"] = float(m.group(2)) if m.group(2) else 0.0
                    last = time.time()
                    continue
                m = FPS_RE.search(line)
                if m:
                    G.board["fps"] = float(m.group(1))
                    last = time.time()
        except Exception:
            time.sleep(1)


# ============ 视频捕获 ============
def _split_mjpeg(buf):
    """从字节流里切出完整 JPEG 帧（SOI=FFD8, EOI=FFD9）"""
    frames = []
    while True:
        i = buf.find(b"\xff\xd8")
        if i < 0:
            return frames, buf[-1:]
        j = buf.find(b"\xff\xd9", i + 2)
        if j < 0:
            return frames, buf[i:]
        frames.append(buf[i:j + 2])
        buf = buf[j + 2:]


def video_loop(url=None, cam_idx=0):
    """ffmpeg 直接输出 MJPEG，Python 只做字节切帧（零解码零编码）"""
    url = url or RTSP
    cmd = ["ffmpeg", "-loglevel", "error",
           "-rtsp_transport", "tcp",
           "-flags", "low_delay",
           "-fflags", "nobuffer",
           "-probesize", "32", "-analyzeduration", "0",
           "-i", url,
           "-vf", f"scale={STREAM_W}:{STREAM_H}",
           "-f", "mjpeg", "-q:v", "8",
           "-"]
    q = G.mjpeg_q0 if cam_idx == 0 else G.mjpeg_q1

    while True:
        try:
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL, bufsize=0)
        except Exception as e:
            print(f"[VIDEO{cam_idx}] 启动失败: {e}")
            time.sleep(3)
            continue

        buf = b""
        t0 = time.time()
        cnt = 0
        while True:
            chunk = p.stdout.read(131072)
            if not chunk:
                break
            buf += chunk
            frames, buf = _split_mjpeg(buf)

            for jpg in frames:
                # 覆盖式入队（丢旧帧保实时）
                if q.full():
                    try:
                        q.get_nowait()
                    except queue.Empty:
                        pass
                try:
                    q.put_nowait(jpg)
                except queue.Full:
                    pass

                with G.frame_lock:
                    if cam_idx == 0:
                        G.frame_jpg = jpg
                    else:
                        G.frame_jpg1 = jpg
                        G.cam1_ok = True
                cnt += 1

            if cnt and cnt % 30 == 0:
                el = time.time() - t0
                if el > 0.1:
                    if cam_idx == 0:
                        G.fps_stat["capture"] = cnt / el

        p.kill()
        if cam_idx == 1:
            G.cam1_ok = False
        time.sleep(1.5)


def hand_frame_loop(url=None):
    """独立的低分辨率 rawvideo 流，专门喂给手部检测（不占视频链路）"""
    url = url or RTSP
    W, H = HAND_W, HAND_H
    need = W * H * 3
    cmd = ["ffmpeg", "-loglevel", "error",
           "-rtsp_transport", "tcp",
           "-flags", "low_delay",
           "-fflags", "nobuffer",
           "-probesize", "32", "-analyzeduration", "0",
           "-i", url,
           "-vf", f"scale={W}:{H}",
           "-pix_fmt", "rgb24", "-f", "rawvideo", "-"]

    while True:
        try:
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL, bufsize=0)
        except Exception as e:
            print(f"[HAND-FEED] 启动失败: {e}")
            time.sleep(3)
            continue

        buf = b""
        while True:
            chunk = p.stdout.read(65536)
            if not chunk:
                break
            buf += chunk
            while len(buf) >= need:
                frame, buf = buf[:need], buf[need:]
                with G.hand_frame_lock:
                    G.hand_frame = frame
                    G.hand_seq += 1
        p.kill()
        time.sleep(1.5)


# ============ 手部检测 ============
LM_CONN = [(0, 1), (1, 2), (2, 3), (3, 4), (0, 5), (5, 6), (6, 7), (7, 8),
           (5, 9), (9, 10), (10, 11), (11, 12), (9, 13), (13, 14), (14, 15),
           (15, 16), (13, 17), (17, 18), (18, 19), (19, 20), (0, 17)]


def _ext(lms, tip, pip):
    """手指伸直：指尖明显比 PIP 更远离手腕（阈值收紧）"""
    wx, wy = lms[0][0], lms[0][1]
    tx, ty = lms[tip][0], lms[tip][1]
    px, py = lms[pip][0], lms[pip][1]
    dt = (tx - wx) ** 2 + (ty - wy) ** 2
    dp = (px - wx) ** 2 + (py - wy) ** 2
    return dt > dp * 1.38          # 1.15 → 1.38，更严格


def _curled(lms, tip, pip):
    """手指蜷曲：指尖比 PIP 更靠近手腕"""
    wx, wy = lms[0][0], lms[0][1]
    tx, ty = lms[tip][0], lms[tip][1]
    px, py = lms[pip][0], lms[pip][1]
    dt = (tx - wx) ** 2 + (ty - wy) ** 2
    dp = (px - wx) ** 2 + (py - wy) ** 2
    return dp > 1e-9 and dt < dp * 0.78


def _d(a, b):
    """两点距离"""
    return ((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2) ** 0.5


def _angle(a, b, c):
    """b 处夹角（度）。三点连线越直越接近 180°"""
    import math
    v1x, v1y = a[0] - b[0], a[1] - b[1]
    v2x, v2y = c[0] - b[0], c[1] - b[1]
    n1 = math.hypot(v1x, v1y)
    n2 = math.hypot(v2x, v2y)
    if n1 < 1e-9 or n2 < 1e-9:
        return 180.0
    cosv = (v1x * v2x + v1y * v2y) / (n1 * n2)
    cosv = max(-1.0, min(1.0, cosv))
    return math.degrees(math.acos(cosv))


# 每根手指的 (MCP, PIP, DIP, TIP) 索引
_FINGER_CHAINS = {
    "index":  (5, 6, 7, 8),
    "middle": (9, 10, 11, 12),
    "ring":   (13, 14, 15, 16),
    "pinky":  (17, 18, 19, 20),
}


def _finger_curl(lms, chain):
    """返回 (PIP角, DIP角)。握拳时两角都很小（接近 0°），伸开时接近 180°"""
    mcp_i, pip_i, dip_i, tip_i = chain
    a_pip = _angle(lms[mcp_i], lms[pip_i], lms[dip_i])
    a_dip = _angle(lms[pip_i], lms[dip_i], lms[tip_i])
    return a_pip, a_dip


def _is_fist(lms):
    """严格握拳判定（双重判据，任一不满足即不算握拳）

    判据 A —— 指尖内收（相对腕部）
        伸直时：TIP 比 PIP 离腕部更远  (d(TIP,wrist) > d(PIP,wrist))
        握拳时：TIP 缩回到 PIP 内侧    (d(TIP,wrist) < d(PIP,wrist) * 0.85)
        该判据与手掌朝向无关，不受角度投影误差影响。

    判据 B —— 指尖贴近掌心
        d(TIP, palm9) / 掌宽 < 0.95

    四指必须同时满足 A 和 B，才记为 curled。要求 4 指全部 curled。
    拇指另判：拇指尖到掌心 < 1.25 掌宽。
    """
    wrist = lms[0]
    palm = lms[9]
    palm_w = _d(lms[5], lms[17])
    if palm_w < 1e-6:
        return False

    chains = [
        (5, 6, 8),    # 食指 MCP, PIP, TIP
        (9, 10, 12),  # 中指
        (13, 14, 16), # 无名指
        (17, 18, 20), # 小指
    ]

    curled = 0
    for mcp_i, pip_i, tip_i in chains:
        d_tip_w = _d(lms[tip_i], wrist)
        d_pip_w = _d(lms[pip_i], wrist)
        # 判据 A：指尖缩回 PIP 内侧
        cond_a = d_tip_w < d_pip_w * 0.90
        # 判据 B：指尖贴近掌心
        cond_b = (_d(lms[tip_i], palm) / palm_w) < 0.95
        if cond_a and cond_b:
            curled += 1

    if curled < 4:
        return False

    # 拇指收拢
    thumb_ok = _d(lms[4], palm) / palm_w < 1.25
    return thumb_ok


def classify(lms):
    i, m, r, p = (_ext(lms, 8, 6), _ext(lms, 12, 10),
                  _ext(lms, 16, 14), _ext(lms, 20, 18))

    tx, ty = lms[4][0], lms[4][1]
    ix, iy = lms[8][0], lms[8][1]
    d = ((tx - ix) ** 2 + (ty - iy) ** 2) ** 0.5
    hs = ((lms[0][0] - lms[9][0]) ** 2 + (lms[0][1] - lms[9][1]) ** 2) ** 0.5

    # ── 握拳：严格关节角度判据（半张手不应误判）
    if _is_fist(lms):
        return "FIST"

    if hs > 0 and d / hs < 0.45 and m and r and p:
        return "OK"
    if i and m and r and p:
        return "OPEN_PALM"
    if i and not (m or r or p):
        return "ONE"
    if i and m and not (r or p):
        return "TWO"
    if i and m and r and not p:
        return "THREE"
    if i and not (m or r) and p:
        return "HORNS"

    cnt = sum([i, m, r, p])
    return f"UNK_{cnt}"


def hand_loop():
    try:
        import mediapipe as mp
        from mediapipe.tasks import python as mp_python
        from mediapipe.tasks.python import vision
        import numpy as np
        from PIL import Image
        import io
    except Exception as e:
        print(f"[HAND] mediapipe 不可用: {e}")
        return

    model = os.path.expanduser("~/hand_self/models/hand_landmarker.task")
    if not os.path.exists(model):
        print(f"[HAND] 模型缺失 {model}")
        return

    base = mp_python.BaseOptions(model_asset_path=model)
    opts = vision.HandLandmarkerOptions(
        base_options=base, running_mode=vision.RunningMode.VIDEO,
        num_hands=2,
        min_hand_detection_confidence=0.15,   # 检出放宽（原来 0.4）
        min_hand_presence_confidence=0.35,    # 保留（防止背景误检）
        min_tracking_confidence=0.30)
    lm = vision.HandLandmarker.create_from_options(opts)
    print("[HAND] MediaPipe 就绪 (双摇杆模式)")

    last_seq = -1
    t0 = time.time()
    cnt = 0

    while True:
        with G.hand_frame_lock:
            frame = G.hand_frame
            seq = G.hand_seq

        if frame is None or seq == last_seq:
            time.sleep(0.001)
            continue
        last_seq = seq

        try:
            arr = np.frombuffer(frame, dtype=np.uint8).reshape(
                (HAND_H, HAND_W, 3))
            mp_img = mp.Image(image_format=mp.ImageFormat.SRGB, data=arr)
            res = lm.detect_for_video(mp_img, int(time.time() * 1000))
        except Exception as e:
            print(f"[HAND] 检测异常: {e}")
            time.sleep(0.05)
            continue

        hands = []
        handed = []
        hand_conf = []
        if res.hand_landmarks:
            for idx, h in enumerate(res.hand_landmarks):
                hands.append([(round(p.x, 5), round(p.y, 5)) for p in h])
                name = "Unknown"
                conf = 0.0
                if idx < len(res.handedness) and res.handedness[idx]:
                    cat = res.handedness[idx][0]
                    name = cat.category_name
                    conf = float(getattr(cat, "score", 0.0) or 0.0)
                handed.append(name)
                hand_conf.append(conf)

        G.hands = hands
        G.handedness = handed

        gestures = []
        hand_objs = []
        for i, pts in enumerate(hands):
            lms3 = [(x, y, 0) for x, y in pts]
            g = classify_mlp([(x, y) for x, y, _ in lms3])
            if g is None:
                g = classify(lms3)          # MLP 不可用时回退到规则
            cf = hand_conf[i] if i < len(hand_conf) else 0.0
            # 低置信度不判握拳（避免误触发急停）
            if g == "FIST" and cf < 0.5:
                g = "UNK_LOWCONF"
            gestures.append(g)
            hn = handed[i] if i < len(handed) else "Unknown"
            hand_objs.append({"landmarks": lms3, "handedness": hn})

        if gestures:
            right_idx = next((i for i, n in enumerate(handed)
                              if n.lower() == "right"), 0)
            G.gesture = gestures[right_idx]
        else:
            G.gesture = "NONE"

        try:
            mapper.update(hand_objs, gestures)
            # 同步给 UDP 广播器（调试用）
            _bc = get_broadcaster()
            _bc.update(mapper.right.state)
            # 同步给 TCP 控制通道（真正驱动小车仿真）
            get_carlink().update(mapper.right.state)
        except Exception as e:
            print(f"[MAPPER] update error: {e}")

        cnt += 1
        if cnt % 10 == 0:
            el = time.time() - t0
            if el > 0.1:
                G.fps_stat["hand"] = cnt / el


# ============ 路由 ============
@app.route("/")
def index():
    with open(os.path.join(HERE, "web", "index.html"), encoding="utf-8") as f:
        return f.read()


def _mjpeg(cam_idx):
    """从 ffmpeg 的 MJPEG 帧队列直接转发 —— 零解码零编码"""
    q = G.mjpeg_q0 if cam_idx == 0 else G.mjpeg_q1

    def gen():
        while True:
            try:
                jpg = q.get(timeout=0.5)
            except queue.Empty:
                continue
            yield (b"--frame\r\nContent-Type: image/jpeg\r\n"
                   b"Content-Length: " + str(len(jpg)).encode() +
                   b"\r\n\r\n" + jpg + b"\r\n")
    return Response(gen(), mimetype="multipart/x-mixed-replace; boundary=frame")


@app.route("/video")
def video():
    return _mjpeg(0)


@app.route("/video1")
def video1():
    return _mjpeg(1)


@app.route("/api/calibrate/start/<which>", methods=["POST"])
def api_cal_start(which):
    mapper.start_calibration(which)
    return jsonify({"ok": True, "which": which, "state": mapper.as_dict()})


@app.route("/api/calibrate/reset/<which>", methods=["POST"])
def api_cal_reset(which):
    mapper.reset_calibration(which)
    return jsonify({"ok": True, "which": which, "state": mapper.as_dict()})


@app.route("/api/carlink")
def api_carlink():
    """摇杆 -> 小车控制通道状态"""
    return jsonify(get_carlink().snapshot())


@app.route("/api/carlink/enable/<int:on>", methods=["POST"])
def api_carlink_enable(on):
    lk = get_carlink()
    lk.enabled = bool(on)
    if not lk.enabled:
        lk._send("stop")
        lk.last_cmd = "stop"
    return jsonify({"ok": True, "enabled": lk.enabled})


@app.route("/api/vehicle")
def api_vehicle():
    """当前车辆控制指令（调试用；实际控制走 UDP 广播）"""
    return jsonify(get_broadcaster().snapshot())


@app.route("/api/vehicle/enable/<int:on>", methods=["POST"])
def api_vehicle_enable(on):
    bc = get_broadcaster()
    bc.enabled = bool(on)
    return jsonify({"ok": True, "enabled": bc.enabled})


@app.route("/api/state")
def api_state():
    return jsonify({
        "joysticks": mapper.as_dict(),
        "joystick": mapper.right.state.as_dict(),   # 兼容旧字段
        "gesture": G.gesture,
        "hands": G.hands,
        "handedness": getattr(G, "handedness", []),
        "board": G.board,
        "fps": G.fps_stat,
        "cam1_ok": G.cam1_ok,
    })


if __name__ == "__main__":
    threading.Thread(target=uart_reader, daemon=True).start()
    G.mjpeg_q0 = queue.Queue(maxsize=3)
    G.mjpeg_q1 = queue.Queue(maxsize=3)
    threading.Thread(target=video_loop, args=(RTSP, 0), daemon=True).start()
    threading.Thread(target=video_loop, args=(RTSP2, 1), daemon=True).start()
    threading.Thread(target=hand_frame_loop, args=(RTSP2,), daemon=True).start()

    # 车辆控制 UDP 广播
    get_broadcaster().start()
    threading.Thread(target=hand_loop, daemon=True).start()
    print("=" * 50)
    print("  安全虚拟摇杆 上位机 (Web)")
    print("  访问: http://localhost:8765")
    print("=" * 50)
    app.run(host="0.0.0.0", port=8765, threaded=True, debug=False)
