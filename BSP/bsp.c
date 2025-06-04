#include "bsp.h"

// LED显示当前运行状态，每10毫秒调用一次，LED灯每200毫秒闪烁一次。
// The LED displays the current operating status, which is invoked every 10 milliseconds, and the LED blinks every 200 milliseconds.  
static void Bsp_Led_Show_State_Handle(void)
{
	static uint8_t led_count = 0;
	led_count++;
	if (led_count > 20)
	{
		led_count = 0;
	//	LED_TOGGLE();
	}
}


// The peripheral device is initialized  外设设备初始化
void Bsp_Init(void)
{
	Beep_On_Time(50);
	Motor_Init();
    USART1_Init();
	Encoder_Init();
	PID_Param_Init();
}

int encoder[4] = {0};
int show_encoder = 0;
int car_state = 0;

// main.c中循环调用此函数，避免多次修改main.c文件。
// This function is called in a loop in main.c to avoid multiple modifications to the main.c file
void Bsp_Loop_Motor_Test(void)
{
	// Detect button down events   检测按键按下事件
	if (Key1_State(KEY_MODE_ONE_TIME))
	{
		Beep_On_Time(50);
		static int state = 0;
		state++;
		int speed = 0;
		if (state == 1)                 // Left Shift
		{
			speed = 2000;
			int16_t against_speed = -speed;
			Motor_Set_Pwm(MOTOR_ID_M1, speed);
			Motor_Set_Pwm(MOTOR_ID_M2, against_speed);
			Motor_Set_Pwm(MOTOR_ID_M3, against_speed);
			Motor_Set_Pwm(MOTOR_ID_M4, speed);
		}
		if (state == 2)
		{
			Motor_Stop(0);
		}
		if (state == 3)                 // Right Shift
		{
			speed = -2000;
            int16_t against_Two_speed = -speed;
			Motor_Set_Pwm(MOTOR_ID_M1, speed);
			Motor_Set_Pwm(MOTOR_ID_M2, against_Two_speed);
			Motor_Set_Pwm(MOTOR_ID_M3, against_Two_speed);
			Motor_Set_Pwm(MOTOR_ID_M4, speed);
		}
		if (state == 4)
		{
			state = 0;
			Motor_Stop(1);
		}

	}

	Bsp_Led_Show_State_Handle();
	Beep_Timeout_Close_Handle();
	HAL_Delay(10);
}

// main.c调用此函数，功能为收SBUS协议数据，在串口中显示

void Bsp_Loop_Test_SBUS(void)
{
    // Detect button down events   检测按键按下事件
    if (Key1_State(KEY_MODE_ONE_TIME))
    {
        Beep_On_Time(50);
        static int press = 0;
        press++;
        printf("press:%d\n", press);
    }
    SBUS_Handle();

    Bsp_Led_Show_State_Handle();
    // The buzzer automatically shuts down when times out   蜂鸣器超时自动关闭
    Beep_Timeout_Close_Handle();
    HAL_Delay(10);
}



/**
 * Time: 2025/4/21
 * author: warnie
 * function: The right-hand potentiometer controls the translation, The left-hand potentiometer controls the rotation
 * issues：No PID is added and the potential value is not converted to a translation angle
 */
void Bsp_Loop_Manual_Control(void)
{
    SBUS_Handle_Manual();

    Bsp_Led_Show_State_Handle();
    // The buzzer automatically shuts down when times out   蜂鸣器超时自动关闭
    Beep_Timeout_Close_Handle();
    HAL_Delay(10);
}

/**
 * @Brief: 获取运动后编码电机的数值，10ms返回一次
 * @Time:
 *
 */
