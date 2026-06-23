#include "main.h"
#include "delay.h"
#include "led.h"
#include "uart.h"
#include <stdint.h>
#include <stdbool.h>
#include "wheel.h"
#include "follow.h"
#include "follow_line.h"

// 自动停车宏（每轮循环约 50ms，20 次 = ~1s）
#define CMD_TIMEOUT_CNT  20
static volatile int cmd_ticks = 0;

// fabsf 替代（无 math 库）
static inline float fabsf_quick(float x) { return (x < 0.0f) ? -x : x; }

// 不同类型测试变量（全局/static = 固定地址，LinkScope 测试）
static uint32_t  u32_val;     // 32位无符号
static int32_t   s32_val;     // 32位有符号
static uint16_t  u16_val;     // 16位无符号
static int16_t   s16_val;     // 16位有符号
static uint8_t   u8_val;      // 8位无符号
static int8_t    s8_val;      // 8位有符号
static float     f_val;       // 单精度浮点
static bool      b_val;       // 布尔
static int       cnt_val;     // 普通 int
static volatile int  you_can_change;     // 普通 int

enum { STATE_IDLE, STATE_RUN, STATE_DONE };
static int state_val = STATE_IDLE;  // 枚举

typedef struct {
	uint16_t x;
	uint16_t y;
	uint8_t  flag;
	struct {
		int16_t r;
		int16_t g;
	} color;
} Point;
static Point p;  // 嵌套结构体，全零初始化

// UART1 接收缓冲区监控变量
static volatile uint8_t  uart1_last_byte;
static volatile uint16_t uart1_total_rx;

// 运行模式: 0=蓝牙遥控  1=巡线  2=前进避障
enum { MODE_REMOTE, MODE_FOLLOW, MODE_AVOID };
static volatile int run_mode = MODE_REMOTE;

// 避障子状态
enum { AVOID_FORWARD, AVOID_BACK, AVOID_TURN };
static volatile int avoid_state = AVOID_FORWARD;
static volatile int avoid_ticks = 0;
// 避障时序: 后退 3s / 50ms = 60 ticks, 旋转 2s / 50ms = 40 ticks
#define AVOID_BACK_TICKS  20
#define AVOID_TURN_TICKS  10

// 运动方向状态，由 UART1 命令更新
static volatile int cmd_dir = DIR_STOP;

// wheel1 占空比镜像，方便 GDB/LinkScope 观测
static volatile int wheel1_duty_val = 96;

// 三路循迹状态镜像，方便 GDB/LinkScope 观测
static volatile uint8_t follow_val = 0;

// UART2 协议帧解析结果镜像，方便 GDB/LinkScope 观测
static volatile float   u2p_lv_val = 0.0f;
static volatile float   u2p_az_val = 0.0f;

