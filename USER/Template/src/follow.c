#include "main.h"
#include "follow.h"

#define F1_PIN  GPIO_Pin_6   // PB6
#define F2_PIN  GPIO_Pin_5   // PE5
#define F3_PIN  GPIO_Pin_6   // PE6

// 三路循迹传感器状态, GDB/LinkScope 可直接观测
// bit2=PB6(L)  bit1=PE5(M)  bit0=PE6(R), 1=黑线
//   4=左  2=中  1=右  6=左+中  3=右+中  7=全黑(停车线)
//   0=全白(脱线) 5=左右无中(异常)
static volatile uint8_t follow_buff = 0;

void follow_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;

	GPIO_InitStruct.GPIO_Pin = F1_PIN;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = F2_PIN;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = F3_PIN;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
}

uint8_t follow_read(void)
{
	follow_buff = 0;
	if (GPIO_ReadInputDataBit(GPIOB, F1_PIN) == Bit_SET)
		follow_buff |= FOLLOW_L;
	if (GPIO_ReadInputDataBit(GPIOE, F2_PIN) == Bit_SET)
		follow_buff |= FOLLOW_M;
	if (GPIO_ReadInputDataBit(GPIOE, F3_PIN) == Bit_SET)
		follow_buff |= FOLLOW_R;
	return follow_buff;
}

uint8_t follow_get(void)
{
	return follow_buff;
}
