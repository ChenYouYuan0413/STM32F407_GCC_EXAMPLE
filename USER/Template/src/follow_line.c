#include "main.h"
#include "follow_line.h"
#include "follow.h"
#include "wheel.h"
#include "delay.h"

// 反向点刹时长 (ms), 抑制过冲惯性
#define BRAKE_MS  5
#define MID_GAP_MS  10 // 大转弯时中传感的bug修复

void follow_line_init(void)
{
	// 巡线初始化, 当前无需额外配置
}

void follow_line_run(void)
{
	uint8_t s = follow_read();

	switch (s) {
	case 2:  // 中 → 直行
		wheel_dir(DIR_FORWARD);
		break;
	case 4:  // 左 → 左转修正, 中传感器检测到黑线后反向点刹
		wheel_dir(DIR_LEFT);
		while (!(follow_read() & FOLLOW_M));
		wheel_dir(DIR_RIGHT);
		Mdelay_Lib(BRAKE_MS);
		wheel_dir(DIR_FORWARD);
		break;
	case 1:  // 右 → 右转修正, 中传感器检测到黑线后反向点刹
		wheel_dir(DIR_RIGHT);
		while (!(follow_read() & FOLLOW_M));
		wheel_dir(DIR_LEFT);
		Mdelay_Lib(BRAKE_MS);
		wheel_dir(DIR_FORWARD);
		break;
	case 6:  // 左+中 → 急左转, 中传感器检测到黑线后反向点刹
		wheel_dir(DIR_LEFT);
		Mdelay_Lib(MID_GAP_MS);
		while (!(follow_read() & FOLLOW_M));
		wheel_dir(DIR_RIGHT);
		Mdelay_Lib(BRAKE_MS);
		wheel_dir(DIR_FORWARD);
		break;
	case 3:  // 右+中 → 急右转, 中传感器检测到黑线后反向点刹
		wheel_dir(DIR_RIGHT);
		Mdelay_Lib(MID_GAP_MS);
		while (!(follow_read() & FOLLOW_M));
		wheel_dir(DIR_LEFT);
		Mdelay_Lib(BRAKE_MS);
		wheel_dir(DIR_FORWARD);
		break;
	case 7:  // 全黑(停车线) → 停车
		wheel_dir(DIR_STOP);
		break;
	case 0:  // 全白(没检测到线) → 直行保持
		wheel_dir(DIR_FORWARD);
		break;
	default: // 5 等异常状态 → 停车
		wheel_dir(DIR_STOP);
		break;
	}
}
