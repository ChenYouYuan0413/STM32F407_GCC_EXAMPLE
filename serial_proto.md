# UART2 控制协议

## 概述

| 项目 | 说明 |
|------|------|
| 串口 | USART2（PA2=TX, PA3=RX） |
| 波特率 | 115200 |
| 数据位 | 8 |
| 停止位 | 1 |
| 校验位 | 无 |
| 字节序 | 小端（Little Endian） |
| 帧长 | 15 字节 |

## 帧格式

```
字节  0    1    2  3  4  5    6  7  8  9    10 11 12 13   14
     [AA] [55] [ linvel_x  ] [  angel_z  ] [  reserved  ] [CRC8]
              \___________/   \__________/   \__________/
               float32 LE      float32 LE     uint8[4]
```

| 偏移 | 长度 | 字段 | 类型 | 说明 |
|------|------|------|------|------|
| 0 | 1 | HEAD0 | uint8 | 固定 `0xAA` |
| 1 | 1 | HEAD1 | uint8 | 固定 `0x55` |
| 2 | 4 | linvel_x | float32 | 线速度, 范围 [-1.0, 1.0] |
| 6 | 4 | angel_z | float32 | 角速度, 范围 [-1.0, 1.0] |
| 10 | 4 | reserved | uint8[4] | 保留位, 填充 0x00 |
| 14 | 1 | crc8 | uint8 | 对字节 2~13 的 CRC-8 校验 |

## 控制含义

### linvel_x（线速度）

| 值 | 动作 |
|----|------|
| 1.0 | 前进 |
| 0 | 停止 |
| -1.0 | 后退 |

### angel_z（角速度）

| 值 | 动作 |
|----|------|
| 1.0 | 左转 |
| 0 | 直行 |
| -1.0 | 右转 |

### 组合控制

| linvel_x | angel_z | 小车行为 |
|----------|---------|----------|
| 1.0 | 0 | 前进 |
| -1.0 | 0 | 后退 |
| 0 | 1.0 | 原地左转 |
| 0 | -1.0 | 原地右转 |
| 1.0 | 1.0 | 前进+左转 |
| 1.0 | -1.0 | 前进+右转 |
| 0 | 0 | 停车 |

> **注意**：z 的正负与 x 无关。z > 0 恒为左转，z < 0 恒为右转，后退时不反转。

## CRC-8 校验

| 参数 | 值 |
|------|-----|
| 多项式 | `0x07`（x⁸ + x² + x¹ + 1） |
| 初始值 | `0x00` |
| 输入不反转 | — |
| 输出不反转 | — |
| 计算范围 | 字节 2 ~ 13（payload，共 12 字节） |

校验流程：计算 payload 的 CRC-8 → 与帧尾 `crc8` 字段比较。不匹配则丢弃该帧。

## Python 发包示例

```python
import struct
import serial

def crc8(data):
    """CRC-8, poly=0x07"""
    table = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = ((c << 1) ^ 0x07) if (c & 0x80) else (c << 1)
        table.append(c & 0xFF)

    crc = 0
    for b in data:
        crc = table[crc ^ b]
    return crc

def build_frame(linvel_x, angel_z):
    """linvel_x, angel_z: -1.0 / 0 / 1.0"""
    payload = struct.pack('<ff4s', linvel_x, angel_z, b'\x00\x00\x00\x00')
    crc = crc8(payload)
    return b'\xAA\x55' + payload + bytes([crc])

# 前进+左转
frame = build_frame(1.0, 1.0)
ser = serial.Serial('/dev/tty.usbserial-xxxx', 115200)
ser.write(frame)
```

## 安全保护

收到有效帧后约 1 秒内若无新帧，小车自动停车。
