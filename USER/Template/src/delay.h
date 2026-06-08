#ifndef __DELAY_H__
#define __DELAY_H__ 
#include "stm32f4xx.h"

#define MYCTL  *((volatile unsigned long *)0xE000E010)
#define MYLOAD  *((volatile unsigned long *)0xE000E014)
#define MYCURVAL  *((volatile unsigned long *)0xE000E018)



void Mdelay_Lib(int nms);
void Udelay_Lib(int nms);


#endif 

