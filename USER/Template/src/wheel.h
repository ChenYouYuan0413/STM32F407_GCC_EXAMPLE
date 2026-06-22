#ifndef __WHEEL_H__
#define __WHEEL_H__

// 方向宏
#define DIR_STOP     0
#define DIR_FORWARD  1
#define DIR_BACKWARD 2
#define DIR_LEFT     3
#define DIR_RIGHT    4

void wheel_init(void);
void wheel_dir(int dir);

// wheel1 占空比调节 (0~100, 步长 10), 用于矫正左右轮速差
int  wheel1_get_duty(void);
void wheel1_set_duty(int duty);

#endif
