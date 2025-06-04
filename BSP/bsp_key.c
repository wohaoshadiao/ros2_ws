#include "bsp_key.h"
#include "bsp.h"
/*---------
 *
 * static:
 * const: 修饰常量关键字
 * uint8_t:	<stdint.h>下一种类型，保证跨平台下的数据类型和大小相同
 * define: 宏定义（常量宏，函数宏等等）
 */

// Static关键字：静态局部变量，利于代码模块化，存储于全局数据区
static uint8_t Key1_is_Press(void)
{
	if (!HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin))
	{
		return KEY_PRESS; // 如果按键被按下，则返回KEY_PRESS
	}
	return KEY_RELEASE;   // 如果按键是松开状态，则返回KEY_RELEASE
}


/* mode:设置模式，0：按下一直返回KEY_PRESS；1：按下只返回一次KEY_PRESS
*Function: 简易按键消抖（按键超过两次，则输出KEY_PRESS）
*/
uint8_t Key1_State(uint8_t mode)
{
	static uint16_t key1_state = 0;

	if (Key1_is_Press() == KEY_PRESS)
	{
		if (key1_state < (mode + 1) * 2)
		{
			key1_state++;
		}
	}
	else
	{
		key1_state = 0;
	}
	if (key1_state == 2)
	{
		return KEY_PRESS;
	}
	return KEY_RELEASE;
}

void Handle_Press(key_status_t* flag)
{
    if(flag->car_state == 0)
    {
        Motion_Ctrl(100, 0, 0);
        flag->car_state = 1;
        flag->Timer_Test_Right = 0;
    }
}

void Handle_NoPress(key_status_t* flag2)
{
    if( flag2->car_state == 1)
    {
        flag2->Timer_Test_Right++;
        // 2s后向右运动
        if(flag2->Timer_Test_Right >= 200)
        {
            Motion_Stop(STOP_BRAKE);
            flag2->Timer_Test_Right = 0;
            flag2->car_state = 0;
        }
    }
}



/*********************************************END OF FILE**********************/
