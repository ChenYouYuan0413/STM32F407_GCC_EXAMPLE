#include "delay.h"

static volatile int mdelay_time;

void SysTick_Handler(void)
{
	
	if(mdelay_time > 0)
	{
		mdelay_time--;
	}

}
/*
    ���ù̼������SysTick��ʱ��ʵ�ֵĺ��뼶�����ʱ����
*/
void Mdelay_Lib(int nms)
{
    //����SysTick��ʱ������һ�������һ���ж�
    SysTick_Config(SystemCoreClock/1000);

    //��SysTick�ж����ȼ�Ϊ0(��߼�)
    NVIC_SetPriority (SysTick_IRQn,0);

    //���úú�,��ʱSysTickÿһ�������һ���ж�mdelay_time�ͼ�1
    mdelay_time = nms;
    while(mdelay_time)
;
}
/*
    ���ù̼������SysTick��ʱ��ʵ�ֵ�΢�뼶�����ʱ����
*/
void Udelay_Lib(int nms)
{
    //����SysTick��ʱ������һ΢�����һ���ж�
    SysTick_Config(SystemCoreClock/1000000);

    //��SysTick�ж����ȼ�Ϊ0(��߼�)
    NVIC_SetPriority (SysTick_IRQn,0);

    //���úú�,��ʱSysTickÿһ΢�����һ���ж�mdelay_time�ͼ�1
    mdelay_time = nms;
    while(mdelay_time)
;
}















