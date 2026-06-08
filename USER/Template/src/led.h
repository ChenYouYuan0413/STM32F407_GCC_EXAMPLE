#ifndef __LED_H__
#define __LED_H__
#include "main.h"
//LED灯的编号和 LED灯的状态的宏定义
enum LED_NUM{D1=1,D2,D3,D4};
enum LED_STATE{LED_ON,LED_OFF};

//LED灯引脚初始化
void led_init(void);
/*LED灯 控制函数
		应该要有两个参数 
				第一个选择要操作的是哪一盏灯 
				第二个选择控制的灯的状态
*/
void led_ctrl(int led_num,int led_state);


//按键初始化  PA0 PE2 PE3 PE4
void key_init(void);

//按键控灯 因为电平信号是由按键进行输入的 只需要一直去检测按键的状态即可
void key_ctrl_led(void);

	#endif
