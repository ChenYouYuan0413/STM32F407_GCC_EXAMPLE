#ifndef __UART2_PROTO_H__
#define __UART2_PROTO_H__

#include <stdint.h>

// 帧格式常量
#define U2P_HEADER0     0xAA
#define U2P_HEADER1     0x55
#define U2P_FRAME_LEN   15
#define U2P_PAYLOAD_LEN 12   // 字节 2~13

// ISR / 主循环逐字节喂入
void uart2_proto_feed(uint8_t byte);

// 主循环轮询: 返回 1 表示刚收到一帧有效数据
int  uart2_proto_poll(void);

// 解析结果 (volatile, GDB/LinkScope 可观测)
extern volatile float   u2p_linvel_x;
extern volatile float   u2p_angel_z;
extern volatile uint8_t u2p_frame_ready;
extern volatile uint8_t u2p_crc_errors;
extern volatile uint8_t u2p_state;
extern volatile uint8_t u2p_idx;
extern volatile uint8_t u2p_buf[U2P_FRAME_LEN];

#endif
