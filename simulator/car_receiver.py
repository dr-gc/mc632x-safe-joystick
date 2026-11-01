#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
============================================================
安全虚拟摇杆 —— 执行端（仿真机 / 小车）接收示例
============================================================

架构（2026-11 更新）：
    板端 MC632X 现在直接对外提供控制服务，执行端主动连板端即可。

    ┌──────────┐  TCP 9001   ┌──────────────────┐  TCP 9002  ┌──────────┐
    │ PC 上位机 │ ──────────> │  MC632X 板端      │ ─────────> │ 执行端    │
    │ 手势识别  │ dir/mag/estop│  四态状态机       │  w/wl/stop │ 本程序    │
    └──────────┘             └──────────────────┘            └──────────┘

    板端负责安全判定与超时兜底，执行端只做指令到动作的映射。

协议：
    TCP 9002，纯文本行，一行一条指令
    板端 -> 执行端：  w\n / sr\n / stop\n   （0.5 秒心跳重发）
    执行端 -> 板端：  发任意内容作为心跳，板端忽略内容

用法：
    python car_receiver.py                        # 连默认板端
    python car_receiver.py --host 192.168.215.201
    python car_receiver.py --host 192.168.215.201 --port 9002 --list

============================================================
"""
import argparse
import socket
import sys
import time
from collections import deque

# ═══════════════════════════════════════════════════════════
# 指令表：板端发来的指令 -> 方向与轮速分配
# ═══════════════════════════════════════════════════════════
# (vx, vy)  vx: 右为正   vy: 前为正
DIRECTIONS = {
    "w":  (0,  1),   # 前进
    "s":  (0, -1),   # 后退
    "a":  (-1, 0),   # 左转
    "d":  (1,  0),   # 右转
    "wl": (-1, 1),   # 左前
    "wr": (1,  1),   # 右前
    "sl": (-1, -1),  # 左后
    "sr": (1, -1),   # 右后
    "stop": (0, 0),  # 停止
}

ARROWS = {
    "w": "↑", "s": "↓", "a": "←", "d": "→",
    "wl": "↖", "wr": "↗", "sl": "↙", "sr": "↘", "stop": "■",
}

NAMES = {
    "w": "前进", "s": "后退", "a": "左转", "d": "右转",
    "wl": "左前", "wr": "右前", "sl": "左后", "sr": "右后", "stop": "停止",
}


# ═══════════════════════════════════════════════════════════
# 驱动接口：换成你的仿真或真车控制
# ═══════════════════════════════════════════════════════════
class RobotDriver:
    """差速底盘驱动。换成 ROS / PyBullet / Webots 只需改这一个类。"""

    def __init__(self, linear=0.30, angular=0.60):
        self.linear = linear    # 前进速度 m/s
        self.angular = angular  # 转向角速度 rad/s
        self.left = 0.0
        self.right = 0.0

    def drive(self, direction, estop=False):
        """direction 是板端发来的指令字符串"""
        if estop or direction == "stop":
            self.left = self.right = 0.0
            self._apply()
            return

        vx, vy = DIRECTIONS.get(direction, (0, 0))
        # 归一化：斜向时每个轮子按 1/sqrt(2) 折算，避免速度突增
        norm = 0.7071 if (vx != 0 and vy != 0) else 1.0

        left = (vy * self.linear + vx * self.angular) * norm
        right = (vy * self.linear - vx * self.angular) * norm

        self.left, self.right = left, right
        self._apply()

    def _apply(self):
        """把 self.left / self.right 送到底盘

        ROS / Gazebo:
            from geometry_msgs.msg import Twist
            msg = Twist()
            msg.linear.x = (self.left + self.right) / 2
            msg.angular.z = (self.right - self.left) / 0.3
            self.pub.publish(msg)

        PyBullet:
            p.setJointMotorControl2(self.car, 0, p.VELOCITY_CONTROL,
                                    targetVelocity=self.left)
            p.setJointMotorControl2(self.car, 1, p.VELOCITY_CONTROL,
                                    targetVelocity=self.right)

        Webots:
            self.left_motor.setVelocity(self.left)
            self.right_motor.setVelocity(self.right)
        """
        pass

    def stop(self):
        self.left = self.right = 0.0
        self._apply()


# ═══════════════════════════════════════════════════════════
# 接收端
# ═══════════════════════════════════════════════════════════
class Receiver:
    def __init__(self, host, port, driver, quiet=False):
        self.host = host
        self.port = port
        self.driver = driver
        self.quiet = quiet
        self.sock = None
        self.running = False

        self.last_cmd = None
        self.last_recv = 0.0
        self.count = 0
        self.history = deque(maxlen=20)
        self.miss_count = 0

    def connect(self):
        try:
            s = socket.create_connection((self.host, self.port), timeout=5)
            s.settimeout(1.0)
            self.sock = s
            self.last_recv = time.time()
            print(f"已连接板端 {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"连接失败: {e}")
            return False

    def run(self, list_mode=False):
        self.running = True
        buf = ""

        while self.running:
            if self.sock is None:
                if not self.connect():
                    if list_mode:
                        print("板端未响应，3 秒后重试...")
                    time.sleep(3)
                    continue

            try:
                data = self.sock.recv(256)
                if not data:
                    raise ConnectionError("板端关闭连接")

                self.last_recv = time.time()
                buf += data.decode("utf-8", "replace")

                while "\n" in buf:
                    line, buf = buf.split("\n", 1)
                    line = line.strip()
                    if line:
                        self._handle(line, list_mode)

            except socket.timeout:
                # 心跳：给板端发个空行保活
                try:
                    self.sock.sendall(b"\n")
                except Exception:
                    self._reconnect()

                # 本端看门狗：板端 2 秒没发东西就自己刹车
                if time.time() - self.last_recv > 2.0:
                    if self.last_cmd != "stop":
                        self.driver.stop()
                        self.last_cmd = "stop"
                        if not list_mode:
                            print("  板端 2 秒无数据 -> 本地刹车")

            except (ConnectionError, OSError):
                self._reconnect()

    def _handle(self, cmd, list_mode):
        if cmd not in DIRECTIONS:
            return

        self.count += 1
        self.history.append((time.time(), cmd))

        estop = (cmd == "stop" and self.last_cmd not in (None, "stop"))

        self.driver.drive(cmd, estop=estop)
        self.last_cmd = cmd

        if list_mode:
            # 只在变化时打印，避免心跳刷屏
            if len(self.history) == 1 or self.history[-2][1] != cmd:
                arrow = ARROWS.get(cmd, "?")
                name = NAMES.get(cmd, cmd)
                flag = "  [急停]" if estop else ""
                print(f"  {arrow} {name:<4} 指令={cmd:<5} seq={self.count}{flag}")
        else:
            if self.history[-1][1] != cmd or (len(self.history) == 1):
                print(f"[{time.strftime('%H:%M:%S')}] {ARROWS.get(cmd,'?')} "
                      f"{NAMES.get(cmd,cmd)}  左轮={self.driver.left:+.2f} "
                      f"右轮={self.driver.right:+.2f}")

    def _reconnect(self):
        if not self.quiet:
            print("连接断开，重连中...")
        if self.sock:
            try:
                self.sock.close()
            except Exception:
                pass
        self.sock = None
        self.driver.stop()
        self.miss_count += 1
        time.sleep(1)

    def stop(self):
        self.running = False
        self.driver.stop()
        if self.sock:
            try:
                self.sock.close()
            except Exception:
                pass


def main():
    ap = argparse.ArgumentParser(
        description="安全虚拟摇杆 - 执行端接收（板端 TCP 9002）")
    ap.add_argument("--host", default="192.168.215.201",
                    help="板端 IP（默认 192.168.215.201）")
    ap.add_argument("--port", type=int, default=9002,
                    help="板端控制输出端口（默认 9002）")
    ap.add_argument("--linear", type=float, default=0.30,
                    help="前进速度 m/s（默认 0.30）")
    ap.add_argument("--angular", type=float, default=0.60,
                    help="转向角速度 rad/s（默认 0.60）")
    ap.add_argument("--list", action="store_true",
                    help="只打印收到的指令，不控制底盘（用于验证链路）")
    args = ap.parse_args()

    print("=" * 56)
    print("  安全虚拟摇杆 · 执行端")
    print("=" * 56)
    print(f"  板端地址: {args.host}:{args.port}")
    print(f"  速度参数: linear={args.linear} m/s  angular={args.angular} rad/s")
    print(f"  模式: {'仅列出指令' if args.list else '驱动底盘'}")
    print("=" * 56)
    print()

    driver = RobotDriver(linear=args.linear, angular=args.angular)
    rx = Receiver(args.host, args.port, driver, quiet=not args.list)

    try:
        rx.run(list_mode=args.list)
    except KeyboardInterrupt:
        print("\n退出")
    finally:
        rx.stop()


if __name__ == "__main__":
    main()
