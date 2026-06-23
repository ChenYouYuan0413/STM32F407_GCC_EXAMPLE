#!/usr/bin/env python3
"""
UART2 小车控制 —— Mac 键盘遥控

依赖: pip install pyserial pynput
用法: python3 car_control.py [串口设备]
  例: python3 car_control.py /dev/tty.usbserial-0001

按键:
  W / S   → 前进 / 后退
  A / D   → 左转 / 右转
  Q       → 退出

同时按下 W+A = 前进+左转, 松开自动回中停止。
"""

import struct
import sys
import time
import serial
from pynput import keyboard

# ============================================================
# 协议常量
# ============================================================
HEADER0 = 0xAA
HEADER1 = 0x55
FRAME_LEN = 15
PAYLOAD_LEN = 12        # 字节 2~13

# ============================================================
# CRC-8, poly=0x07, init=0x00 (与 STM32 端完全一致)
# ============================================================
def _crc8_make_table():
    tbl = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = ((c << 1) ^ 0x07) if (c & 0x80) else (c << 1)
        tbl.append(c & 0xFF)
    return tbl

CRC8_TABLE = _crc8_make_table()

def crc8(data: bytes) -> int:
    crc = 0
    for b in data:
        crc = CRC8_TABLE[crc ^ b]
    return crc

# ============================================================
# 帧构建 (STM32F407 为小端序)
# ============================================================
def build_frame(linvel_x: float, angel_z: float) -> bytes:
    """linvel_x: 1.0=前进  -1.0=后退  0=停
       angel_z:  1.0=左转  -1.0=右转  0=直行"""
    reserved = b'\x00\x00\x00\x00'
    # < 小端序, f=float32
    payload = struct.pack('<ff4s', linvel_x, angel_z, reserved)
    chk = crc8(payload)
    return bytes([HEADER0, HEADER1]) + payload + bytes([chk])

# ============================================================
# 键盘 → 控制量
# ============================================================
linvel_x = 0.0   # 线速度 -1/0/1
angel_z  = 0.0   # 角速度 -1/0/1

_pressed = set()  # 当前按下的按键

def _update_state():
    global linvel_x, angel_z
    # W/S 互斥, 同时按则抵消
    lv = 0.0
    if 'w' in _pressed:  lv += 1.0
    if 's' in _pressed:  lv -= 1.0
    linvel_x = max(-1.0, min(1.0, lv))

    # A/D 互斥
    az = 0.0
    if 'a' in _pressed:  az += 1.0
    if 'd' in _pressed:  az -= 1.0
    angel_z = max(-1.0, min(1.0, az))

def on_press(key):
    try:
        ch = key.char
    except AttributeError:
        return  # 特殊键忽略
    if ch in ('w','a','s','d'):
        _pressed.add(ch)
        _update_state()
    elif ch == 'q':
        _running = False  # noqa (handled in loop)

def on_release(key):
    try:
        ch = key.char
    except AttributeError:
        return
    if ch in ('w','a','s','d'):
        _pressed.discard(ch)
        _update_state()
    elif ch == 'q':
        global _running
        _running = False

# ============================================================
# 主循环
# ============================================================
def main():
    global _running

    # 串口
    port = sys.argv[1] if len(sys.argv) > 1 else '/dev/tty.usbserial-0001'
    try:
        ser = serial.Serial(port, 115200, timeout=0)
        print(f'[OK] 串口已打开: {port} @115200')
    except serial.SerialException as e:
        print(f'[FAIL] 无法打开串口 {port}: {e}')
        print('用法: python3 car_control.py [串口设备]')
        sys.exit(1)

    # 键盘监听
    listener = keyboard.Listener(on_press=on_press, on_release=on_release)
    listener.start()

    print('小车已就绪。按键: W前进 S后退 A左转 D右转 Q退出')
    print('当前方向: 停车')

    _running = True
    last_state = None
    period = 0.05  # 20 Hz

    try:
        while _running:
            frame = build_frame(linvel_x, angel_z)
            ser.write(frame)

            # 终端显示方向变化
            state = (linvel_x, angel_z)
            if state != last_state:
                dirs = []
                if linvel_x > 0.1:    dirs.append('前进')
                elif linvel_x < -0.1: dirs.append('后退')
                if angel_z > 0.1:     dirs.append('左转')
                elif angel_z < -0.1:  dirs.append('右转')
                if not dirs:          dirs.append('停车')
                print(f'\r> {"+".join(dirs)}  [{linvel_x:+.0f}, {angel_z:+.0f}]  ', end='', flush=True)
                last_state = state

            time.sleep(period)
    except KeyboardInterrupt:
        pass
    finally:
        _running = False
        # 发送停车帧
        ser.write(build_frame(0.0, 0.0))
        ser.close()
        listener.stop()
        print('\n已退出，小车已停车。')

_running = False

if __name__ == '__main__':
    main()
