# 板端代码（MC632X + RT-Thread）

本目录收录在富瀚 MC632X SDK V3.4.0 基础上修改的文件，属于源码覆盖层。

构建时需要自行准备完整的外部 SDK。本仓库不包含厂商 SDK、交叉编译工具链 arm-fullhanv2-eabi-b3、人形检测模型 30055、厂商静态库与 CV2005 sensor 参数，这些内容版权归富瀚微电子所有。

firmware/ 下的文件源自厂商示例代码，本项目在其基础上做了修改，原有版权声明保留。这些文件不适用仓库根的 MIT 许可证，使用与再分发应遵守富瀚微电子提供的原始 SDK 许可条款。

## 文件说明

| 文件 | 原始路径 | 修改内容 |
|---|---|---|
| sample_nna_obj_detect.c | app/media_demo/sample_demo/nna_app/src/ | 人形检测主循环、模型内联、摇杆状态机、串口输出格式 |
| nna_config.c | app/media_demo/sample_common/MC632X/dsp/nna/src/ | 新增 mem: 前缀与 model_from_mem 标志，静态模型跳过 free |
| enc_config.c | app/media_demo/sample_common/MC632X/dsp/enc/src/ | GOP 由 50 改为 15，降低 RTSP 首帧延迟 |
| Sensor1_Kconfig | app/media_demo/script/ | 新增 CV2005 选项（原厂只给 Sensor0 配了这类 sensor）|
| media_demo.config | app/media_demo/ | 完整 menuconfig 配置，含双摄与屏显 |

## 关键配置

| 配置项 | 值 | 说明 |
|---|---|---|
| VIDEO_GRP0 / VIDEO_GRP1 | y | 启用两组视频处理 |
| CONFIG_SENSOR_SLAVE_MODE | y | 双摄主从同步，不开则第二路 ISP 报 -3003 |
| FH_USING_CV2005_MIPI_G0/G1 | y | 两路都用 CV2005 |
| FH_APP_USING_FORMAT_1080P15_G0/G1 | y | Sensor 输出格式（CV2005 仅支持 1080P）|
| CH0_ENABLE_G0/G1 | y | 两路 H.264 编码通道 |
| CH1_ENABLE_G0 | y | 屏显通道 |
| FH_APP_USING_RTSP_G0/G1 | y | 两路 RTSP 服务（1246 / 1252）|
| DEVICE_DHD0 | y | 显示设备 |
| YCMEAN_EN_G0 | 未设置 | 关闭 loop buffer，屏显必需 |
| FH_APP_OPEN_OVERLAY | 未设置 | 关闭 OSD，初始化失败会中止整个 demo |

## 模型内联

模型不依赖文件系统，直接编进固件 `.data` 段：

```c
asm(
  ".section .data\n"
  ".global persondet_nbg_start\n"
  "persondet_nbg_start:\n"
  ".incbin \"persondet.nbg\"\n"
  ".global persondet_nbg_end\n"
  "persondet_nbg_end:\n"
  ".previous\n"
);
```

配套在 nna_config.c 中使用 mem: 前缀加载并标记 model_from_mem，避免对静态区内存调用 free。

## 串口输出格式

```
[BOX] num=1 maxconf=0.979
[ROD] UNLOCK x=-121 y=1000 xs=-59 ys=523 find=1 rz=none bb=293,0,705,987
NN[0] Det FPS: 5.000000
```

bb 字段为归一化 ×1000 的检测框坐标 x,y,w,h。

## 构建步骤

```bash
# 1. 把本目录文件放回 SDK 对应位置
# 2. 生成 config.h
python3 script/rtt/menu_anno.py .config config.h MC632X

# 3. 编译
export PATH=/opt/fullhan/toolchain/arm-fullhanv2-eabi-b3/bin:$PATH
make -j8

# 4. 产物
ls -lh out/bin/media_demo.bin    # 约 4.1 MB
```

## 烧录

```
1. 板子复位，等 Hit any key 提示，快速连按空格进入 U-Boot
2. tftp 40000000 media_demo.bin
3. go 40000000
4. ifconfig e0 192.168.215.201 netmask 255.255.255.0
5. vlcview -p 1240
```

板子每次重启后 IP 都是 0.0.0.0，RT-Thread 默认不启用 DHCP，必须重新配置。

## 已知问题

| 问题 | 现象 | 状态 |
|---|---|---|
| LCD 数据线 | 只能显示黑白，RGB 颜色不显示 | 软件侧已穷尽排查，判定物理层 |
| CV2005 帧率 | 配置 15 fps，实际输出 6.55 fps | Sensor 与 MIPI 时序限制 |
| CV2005 分辨率 | 不支持 720P，报 -3003 | 驱动库只含 1080P 配置 |
