#!/usr/bin/env bash
# 快捷控制脚本：./car.sh w
# 用法: ./car.sh w|s|a|d|stop|wl|wr|sl|sr|status

FIFO=/tmp/carctl/cmd

if [ ! -p "$FIFO" ]; then
    echo "错误：指令通道不存在 $FIFO"
    echo "请先启动服务端: cd ~/mc630x_rtthread/host && python3 tcp_ctrl_hb.py"
    exit 1
fi

case "${1:-}" in
    status|"")
        echo "=== TCP 9000 连接 ==="
        ss -tn 2>/dev/null | grep ":9000" | grep ESTAB || echo "  (无连接)"
        echo "=== 服务端进程 ==="
        pgrep -f tcp_ctrl_hb.py >/dev/null && echo "  运行中" || echo "  未运行"
        exit 0
        ;;
esac

echo "$1" > "$FIFO"
echo "已发送: $1"