int main()
{
	led_init();
	you_can_change = 0 ;
	usart1_init(9600);
	usart2_init(115200);
	wheel_init();
	wheel1_set_duty(96);
	follow_init();
	follow_line_init();
	while (1)
	{
		// 读取循迹传感器
		follow_val = follow_read();

		// ---- UART2 协议帧接收 ----
		{
			uint8_t ch;
			while (uart2_read(&ch, 1) > 0) {
				uart2_proto_feed(ch);
			}
		}
		if (uart2_proto_poll()) {
			u2p_lv_val = u2p_linvel_x;
			u2p_az_val = u2p_angel_z;

			float x = u2p_linvel_x;
			float z = u2p_angel_z;
			if (fabsf_quick(x) < 0.3f && fabsf_quick(z) < 0.3f) {
				cmd_dir = DIR_STOP;
			} else if (fabsf_quick(z) > 0.3f) {
				cmd_dir = (z > 0.0f) ? DIR_LEFT : DIR_RIGHT;
			} else if (x > 0.3f) {
				cmd_dir = DIR_FORWARD;
			} else if (x < -0.3f) {
				cmd_dir = DIR_BACKWARD;
			}
			cmd_ticks = 0;
		}

		// 解析 UART1 命令
		{
			uint8_t ch;
			if (uart1_read(&ch, 1) > 0) {
				uart1_last_byte = ch;
				uart1_total_rx++;
				switch (ch) {
				case 'D': run_mode = MODE_FOLLOW; break;
				case 'E': run_mode = MODE_REMOTE; break;
				case 'F': run_mode = MODE_AVOID; avoid_state = AVOID_FORWARD; avoid_ticks = 0; cmd_dir = DIR_FORWARD; break;
				case 'G': cmd_dir = DIR_FORWARD;  cmd_ticks = 0; break;
				case 'I': cmd_dir = DIR_STOP;     cmd_ticks = 0; break;
				case 'K': cmd_dir = DIR_BACKWARD; cmd_ticks = 0; break;
				case 'H': cmd_dir = DIR_LEFT;     cmd_ticks = 0; break;
				case 'J': cmd_dir = DIR_RIGHT;    cmd_ticks = 0; break;
				case 'A': {
					int d = wheel1_get_duty();
					int step = (d >= 90) ? 1 : 10;
					wheel1_set_duty(d + step);
					wheel1_duty_val = wheel1_get_duty();
					break;
				}
				case 'C': {
					int d = wheel1_get_duty();
					int step = (d > 90) ? 1 : 10;
					wheel1_set_duty(d - step);
					wheel1_duty_val = wheel1_get_duty();
					break;
				}
				}
			}
		}

		if (run_mode == MODE_FOLLOW) {
			// 巡线模式
			follow_line_run();
		} else if (run_mode == MODE_AVOID) {
			// 前进避障模式: 全黑(7)触发后退→旋转→再前进
			switch (avoid_state) {
			case AVOID_FORWARD:
				wheel_dir(DIR_FORWARD);
				if (follow_val == 0) {
					avoid_state = AVOID_BACK;
					avoid_ticks = 0;
				}
				break;
			case AVOID_BACK:
				wheel_dir(DIR_BACKWARD);
				avoid_ticks++;
				Mdelay_Lib(50);
				if (avoid_ticks >= AVOID_BACK_TICKS) {
					avoid_state = AVOID_TURN;
					avoid_ticks = 0;
				}
				break;
			case AVOID_TURN:
				wheel_dir(DIR_LEFT);
				avoid_ticks++;
				Mdelay_Lib(50);
				if (avoid_ticks >= AVOID_TURN_TICKS) {
					avoid_state = AVOID_FORWARD;
					avoid_ticks = 0;
				}
				break;
			}
		} else {
			// 蓝牙遥控模式: UART 命令控制
			if (cmd_ticks >= CMD_TIMEOUT_CNT) {
				cmd_dir = DIR_STOP;
				cmd_ticks = 0;
			} else {
				cmd_ticks++;
			}
			wheel_dir(cmd_dir);
			Mdelay_Lib(50);
		}

		// 各类型变量递增/翻转，超出范围自动回零/测试linkscope 采样率3khz
		if (u32_val < 50000) u32_val += 1000;  else u32_val = 0;
		if (s32_val >    -5) s32_val -= 1;     else s32_val = 0;
		if (u16_val <   100) u16_val += 10;    else u16_val = 0;
		if (s16_val >    -5) s16_val -= 1;     else s16_val = 0;
		if (u8_val  <   200) u8_val  += 1;     else u8_val  = 0;
		if (s8_val  >    -5) s8_val  -= 1;     else s8_val  = 0;
		if (f_val   < 100.0f) f_val  += 1.5f;  else f_val   = 0.0f;
		b_val    = !b_val;
		if (cnt_val <   200) cnt_val += 1;     else cnt_val = 0;
		if (++state_val > STATE_DONE) state_val = STATE_IDLE;
		if (++p.x > 100)    { p.x = 0; p.y += 10; }
		if (p.y  > 200)      p.y = 0;
		p.flag = p.x > 50;
		p.color.r += 1; if (p.color.r > 100) p.color.r = 0;
		p.color.g -= 1; if (p.color.g <  -5) p.color.g = 0;

		led_ctrl(D1, LED_ON);

	}
}
