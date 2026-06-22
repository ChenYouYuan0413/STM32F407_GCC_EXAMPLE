#include "main.h"
#include "wheel.h"

// 引脚定义
// 电机1(wheel1, PWM调速): A=PC7 B=PC8  → TIM3_CH2, TIM3_CH3
// 电机2(wheel2, 开关控制): A=PC9 B=PC11
#define WHEEL1_A   GPIO_Pin_7
#define WHEEL1_B   GPIO_Pin_8
#define WHEEL2_A   GPIO_Pin_9
#define WHEEL2_B   GPIO_Pin_11

static GPIO_TypeDef* const WHEEL_PORT = GPIOC;

// TIM3 PWM 参数: APB1=42MHz → TIM3_CLK=84MHz
// PSC=83 → 1MHz, ARR=999 → 1kHz PWM
#define TIM3_ARR     1000
#define TIM3_PSC     83

// wheel1 占空比 0~100, 步长 10, 默认 100%(与 wheel2 等速)
static volatile int wheel1_duty = 96;

int wheel1_get_duty(void)
{
	return wheel1_duty;
}

void wheel1_set_duty(int duty)
{
	if (duty < 0)   duty = 0;
	if (duty > 100) duty = 100;
	wheel1_duty = duty;
}

static inline uint16_t duty_to_ccr(int percent)
{
	return (uint16_t)((uint32_t)percent * TIM3_ARR / 100);
}

void wheel_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;

	// 时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// PC9, PC11: wheel2 — 推挽输出
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Pin   = WHEEL2_A | WHEEL2_B;
	GPIO_Init(WHEEL_PORT, &GPIO_InitStruct);

	// PC7, PC8: wheel1 — AF 模式 (TIM3 PWM)
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Pin   = WHEEL1_A | WHEEL1_B;
	GPIO_Init(WHEEL_PORT, &GPIO_InitStruct);

	GPIO_PinAFConfig(WHEEL_PORT, GPIO_PinSource7, GPIO_AF_TIM3);
	GPIO_PinAFConfig(WHEEL_PORT, GPIO_PinSource8, GPIO_AF_TIM3);

	// TIM3 时基: 1kHz
	TIM_TimeBaseInitStruct.TIM_Prescaler = TIM3_PSC;
	TIM_TimeBaseInitStruct.TIM_Period = TIM3_ARR - 1;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

	// CH2 (PC7), CH3 (PC8): PWM1 模式, 初始占空比 0
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC2Init(TIM3, &TIM_OCInitStruct);
	TIM_OC3Init(TIM3, &TIM_OCInitStruct);

	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	// wheel2 初始拉低, 停车
	GPIO_ResetBits(WHEEL_PORT, WHEEL2_A | WHEEL2_B);
}

// wheel1: 0=停 1=正转 2=反转, 导通侧用 PWM, 关断侧 CCR=0
static void wheel1_ctrl(int dir)
{
	uint16_t ccr = duty_to_ccr(wheel1_duty);
	switch (dir) {
	case 1:  // 正转: A=0 B=PWM
		TIM_SetCompare2(TIM3, 0);
		TIM_SetCompare3(TIM3, ccr);
		break;
	case 2:  // 反转: A=PWM B=0
		TIM_SetCompare2(TIM3, ccr);
		TIM_SetCompare3(TIM3, 0);
		break;
	default: // 停止: A=0 B=0
		TIM_SetCompare2(TIM3, 0);
		TIM_SetCompare3(TIM3, 0);
		break;
	}
}

// wheel2: 保持 GPIO 开关控制
static void wheel2_ctrl(int dir)
{
	switch (dir) {
	case 1:
		GPIO_ResetBits(WHEEL_PORT, WHEEL2_A);
		GPIO_SetBits  (WHEEL_PORT, WHEEL2_B);
		break;
	case 2:
		GPIO_SetBits  (WHEEL_PORT, WHEEL2_A);
		GPIO_ResetBits(WHEEL_PORT, WHEEL2_B);
		break;
	default:
		GPIO_ResetBits(WHEEL_PORT, WHEEL2_A | WHEEL2_B);
		break;
	}
}

void wheel_dir(int dir)
{
	switch (dir) {
	case DIR_STOP:      // 停车
		wheel1_ctrl(0);
		wheel2_ctrl(0);
		break;
	case DIR_FORWARD:   // 双电机正转
		wheel1_ctrl(1);
		wheel2_ctrl(1);
		break;
	case DIR_BACKWARD:  // 双电机反转
		wheel1_ctrl(2);
		wheel2_ctrl(2);
		break;
	case DIR_LEFT:      // 左转
		wheel1_ctrl(1);
		wheel2_ctrl(2);
		break;
	case DIR_RIGHT:     // 右转
		wheel1_ctrl(2);
		wheel2_ctrl(1);
		break;
	default:
		wheel1_ctrl(0);
		wheel2_ctrl(0);
		break;
	}
}
