
#include "bsp_sbus.h"
#include "bsp.h"
#include "string.h"


#define SBUS_RECV_MAX    25
#define SBUS_START       0x0F
#define SBUS_END         0x00

// Parameters related to receiving data  接收数据相关参数
uint8_t sbus_start = 0;
uint8_t sbus_buf_index = 0;
uint8_t sbus_new_cmd = 0;

// data-caching mechanism  数据缓存
uint8_t inBuffer[SBUS_RECV_MAX] = {0};
uint8_t failsafe_status = SBUS_SIGNAL_FAILSAFE;

uint8_t sbus_data[SBUS_RECV_MAX] = {0};
int16_t g_sbus_channels[18] = {0};



// Parses SBUS data into channel values  解析SBUS的数据，转化成通道数值。
static int SBUS_Parse_Data(void)
{
    g_sbus_channels[0]  = ((sbus_data[1] | sbus_data[2] << 8) & 0x07FF);
    g_sbus_channels[1]  = ((sbus_data[2] >> 3 | sbus_data[3] << 5) & 0x07FF);
    g_sbus_channels[2]  = ((sbus_data[3] >> 6 | sbus_data[4] << 2 | sbus_data[5] << 10) & 0x07FF);
    g_sbus_channels[3]  = ((sbus_data[5] >> 1 | sbus_data[6] << 7) & 0x07FF);
    g_sbus_channels[4]  = ((sbus_data[6] >> 4 | sbus_data[7] << 4) & 0x07FF);
    g_sbus_channels[5]  = ((sbus_data[7] >> 7 | sbus_data[8] << 1 | sbus_data[9] << 9) & 0x07FF);
    g_sbus_channels[6]  = ((sbus_data[9] >> 2 | sbus_data[10] << 6) & 0x07FF);
    g_sbus_channels[7]  = ((sbus_data[10] >> 5 | sbus_data[11] << 3) & 0x07FF);
    #ifdef ALL_CHANNELS
    g_sbus_channels[8]  = ((sbus_data[12] | sbus_data[13] << 8) & 0x07FF);
    g_sbus_channels[9]  = ((sbus_data[13] >> 3 | sbus_data[14] << 5) & 0x07FF);
    g_sbus_channels[10] = ((sbus_data[14] >> 6 | sbus_data[15] << 2 | sbus_data[16] << 10) & 0x07FF);
    g_sbus_channels[11] = ((sbus_data[16] >> 1 | sbus_data[17] << 7) & 0x07FF);
    g_sbus_channels[12] = ((sbus_data[17] >> 4 | sbus_data[18] << 4) & 0x07FF);
    g_sbus_channels[13] = ((sbus_data[18] >> 7 | sbus_data[19] << 1 | sbus_data[20] << 9) & 0x07FF);
    g_sbus_channels[14] = ((sbus_data[20] >> 2 | sbus_data[21] << 6) & 0x07FF);
    g_sbus_channels[15] = ((sbus_data[21] >> 5 | sbus_data[22] << 3) & 0x07FF);
    #endif

    // 安全检测，检测是否失联或者数据错误
    // Security detection to check for lost connections or data errors
    failsafe_status = SBUS_SIGNAL_OK;
    if (sbus_data[23] & (1 << 2))
    {
        failsafe_status = SBUS_SIGNAL_LOST;
        printf("SBUS_SIGNAL_LOST\n");
        // lost contact errors  遥控器失联错误
    }
    else if (sbus_data[23] & (1 << 3))
    {
        failsafe_status = SBUS_SIGNAL_FAILSAFE;
        printf("SBUS_SIGNAL_FAILSAFE\n");
        // data loss error  数据丢失错误
    }
    return failsafe_status;
}

// Receives SBUS cache data  接收SBUS的缓存数据
void SBUS_Reveive(uint8_t data)
{
    // 如果符合协议开始标志，则开始接收数据
    if (sbus_start == 0 && data == SBUS_START)
    {
        sbus_start = 1;
        sbus_new_cmd = 0;
        sbus_buf_index = 0;
        inBuffer[sbus_buf_index] = data;          // 起始位
        inBuffer[SBUS_RECV_MAX - 1] = 0xff;       // 接收一帧数据后，判断停止位是否正确
    }
    else if (sbus_start)
    {
        sbus_buf_index++;
        inBuffer[sbus_buf_index] = data;
    }

    // Finish receiving a frame of data  完成接收一帧数据
    if (sbus_start && (sbus_buf_index >= (SBUS_RECV_MAX - 1)))
    {
        sbus_start = 0;
        if (inBuffer[SBUS_RECV_MAX - 1] == SBUS_END)
        {
            memcpy(sbus_data, inBuffer, SBUS_RECV_MAX);
            sbus_new_cmd = 1;
        }
    }
}

