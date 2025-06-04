#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#include "gpio.h"


#define KEY_PRESS           1
#define KEY_RELEASE         0

#define KEY_MODE_ONE_TIME   1
#define KEY_MODE_ALWAYS     0

typedef struct KEY_STATUS
{
    uint16_t car_state;
    uint16_t Timer_Test_Right;
} key_status_t;

uint8_t Key1_State(uint8_t mode);
void Handle_Press(key_status_t* flag);
void Handle_NoPress(key_status_t* flag2);

#endif /* __BSP_KEY_H__ */
