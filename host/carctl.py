#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
车辆控制台（交互式）
====================
直接运行即可，输入指令回车发送。

  python3 carctl.py

支持：
  w / forward / 前进     前进
  s / back / 后退        后退
  a / left / 左转        左转
  d / right / 右转       右转
  stop / 停止            停止
  q / quit / 退出        退出控制台

特殊：
  wl / 左前     wr / 右前
  sl / 左后     sr / 右后
  status       查看当前指令和连接
  raw <文字>   直接发送任意文字（不校验）
"""
import os
import sys

FIFO = "/tmp/carctl/cmd"

ALIASES = {
    "w": "w", "forward": "w", "前进": "w", "f": "w",
    "s": "s", "back": "s", "后退": "s", "b": "s",
    "a": "a", "left": "a", "左转": "a", "l": "a",
    "d": "d", "right": "d", "右转": "d", "r": "d",
    "stop": "stop", "停止": "stop", "停": "stop", "st": "stop",
    "wl": "wl", "左前": "wl",
    "wr": "wr", "右前": "wr",
    "sl": "sl", "左后": "sl",
    "sr": "sr", "右后": "sr",
}

HELP = """\
────────────────────────────────────────────────
 指令           动作
 ─────────────  ─────────────
 w  / 前进       前进
 s  / 后退       后退
 a  / 左转       左转
 d  / 右转       右转
 wl / 左前       左前
 wr / 右前       右前
 sl / 左后       左后
 sr / 右后       右后
 stop / 停止     停止
 ─────────────  ─────────────
 .              重发当前指令
 status         连接状态
 raw <文字>     发送任意内容
 q  / 退出       退出
────────────────────────────────────────────────"""


def send(cmd):
    """写入 FIFO（服务端心跳会自动重发）"""
    try:
        with open(FIFO, "w") as f:
            f.write(cmd + "\n")
        return True
    except Exception as e:
        print(f"  ✗ 发送失败: {e}")
        return False


def conn_status():
    """从 /proc/net/tcp 粗略查 9000 的连接数"""
    try:
        import subprocess
        out = subprocess.run(["ss", "-tn"], capture_output=True, text=True, timeout=3).stdout
        lines = [l for l in out.splitlines() if ":9000" in l and "ESTAB" in l]
        return len(lines), lines
    except Exception:
        return -1, []


def main():
    if not os.path.exists(FIFO):
        print(f"错误：指令通道不存在 {FIFO}")
        print("请先启动服务端：python3 tcp_ctrl_hb.py")
        sys.exit(1)

    print("=" * 50)
    print("  车辆控制台")
    print("=" * 50)
    print(HELP)

    cur = None
    while True:
        try:
            raw = input("\n指令> ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break

        if not raw:
            continue

        low = raw.lower()

        # 退出
        if low in ("q", "quit", "exit", "退出"):
            send("stop")
            print("  已发送 stop，退出")
            break

        # 重发
        if raw == ".":
            if cur:
                send(cur)
                print(f"  重发: {cur}")
            else:
                print("  还没有发过指令")
            continue

        # 状态
        if low == "status":
            n, lines = conn_status()
            print(f"  连接数: {n}")
            for l in lines:
                print(f"    {l.strip()}")
            print(f"  当前指令: {cur or '(无)'}")
            continue

        # 任意内容
        if low.startswith("raw "):
            txt = raw[4:]
            if send(txt):
                print(f"  已发送: {txt!r}")
            continue

        # 别名解析
        mapped = ALIASES.get(low)
        if mapped:
            if send(mapped):
                cur = mapped
                print(f"  ✓ 已发送: {mapped}")
            continue

        # 未识别：仍然发出去（方便测"未知指令"的拒绝逻辑）
        if send(raw):
            cur = raw
            print(f"  ? 未识别指令，已原样发送: {raw!r}（对方应打印「未知指令」）")

    print("\n再见")


if __name__ == "__main__":
    main()
