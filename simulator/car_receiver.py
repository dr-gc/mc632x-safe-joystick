#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
============================================================
安全虚拟摇杆 —— 小车仿真控制端（Windows 接收示例）
============================================================
在 Windows 仿真主机上运行本脚本即可接收控制指令。
Linux 端（本机）以 20Hz 广播 UDP 包，本脚本持续接收并解析。

用法：
    python car_receiver.py
    python car_receiver.py --port 9000
    python car_receiver.py --list      # 只看方向变化，不驱动小车

依赖：仅 Python 标准库（无需额外安装）
============================================================
"""
import argparse
import json
import socket
import sys
import time

# ── 协议定义 ────────────────────────────────────────────────
# 方向枚举（与 Linux 端 joystick.py 的 _direction_of 输出一致）
DIRECTIONS = {
    "STOP":       (0, 0),
    "FWD":        (0, +1),
    "BACK":       (0, -1),
    "LEFT":       (-1, 0),
    "RIGHT":      (+1, 0),
    "FWD_LEFT":   (-1, +1),
    "FWD_RIGHT":  (+1, +1),
    "BACK_LEFT":  (-1, -1),
    "BACK_RIGHT": (+1, -1),
}

DIR_CN = {
    "STOP": "停止", "FWD": "前进", "BACK": "后退",
    "LEFT": "左转", "RIGHT": "右转",
    "FWD_LEFT": "左前", "FWD_RIGHT": "右前",
    "BACK_LEFT": "左后", "BACK_RIGHT": "右后",
}


# ── 你的小车控制接口（替换成仿真 API）────────────────────────
def drive(direction: str, speed: int, estop: bool):
    """
    把控制指令下发给小车仿真。

    direction : 上面 DIRECTIONS 里的 9 种之一
    speed     : 0..100 百分比
    estop     : True 表示急停（握拳触发），此时应立刻停车

    需要你按仿真平台提供的接口改写本函数，例如：
        # ROS:   self.pub.publish(Twist(...))
        # PyBullet: 设置车轮关节速度
        # Webots:  设置 DifferentialWheels motor
    """
    if estop:
        # TODO: 在你的仿真里实现急停
        pass
    vx, vy = DIRECTIONS.get(direction, (0, 0))
    # 示例：把方向向量 × 速度换算成左右轮速（差速小车）
    left = (vy + vx) * speed
    right = (vy - vx) * speed
    # TODO: 用 left / right 驱动小车
    return left, right


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=9000, help="UDP 监听端口（默认 9000）")
    ap.add_argument("--host", default="0.0.0.0", help="绑定地址（默认全部）")
    ap.add_argument("--list", action="store_true", help="只打印方向变化，不调用 drive()")
    args = ap.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        sock.bind((args.host, args.port))
    except OSError as e:
        print(f"[错误] 无法绑定 {args.host}:{args.port} —— {e}")
        print("       可能端口被占用，或需要管理员权限。")
        sys.exit(1)

    sock.settimeout(2.0)
    print("=" * 58)
    print(f"  安全虚拟摇杆 —— 小车控制接收端")
    print(f"  监听 UDP {args.host}:{args.port}")
    print("=" * 58)

    last_dir = None
    last_estop = None
    lost_at = None
    pkt_count = 0
    t_start = time.time()

    try:
        while True:
            try:
                data, addr = sock.recvfrom(2048)
            except socket.timeout:
                if lost_at is None:
                    lost_at = time.time()
                    print(f"\n[警告] {addr if 'addr' in dir() else ''} 超过 2 秒无数据 —— "
                          f"Linux 端可能已停止广播")
                    if not args.list:
                        drive("STOP", 0, False)
                continue

            lost_at = None
            pkt_count += 1

            try:
                cmd = json.loads(data.decode("utf-8"))
            except Exception as e:
                print(f"[解析失败] {e}: {data[:80]!r}")
                continue

            direction = cmd.get("dir", "STOP")
            speed = int(cmd.get("speed", 0))
            estop = bool(cmd.get("estop", False))
            hand = bool(cmd.get("hand", False))

            # 只在变化时打印，避免刷屏
            if direction != last_dir or estop != last_estop:
                arrow = {
                    "STOP": "■", "FWD": "↑", "BACK": "↓", "LEFT": "←", "RIGHT": "→",
                    "FWD_LEFT": "↖", "FWD_RIGHT": "↗",
                    "BACK_LEFT": "↙", "BACK_RIGHT": "↘",
                }.get(direction, "?")
                tag = "  [急停]" if estop else ""
                hs = "" if hand else "  (无手)"
                print(f"{arrow} {DIR_CN.get(direction, direction):4s} "
                      f"速度 {speed:3d}%  seq={cmd.get('seq')}{tag}{hs}")
                last_dir, last_estop = direction, estop

            if not args.list:
                drive(direction, speed, estop)

    except KeyboardInterrupt:
        elapsed = max(time.time() - t_start, 0.001)
        print(f"\n\n退出。共收到 {pkt_count} 包 / {elapsed:.1f}s "
              f"= {pkt_count/elapsed:.1f} Hz")
        if not args.list:
            drive("STOP", 0, False)
    finally:
        sock.close()


if __name__ == "__main__":
    main()
