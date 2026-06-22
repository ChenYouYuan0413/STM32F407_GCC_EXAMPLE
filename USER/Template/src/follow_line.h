#ifndef __FOLLOW_LINE_H__
#define __FOLLOW_LINE_H__

// 巡线模式状态机, 基于 follow_buff 的 switch 状态机
void follow_line_init(void);
void follow_line_run(void);   // 每轮主循环调用一次, 直接控制 wheel

#endif
