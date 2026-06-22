#ifndef __FOLLOW_H__
#define __FOLLOW_H__

#include <stdint.h>

void    follow_init(void);
uint8_t follow_read(void);   // 读取三路传感器，更新 follow_buff 并返回
uint8_t follow_get(void);    // 直接返回 follow_buff 当前值

// 三路循迹, 1=黑线 0=白
// bit2: PB6(L)  bit1: PE5(M)  bit0: PE6(R)
#define FOLLOW_L   (1 << 2)  // 0x04
#define FOLLOW_M   (1 << 1)  // 0x02
#define FOLLOW_R   (1 << 0)  // 0x01

// follow_buff 状态编码
#define FW_R          1  // 右
#define FW_M          2  // 中
#define FW_RM         3  // 右+中
#define FW_L          4  // 左
#define FW_LM         6  // 左+中
#define FW_ALL_BLACK  7  // 全黑(停车线)

#endif