//  SBUS接收处理数据句柄
void SBUS_Handle(void)
{
    if (sbus_new_cmd)
    {
        int res = SBUS_Parse_Data();
        sbus_new_cmd = 0;
        if (res) return;
        #if SBUS_ALL_CHANNELS
        printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
               g_sbus_channels[0], g_sbus_channels[1], g_sbus_channels[2],
			   g_sbus_channels[3], g_sbus_channels[4], g_sbus_channels[5],
			   g_sbus_channels[6], g_sbus_channels[7], g_sbus_channels[8],
			   g_sbus_channels[9], g_sbus_channels[10], g_sbus_channels[11],
			   g_sbus_channels[12], g_sbus_channels[13], g_sbus_channels[14],
			   g_sbus_channels[15]);
        #else
        printf("%d,%d,%d,%d,%d,%d,%d,%d\r\n",
               g_sbus_channels[0], g_sbus_channels[1], g_sbus_channels[2],
			   g_sbus_channels[3], g_sbus_channels[4],g_sbus_channels[5],
			   g_sbus_channels[6], g_sbus_channels[7]);
        #endif
    }
}


// SBUS协议转化为PWM，传入Motor控制函数
// The SBUS protocol is converted to PWM and passed to the motor control function
void SBUS_Handle_Manual(void)
{
    if (sbus_new_cmd)
    {
        int res = SBUS_Parse_Data();
        sbus_new_cmd = 0;
        if (res) return;
#if SBUS_ALL_CHANNELS           // 条件编译，灵活调度空间.
        printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
               g_sbus_channels[0], g_sbus_channels[1], g_sbus_channels[2],
			   g_sbus_channels[3], g_sbus_channels[4], g_sbus_channels[5],
			   g_sbus_channels[6], g_sbus_channels[7], g_sbus_channels[8],
			   g_sbus_channels[9], g_sbus_channels[10], g_sbus_channels[11],
			   g_sbus_channels[12], g_sbus_channels[13], g_sbus_channels[14],
			   g_sbus_channels[15]);
#else

        int16_t Pitch = g_sbus_channels[1];
        int16_t Roll  = g_sbus_channels[0];
        int16_t Energy = g_sbus_channels[2];
        int16_t Yaw = g_sbus_channels[3];
        uint16_t Mode = g_sbus_channels[4];

        // M2:Mode2 M1:Mode1 M3:Mode3 M4:Mode4
        static uint8_t M1_flag = 0,M2_flag = 0,M3_flag = 0,M4_flag_value = 0; int speed = 0;

        if (Mode != 1000)
        {
            Motor_Stop(1);
        }
        else
        {
            // M1_flag： 1 -> 前进     2 -> 后退
            if(Pitch>=800 && Pitch <= 1000)     M1_flag = 0;
            else if(Pitch >= 200 && Pitch <= 799)   M1_flag = 1;
            else if(Pitch <= 1800 && Pitch >= 1001)     M1_flag = 2;
            // M2_flag： 1 -> 左移     2 -> 右移
            if(Roll>=900 && Roll <= 1150)     M2_flag = 0;
            else if(Roll >= 200 && Roll <= 799)   M2_flag = 1;
            else if(Roll <= 1800 && Roll >= 1001)     M2_flag = 2;
            // M3_flag:  1 -> 左旋    2-> 右旋
            if(Yaw>=850 && Yaw <= 1150)     M3_flag = 0;
            else if(Yaw >= 200 && Yaw <= 799)   M3_flag = 1;
            else if(Yaw <= 1800 && Yaw >= 1001)     M3_flag = 2;
            // 右平移

        }

        if(M1_flag == 0 && M2_flag == 0 && M3_flag == 0)
        {
            Motor_Stop(1);
        }
        // 前进
        else if(M1_flag == 1 )
        {
            speed = -2000;
            int16_t against_speed = -speed;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, speed);
            Motor_Set_Pwm(MOTOR_ID_M3, speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
        }
        // 后退
        else if(M1_flag == 2 )
        {
            speed = 2000;
            int16_t against_speed = -speed;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, speed);
            Motor_Set_Pwm(MOTOR_ID_M3, speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
        }
        // 左移
        else if(M2_flag == 1 )
        {
            speed = 2000;
            int16_t against_speed = -speed;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, against_speed);
            Motor_Set_Pwm(MOTOR_ID_M3, against_speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
        }
        // 右移
        else if(M2_flag == 2 )
        {
            speed = -2000;
            int16_t against_Two_speed = -speed;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, against_Two_speed);
            Motor_Set_Pwm(MOTOR_ID_M3, against_Two_speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
        }
        else if(M3_flag == 1 )
        {
            speed = 2000;
            int16_t against_Two_speed = -speed;
            Motor_Set_Pwm(MOTOR_ID_M1, speed);
            Motor_Set_Pwm(MOTOR_ID_M2, speed);
            Motor_Set_Pwm(MOTOR_ID_M3, against_Two_speed);
            Motor_Set_Pwm(MOTOR_ID_M4, against_Two_speed);
        }
        else if(M3_flag == 2 )
        {
            speed = 2000;
            int16_t against_Two_speed = -speed;
            Motor_Set_Pwm(MOTOR_ID_M1, against_Two_speed);
            Motor_Set_Pwm(MOTOR_ID_M2, against_Two_speed);
            Motor_Set_Pwm(MOTOR_ID_M3, speed);
            Motor_Set_Pwm(MOTOR_ID_M4, speed);
        }

#endif
    }

}