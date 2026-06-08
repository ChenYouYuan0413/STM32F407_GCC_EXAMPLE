#include "delay.h"

static int mdelay_time;

void SysTick_Handler(void)
{
	
	if(mdelay_time > 0)
	{
		mdelay_time--;
	}

}
/*
    利用固件库控制SysTick定时器实现的毫秒级别的延时函数
*/
void Mdelay_Lib(int nms)
{
    //配置SysTick定时器让其一毫秒产生一次中断
    SysTick_Config(SystemCoreClock/1000);

    //让SysTick中断优先级为0(最高级)
    NVIC_SetPriority (SysTick_IRQn,0);

    //设置好后,此时SysTick每一毫秒产生一次中断mdelay_time就减1
    mdelay_time = nms;
    while(mdelay_time)
;
}
/*
    利用固件库控制SysTick定时器实现的微秒级别的延时函数
*/
void Udelay_Lib(int nms)
{
    //配置SysTick定时器让其一微秒产生一次中断
    SysTick_Config(SystemCoreClock/1000000);

    //让SysTick中断优先级为0(最高级)
    NVIC_SetPriority (SysTick_IRQn,0);

    //设置好后,此时SysTick每一微秒产生一次中断mdelay_time就减1
    mdelay_time = nms;
    while(mdelay_time)
;
}















