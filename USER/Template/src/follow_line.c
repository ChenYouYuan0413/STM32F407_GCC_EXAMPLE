#include "main.h"
#include "follow_line.h"
#include "follow.h"
#include "wheel.h"

void follow_line_init(void)
{
	// 巡线初始化, 当前无需额外配置
}

void follow_line_run(void)
{
	uint8_t s = follow_get();

	switch (s) {
	case 2:  // 中 → 直行
		wheel_dir(DIR_FORWARD);
		break;
	case 4:  // 左 → 左转修正
		wheel_dir(DIR_LEFT);
		break;
	case 1:  // 右 → 右转修正
		wheel_dir(DIR_RIGHT);
		break;
	case 6:  // 左+中 → 急左转
		wheel_dir(DIR_LEFT);
		break;
	case 3:  // 右+中 → 急右转
		wheel_dir(DIR_RIGHT);
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
