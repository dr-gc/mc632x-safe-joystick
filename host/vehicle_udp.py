#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
车辆控制指令 UDP 广播
====================
把摇杆状态编成固定 JSON 包，以 20Hz 广播到局域网。
Windows 小车仿真端只要 bind 这个端口就能收到，无需握手。

协议（JSON，UTF-8，单包 < 300 字节）：
{
  "v": 1,                  // 协议版本
  "seq": 12345,            // 递增序号，用于丢包/重复检测
  "ts": 1730451234.567,    // 发送时间（秒，本机单调时钟）
  "dir": "FWD",            // STOP/FWD/BACK/LEFT/RIGHT/FWD_LEFT/FWD_RIGHT/BACK_LEFT/BACK_RIGHT
  "x": 523,                // -1000..1000，右为正
  "y": 780,                // -1000..1000，前为正
  "speed": 78,             // 0..100 百分比（摇杆幅度）
  "estop": false,          // 急停（握拳）
  "active": true,          // 是否在控制中（手在工作区且超过死区）
  "hand": true             // 是否检测到手
}

Windows 端示例（Python）：
    import socket, json
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(("0.0.0.0", 9000))
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    while True:
        data, addr = s.recvfrom(2048)
        cmd = json.loads(data)
        drive(cmd["dir"], cmd["speed"], cmd["estop"])
"""
import json
import socket
import threading
import time

PORT = 9000          # 广播端口
RATE_HZ = 20         # 发送频率
PROTOCOL_VERSION = 1


class VehicleBroadcaster:
    """把摇杆状态广播成车辆控制指令"""

    def __init__(self, port=PORT, rate_hz=RATE_HZ, broadcast_addr="255.255.255.255"):
        self.port = port
        self.interval = 1.0 / max(rate_hz, 1)
        self.addr = broadcast_addr
        self.seq = 0
        self.t0 = time.monotonic()
        self._sock = None
        self._thread = None
        self._stop = threading.Event()
        self._lock = threading.Lock()
        self._last = {
            "dir": "STOP", "x": 0, "y": 0, "speed": 0,
            "estop": False, "active": False, "hand": False,
        }
        self.enabled = True

    # ---------- 生命周期 ----------
    def start(self):
        try:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        except Exception as e:
            print(f"[VEHICLE] UDP socket 创建失败: {e}")
            self._sock = None
            return False

        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()
        print(f"[VEHICLE] UDP 广播已启动 -> {self.addr}:{self.port} @ {int(1/self.interval)}Hz")
        return True

    def stop(self):
        self._stop.set()
        if self._sock:
            try:
                self._sock.close()
            except Exception:
                pass
            self._sock = None

    # ---------- 状态更新 ----------
    def update(self, joy_state):
        """joy_state: HandJoystick.state（JoystickState 实例）"""
        with self._lock:
            self._last = {
                "dir": joy_state.direction or "STOP",
                "x": int(joy_state.x),
                "y": int(joy_state.y),
                "speed": int(min(100, round(joy_state.magnitude / 10.0))),
                "estop": bool(joy_state.estop),
                "active": bool(joy_state.active),
                "hand": bool(joy_state.hand_present),
            }

    # ---------- 发包 ----------
    def _packet(self):
        with self._lock:
            body = dict(self._last)
        self.seq += 1
        body["v"] = PROTOCOL_VERSION
        body["seq"] = self.seq
        body["ts"] = round(time.monotonic() - self.t0, 3)
        return json.dumps(body, separators=(",", ":")).encode("utf-8")

    def _loop(self):
        while not self._stop.is_set():
            if self.enabled and self._sock:
                try:
                    self._sock.sendto(self._packet(), (self.addr, self.port))
                except Exception:
                    pass
            time.sleep(self.interval)

    # ---------- 调试 ----------
    def snapshot(self):
        with self._lock:
            d = dict(self._last)
        d["v"] = PROTOCOL_VERSION
        d["seq"] = self.seq
        return d


# ── 单例 ──
_bc = None


def get_broadcaster():
    global _bc
    if _bc is None:
        _bc = VehicleBroadcaster()
    return _bc
