#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
TCP 控制服务端（心跳版）
=======================
- 无限 accept，断线不退出
- **心跳**：每 0.5s 重发当前指令；看门狗不会被误触
- 指令来自 FIFO：echo "w" > /tmp/carctl/cmd
- 发一次 w 小车持续走；发 stop 才停
- 笔记本真失联 -> 桥接侧 3s 看门狗自动刹车（安全气囊保留）
"""
import os
import socket
import threading
import time

PORT = 9000
FIFO = "/tmp/carctl/cmd"
UNIX_SOCK = "/tmp/carctl/ctrl.sock"
HEARTBEAT_SEC = 0.5

clients = []                    # [(conn, addr), ...]
clients_lock = threading.Lock()
cur = ["stop"]                  # 当前指令，默认停止
cur_lock = threading.Lock()


def reader():
    """FIFO 读指令 -> 只更新 cur，不直接发送（由心跳统一发）"""
    while True:
        if not os.path.exists(FIFO):
            try:
                os.mkfifo(FIFO)
            except FileExistsError:
                pass
            except Exception as e:
                print(f"!! mkfifo 失败: {e}", flush=True)
                return
        try:
            with open(FIFO, "r") as f:
                for line in f:
                    line = line.strip()
                    if not line:
                        continue
                    with cur_lock:
                        cur[0] = line
                    print(f"当前指令 -> {line}", flush=True)
        except Exception as e:
            print(f"!! FIFO 读异常: {e}", flush=True)


threading.Thread(target=reader, daemon=True).start()


def unix_listener():
    """摇杆侧（web_host.py）通过 Unix socket 发指令"""
    os.makedirs(os.path.dirname(UNIX_SOCK), exist_ok=True)
    if os.path.exists(UNIX_SOCK):
        try:
            os.unlink(UNIX_SOCK)
        except OSError:
            pass
    us = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    us.bind(UNIX_SOCK)
    us.listen(8)
    try:
        os.chmod(UNIX_SOCK, 0o666)
    except OSError:
        pass
    print(f"Unix socket: {UNIX_SOCK}", flush=True)
    while True:
        try:
            c, _ = us.accept()
        except OSError:
            return
        try:
            data = c.recv(1024).decode(errors="replace")
            for line in data.splitlines():
                line = line.strip()
                if not line:
                    continue
                with cur_lock:
                    cur[0] = line
                print(f"[JOY] 当前指令 -> {line}", flush=True)
        except OSError:
            pass
        finally:
            try:
                c.close()
            except OSError:
                pass


threading.Thread(target=unix_listener, daemon=True).start()


def heartbeat(conn, addr):
    """每 0.5s 重发当前指令"""
    last_sent = None
    while True:
        with clients_lock:
            alive = conn in [c for c, _ in clients]
        if not alive:
            return
        with cur_lock:
            cmd = cur[0]
        try:
            conn.sendall((cmd + "\n").encode())
            if cmd != last_sent:
                print(f"> [{addr[0]}] {cmd}", flush=True)
                last_sent = cmd
        except OSError:
            print(f"!! 心跳发送失败 {addr} (连接断开)", flush=True)
            with clients_lock:
                if conn in [c for c, _ in clients]:
                    pass
            return
        time.sleep(HEARTBEAT_SEC)


def rx(conn, addr):
    while True:
        try:
            d = conn.recv(1024)
        except OSError:
            break
        if not d:
            break
        for line in d.decode(errors="replace").splitlines():
            if line.strip():
                print(f"< [{addr[0]}] {line}", flush=True)
    with clients_lock:
        for i, (c, a) in enumerate(clients):
            if c is conn:
                clients.pop(i)
                break
    try:
        conn.close()
    except OSError:
        pass
    print(f"peer closed {addr} (等下一个连接)", flush=True)


srv = socket.socket()
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(("0.0.0.0", PORT))
srv.listen(5)
print(f"listening {PORT} ...", flush=True)
print(f"心跳: 每 {HEARTBEAT_SEC}s 重发当前指令 | 指令通道: echo \"w\" > {FIFO}", flush=True)

while True:
    try:
        conn, addr = srv.accept()
    except KeyboardInterrupt:
        print("\n退出", flush=True)
        break
    with clients_lock:
        clients.append((conn, addr))
    print("connected:", addr, flush=True)
    threading.Thread(target=rx, args=(conn, addr), daemon=True).start()
    threading.Thread(target=heartbeat, args=(conn, addr), daemon=True).start()