void Bsp_Loop_Test_Encoder(void)
{
// Detect button down events   检测按键按下事件
	if (Key1_State(KEY_MODE_ONE_TIME))
	{
		Beep_On_Time(50);
		static int state = 0;
		state++;
		int speed = 0;
		if (state == 1)
		{
			speed = 2000;
			Motor_Set_Pwm(MOTOR_ID_M1, speed);
			Motor_Set_Pwm(MOTOR_ID_M2, speed);
			Motor_Set_Pwm(MOTOR_ID_M3, speed);
			Motor_Set_Pwm(MOTOR_ID_M4, speed);
		}
		if (state == 2)
		{
			Motor_Stop(0);
		}
		if (state == 3)
		{
			speed = -2000;
			Motor_Set_Pwm(MOTOR_ID_M1, speed);
			Motor_Set_Pwm(MOTOR_ID_M2, speed);
			Motor_Set_Pwm(MOTOR_ID_M3, speed);
			Motor_Set_Pwm(MOTOR_ID_M4, speed);
		}
		if (state == 4)
		{
			state = 0;
			Motor_Stop(1);
		}
	}

	show_encoder++;
	if (show_encoder > 10)
	{
		show_encoder = 0;
		Encoder_Get_ALL(encoder);
		printf("Encoder:%d, %d, %d, %d\n", encoder[0], encoder[1], encoder[2], encoder[3]);
	}

	Encoder_Update_Count();
	Bsp_Led_Show_State_Handle();
	Beep_Timeout_Close_Handle();
	HAL_Delay(10);
}


void Bsp_Loop_Test_PID(void)
{
	Motion_Handle();

    static key_status_t Flag_Status={0};
    static uint8_t Key_Flag = 0;
    Key_Flag = Key1_State(0);

    switch (Key_Flag)
    {
        // 按键按下，一直返回Press
        case Key_Status_Press :
            Beep_On_Time(50);
            Handle_Press(&Flag_Status);
            break;

        case Key_Status_NoPress:
            // 增加判断
          Handle_NoPress(&Flag_Status);
            break;
    }

	Bsp_Led_Show_State_Handle();
	Beep_Timeout_Close_Handle();
	HAL_Delay(10);
}


void Bsp_Loop_Test_ros2_cmdvel(void)
{
    USART1_Init();
    int16_t V_x = (int16_t)(linear_x_value * 10);
    int16_t V_z = (int16_t)(angular_z_value * 10);
    static uint8_t status_flag = 0;
    int speed = 0;
    // 调用你的运动控制函数
//    Motion_Handle();
    if(V_x > 0 )
        status_flag = 1;
    else if (V_x < 0)
        status_flag = 2;
    else if(V_z > 0)
        status_flag = 3;
    else if(V_z < 0)
        status_flag = 4;
    else status_flag = 5;

    switch (status_flag)
    {
        // 按键按下，一直返回Press
        case Status_up :
            Beep_On_Time(50);
            speed = 150;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, speed);
            Motor_Set_Pwm(MOTOR_ID_M3, speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
            status_flag = 0;
            break;

        case Status_back:
            // 增加判断
            speed = -150;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, speed);
            Motor_Set_Pwm(MOTOR_ID_M3, speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
            status_flag = 0;
            break;
        case Status_left:
            // 增加判断
            speed = 150;
            int16_t against_speed = -speed;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, against_speed);
            Motor_Set_Pwm(MOTOR_ID_M3, against_speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
            status_flag = 0;
            break;
        case Status_right:
            // 增加判断
            speed = -150;
            int16_t against_Two_speed = -speed;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, against_Two_speed);
            Motor_Set_Pwm(MOTOR_ID_M3, against_Two_speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
            status_flag = 0;
            break;
        default:
            Motor_Stop(0);
            status_flag = 0;
            break;
    }
//    Motion_Ctrl(V_x, 0, V_z);
    HAL_Delay(10);

//    Motor_Set_Pwm(MOTOR_ID_M1, V_x);
//    Motor_Set_Pwm(MOTOR_ID_M2, V_x);
//    Motor_Set_Pwm(MOTOR_ID_M3, V_x);
//    Motor_Set_Pwm(MOTOR_ID_M4, V_x);
//
//    printf("V_x=%d\r\n", V_x);
//    printf("V_z=%d\r\n", V_z);
}