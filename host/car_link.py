#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
摇杆 → 板端状态机
==================
把摇杆状态直接送给 MC632X 板端（TCP 9001），由板端状态机判定后输出控制指令。

架构变化：
  改造前：web_host.py -> car_link.py -> Unix socket -> tcp_ctrl_hb.py -> TCP 9000 -> 小车
  改造后：web_host.py -> car_link.py -> TCP 192.168.215.201:9001 -> 板端状态机 -> 板端输出

板端接收格式（纯文本行）：
  dir=<FWD|BACK|LEFT|RIGHT|FWD_LEFT|FWD_RIGHT|BACK_LEFT|BACK_RIGHT|STOP> mag=<0~1000> estop=<0|1>

板端行为：
  · 自己跑四态状态机（STANDBY / READY / ACTIVE / EMERGENCY）
  · 1 秒收不到数据 -> 自动停车
  · 急停锁存，回中解除
  · 输出 [CMD] <w|s|a|d|wl|wr|sl|sr|stop>
"""
import socket
import threading
import time

BOARD_HOST = "192.168.215.201"
BOARD_PORT = 9001

# 摇杆幅度低于该值视为停止（0~1000），板端也有一份同样的门限
MIN_SPEED = 60

# 发送间隔：板端有 1 秒看门狗，这里 0.2 秒重发一次足够
SEND_INTERVAL = 0.2

RECONNECT_INTERVAL = 1.0


class BoardLink:
    """摇杆状态 -> 板端状态机"""

    def __init__(self, host=BOARD_HOST, port=BOARD_PORT):
        self.host = host
        self.port = port

        self.sock = None
        self.connected = False
        self._lock = threading.Lock()
        self._want_reconnect = True

        # 当前要发的状态（由 update() 更新，由发送线程周期性发出）
        self.cur_dir = "STOP"
        self.cur_mag = 0
        self.cur_estop = 0

        # 统计与调试
        self.sent_count = 0
        self.last_cmd = "stop"
        self.last_error = ""
        self.connect_count = 0
        self.enabled = True

        # 后台发送线程
        self._thread = threading.Thread(target=self._tx_loop, daemon=True)
        self._thread.start()

    # ---------- 连接管理 ----------
    def _connect(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(2.0)
            s.connect((self.host, self.port))
            s.settimeout(0.3)
            with self._lock:
                self.sock = s
                self.connected = True
                self.connect_count += 1
            print(f"[BOARD] connected {self.host}:{self.port}")
            return True
        except Exception as e:
            self.last_error = str(e)
            return False

    def _disconnect(self):
        with self._lock:
            if self.sock:
                try:
                    self.sock.close()
                except Exception:
                    pass
            self.sock = None
            self.connected = False

    # ---------- 发送线程 ----------
    def _tx_loop(self):
        """每 SEND_INTERVAL 发一次当前状态；板端靠这个续看门狗"""
        while self._want_reconnect:
            if not self.connected:
                if not self._connect():
                    time.sleep(RECONNECT_INTERVAL)
                    continue

            with self._lock:
                line = f"dir={self.cur_dir} mag={self.cur_mag} estop={self.cur_estop}\n"
                sock = self.sock

            if sock is None:
                self._disconnect()
                continue

            try:
                sock.sendall(line.encode())
                with self._lock:
                    self.sent_count += 1

                # 读 ACK（非阻塞，失败不影响发送）
                try:
                    ack = sock.recv(32)
                    if not ack:
                        raise ConnectionError("peer closed")
                except socket.timeout:
                    pass
                except (ConnectionError, OSError):
                    raise

            except Exception as e:
                self.last_error = str(e)
                print(f"[BOARD] send failed: {e}, reconnecting...")
                self._disconnect()
                time.sleep(RECONNECT_INTERVAL)
                continue

            time.sleep(SEND_INTERVAL)

    # ---------- 主更新接口 ----------
    def update(self, joy_state):
        """joy_state: joystick.JoystickState"""
        if not self.enabled:
            return

        direction = getattr(joy_state, "direction", "STOP") or "STOP"
        magnitude = int(getattr(joy_state, "magnitude", 0) or 0)
        estop = bool(getattr(joy_state, "estop", False))

        # 幅度不足时统一当作 STOP，避免板端收到无效方向
        if direction not in ("STOP", "ESTOP") and magnitude < MIN_SPEED:
            direction = "STOP"
            magnitude = 0

        if direction == "ESTOP":
            estop = True
            direction = "STOP"
            magnitude = 0

        with self._lock:
            self.cur_dir = direction
            self.cur_mag = magnitude
            self.cur_estop = 1 if estop else 0
            # 记录给前端看（板端实际输出以串口为准）
            self.last_cmd = "stop" if (estop or direction == "STOP") else direction

    # ---------- 调试 ----------
    def snapshot(self):
        with self._lock:
            return {
                "enabled": self.enabled,
                "connected": self.connected,
                "board": f"{self.host}:{self.port}",
                "cur_dir": self.cur_dir,
                "cur_mag": self.cur_mag,
                "cur_estop": self.cur_estop,
                "sent_count": self.sent_count,
                "connect_count": self.connect_count,
                "send_interval": SEND_INTERVAL,
                "min_speed": MIN_SPEED,
                "last_error": self.last_error,
            }

    def close(self):
        self._want_reconnect = False
        self._disconnect()


_link = None


def get_carlink():
    global _link
    if _link is None:
        _link = BoardLink()
    return _link
