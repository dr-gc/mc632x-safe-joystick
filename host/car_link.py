#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
摇杆 → TCP 控制通道
==================
把手势摇杆的八方向映射成仿真机认识的指令，通过 Unix socket 发给
本机运行的 tcp_ctrl_hb.py（它负责 TCP 心跳发送）。

架构：
    web_host.py (手势/摇杆)
        └─ CarLink.update(joy_state)
              └─ 方向映射 + 死区判断 + 变化检测
                    └─ Unix socket /tmp/carctl/ctrl.sock
                          └─ tcp_ctrl_hb.py（0.5s 心跳 → 仿真机）

方向 -> 指令映射（仿真机支持 w/s/a/d/stop 及斜向）：
    FWD         -> w
    BACK        -> s
    LEFT        -> a
    RIGHT       -> d
    FWD_LEFT    -> wl
    FWD_RIGHT   -> wr
    BACK_LEFT   -> sl
    BACK_RIGHT  -> sr
    STOP        -> stop
"""
import json
import os
import socket
import threading
import time

CTRL_SOCK = "/tmp/carctl/ctrl.sock"

DIR_TO_CMD = {
    "FWD": "w",
    "BACK": "s",
    "LEFT": "a",
    "RIGHT": "d",
    "FWD_LEFT": "wl",
    "FWD_RIGHT": "wr",
    "BACK_LEFT": "sl",
    "BACK_RIGHT": "sr",
    "STOP": "stop",
}

# 摇杆幅度低于该值视为停止（0~1000）
MIN_SPEED = 60


class CarLink:
    """摇杆状态 -> 车辆指令"""

    def __init__(self, sock_path=CTRL_SOCK):
        self.sock_path = sock_path
        self.last_cmd = None
        self.last_sent_at = 0.0
        self.sent_count = 0
        self.estop_latched = False
        self._lock = threading.Lock()
        self.enabled = True

    # ---------- 发送 ----------
    def _send(self, cmd):
        """通过 Unix socket 把指令送到 tcp_ctrl_hb.py"""
        try:
            s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            s.settimeout(0.3)
            s.connect(self.sock_path)
            s.sendall((cmd + "\n").encode())
            s.close()
            return True
        except Exception:
            return False

    # ---------- 主更新 ----------
    def update(self, joy_state):
        """joy_state: JoystickState（来自 HandJoystick.update）"""
        if not self.enabled:
            return

        with self._lock:
            direction = joy_state.direction or "STOP"
            magnitude = int(getattr(joy_state, "magnitude", 0) or 0)
            estop = bool(getattr(joy_state, "estop", False))

            # 幅度太小 -> 停止（防止手抖导致小车乱动）
            if direction not in ("STOP", "ESTOP") and magnitude < MIN_SPEED:
                direction = "STOP"

            # 急停：锁存，直到手离开才解除
            if estop:
                self.estop_latched = True
            elif direction == "STOP" and magnitude < MIN_SPEED:
                self.estop_latched = False

            if self.estop_latched:
                direction = "ESTOP"

            cmd = "stop" if direction in ("STOP", "ESTOP") else DIR_TO_CMD.get(direction, "stop")

            # 只在变化时发送（心跳由 tcp_ctrl_hb.py 负责）
            if cmd != self.last_cmd:
                ok = self._send(cmd)
                if ok:
                    self.last_cmd = cmd
                    self.last_sent_at = time.time()
                    self.sent_count += 1

    # ---------- 调试 ----------
    def snapshot(self):
        with self._lock:
            return {
                "enabled": self.enabled,
                "last_cmd": self.last_cmd,
                "sent_count": self.sent_count,
                "estop_latched": self.estop_latched,
                "sock": self.sock_path,
                "min_speed": MIN_SPEED,
            }


_link = None


def get_carlink():
    global _link
    if _link is None:
        _link = CarLink()
    return _link
