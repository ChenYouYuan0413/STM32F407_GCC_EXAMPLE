#include "led.h"




//LED灯引脚初始化
	void led_init(void)
	{
					//1.使能对应的时钟
					RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF, ENABLE);
					//2.初始化LED灯对应的引脚
					GPIO_InitTypeDef GPIO_InitStruct;
					GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //引脚
					GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT; //输出功能
					GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; //输出一般设置50MHz
					GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; // 推挽输出
					//初始化PF9 PF10
					GPIO_Init(GPIOF, &GPIO_InitStruct);
					//初始化PE13 PE14
					GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
					GPIO_Init(GPIOE, &GPIO_InitStruct);
					// led_ctrl(D1,LED_OFF);
					// led_ctrl(D2,LED_OFF);
					// led_ctrl(D3,LED_OFF);
					// led_ctrl(D4,LED_OFF);
								
					
	}
	
/*LED灯 控制函数
	应该要有两个参数 
		第一个选择要操作的是哪一盏灯 
		第二个选择控制的灯的状态
*/
void led_ctrl(int led_num,int led_state)
{
	GPIO_TypeDef *GPIOx;//需要操作的GPIO引脚分组 
	uint16_t GPIO_Pin_x;//需要操作的引脚的编号
	
	//判断是哪一盏灯
	switch(led_num)
	{
		case D1:
			GPIOx = GPIOF;
			GPIO_Pin_x = GPIO_Pin_9;
			break;
		case D2:
			GPIOx = GPIOF;
			GPIO_Pin_x = GPIO_Pin_10;
			break;
		case D3:
			GPIOx = GPIOE;
			GPIO_Pin_x = GPIO_Pin_13;
			break;
		case D4:
			GPIOx = GPIOE;
			GPIO_Pin_x = GPIO_Pin_14;
			break;
			
//			case D2: 
//			case D3: 
//			case D4: 
		/*
			交给你们来补全
		*/
	}
	
	//判断灯的状态
	if(led_state == LED_ON)
	{
		GPIO_ResetBits(GPIOx,GPIO_Pin_x);
	}
	else
	{
		GPIO_SetBits(GPIOx,GPIO_Pin_x);
	}
}

//按键初始化  PA0 PE2 PE3 PE4
void key_init(void)
{
		//所有变量的定义必须要在函数的最前面进行定义
		//使能时钟 
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA, ENABLE);
		//2.初始化LED灯对应的引脚
		GPIO_InitTypeDef GPIO_InitStruct;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0; //引脚
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN; //输入功能
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; //输出一般设置50MHz
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; // 上拉输入
		//初始化PA0
		GPIO_Init(GPIOA, &GPIO_InitStruct);
		//初始化PE2 PE3 PE4
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
		GPIO_Init(GPIOE, &GPIO_InitStruct);
}

//按键控灯 因为电平信号是由按键进行输入的 只需要一直去检测按键的状态即可
void key_ctrl_led(void)
{
		int k[4] = {1,1,1,1};//分别保存四个按键的状态值
		int i;//不同按键的下标
		while(1)
		{
				//轮流查询各个按键的状态
				k[0] = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0);
				k[1] = GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2);
				k[2] = GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3);
				k[3] = GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4);
				//写一个循环来点亮对应的LED灯 
				for(i = 0;i < 4;i++)
				{
						if(k[i] == 0)	//对应的按键被按下了
						{
								led_ctrl(i+1,LED_ON);
						}
						else
						{
								led_ctrl(i+1,LED_OFF);
						}
				}
		}
}
