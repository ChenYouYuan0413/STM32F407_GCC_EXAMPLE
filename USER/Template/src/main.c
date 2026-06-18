#include "main.h"
#include "delay.h"
#include "led.h"
#include <stdint.h>
#include <stdbool.h>

// 不同类型测试变量（全局/static = 固定地址，LinkScope/OpenOCD 可读）
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

int main()
{
	led_init();
	you_can_change = 0 ;

	while (1)
	{
		led_ctrl(D1, LED_ON);
		Mdelay_Lib(50);

		// 各类型变量递增/翻转，超出范围自动回零
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

		led_ctrl(D1, LED_OFF);
		Mdelay_Lib(50);
	}
}


