#include "bsp_beep.h"
#include "bsp.h"

uint16_t beep_on_time = 0;
uint8_t beep_state = 0;

// 刷新蜂鸣器打开的时间
static void Beep_Set_Time(uint16_t time)
{
	beep_on_time = time;
}

// 获取当前蜂鸣器打开的剩余时间
static uint16_t Beep_Get_Time(void)
{
	return beep_on_time;
}

// 刷新蜂鸣器的状态
static void Beep_Set_State(uint8_t state)
{
	beep_state = state;
}

// 获取蜂鸣器的状态
static uint8_t Beep_Get_State(void)
{
	return beep_state;
}

// 设置蜂鸣器开启时间，time=0时关闭，time=1时一直响，time>=10，延迟xx毫秒后自动关闭
void Beep_On_Time(uint16_t time)
{
	if (time == BEEP_STATE_ON_ALWAYS)
	{
		Beep_Set_State(BEEP_STATE_ON_ALWAYS);
		Beep_Set_Time(0);
		BEEP_ON();
	}
	else if (time == BEEP_STATE_OFF)
	{
		Beep_Set_State(BEEP_STATE_OFF);
		Beep_Set_Time(0);
		BEEP_OFF();
	}
	else
	{
		if (time >= 10)
		{
			Beep_Set_State(BEEP_STATE_ON_DELAY);
			Beep_Set_Time(time / 10);
			BEEP_ON();
		}
	}
}

// 蜂鸣器超时自动关闭程序, 10毫秒调用一次
void Beep_Timeout_Close_Handle(void)
{
	if (Beep_Get_State() == BEEP_STATE_ON_DELAY)
	{
		if (Beep_Get_Time())
		{
			beep_on_time--;
		}
		else
		{
			BEEP_OFF();
			Beep_Set_State(BEEP_STATE_OFF);
		}
	}
}


