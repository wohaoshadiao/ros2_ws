/*
 * bsp_uart.c
 *
 *  Created on: Mar 4, 2022
 *      Author: Administrator
 */

#include "bsp_uart.h"
#include "bsp.h"
#include <stdio.h>
#include <string.h>

#define ENABLE_UART_DMA    1
#define RX_BUFFER_SIZE 100

char rxBuffer[RX_BUFFER_SIZE];
uint16_t rxIndex = 0;
float linear_x_value = 0.0f;
float angular_z_value = 0.0f;
uint8_t RxTemp = 0;


// Initialize USART1  初始化串口1
void USART1_Init(void)
{
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&RxTemp, 1);     // 获取到一字节，进入串口中断

//    printf("start serial\n");
}

// The serial port sends one byte  串口发送一个字节
void USART1_Send_U8(uint8_t ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
}

// The serial port sends a string of data  串口发送一串数据
void USART1_Send_ArrayU8(uint8_t *BufferPtr, uint16_t Length)
{
    #if ENABLE_UART_DMA
    HAL_UART_Transmit_DMA(&huart1, BufferPtr, Length);
    #else
    while (Length--)
    {
        USART1_Send_U8(*BufferPtr);
        BufferPtr++;
    }
    #endif
}

// 串口接收完成中断
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

    if (huart == &huart1)
    {
        uint8_t received_char = RxTemp; // 上次接收到的字符
        if (received_char != '\n' && rxIndex < RX_BUFFER_SIZE - 1)
        {
            rxBuffer[rxIndex++] = received_char; // 存储字符
            // 继续接收下一个字符
            HAL_UART_Receive_IT(&huart1, (uint8_t *)&RxTemp, 1);
        }
        else
        {
            // 完整一行数据接收完毕
            rxBuffer[rxIndex] = '\0'; // 添加字符串结束符
            rxIndex = 0; // 重置缓冲区索引

            // 解析数据
            parse_uart_data(rxBuffer);

            // 继续接收下一行
            HAL_UART_Receive_IT(&huart1, (uint8_t *)&RxTemp, 1);
        }
    }

}


void parse_uart_data(const char* data)
{
    // 样本字符串：
    // linear_x:0.00,linear_y:0.00,linear_z:0.00,angular_x:0.00,angular_y:0.00,angular_z:0.50

    // 使用 sscanf 或 strstr 提取
    char *ptr;

    // 提取 linear_x
    ptr = strstr(data, "linear_x:");
    if (ptr != NULL)
    {
        // 转到数字部分
        float temp;
        if (sscanf(ptr, "linear_x:%f", &temp))
        {
            linear_x_value = temp;
//            printf("linear_x_value: %f\r\n", linear_x_value);
        }
    }

    // 提取 angular_z
    ptr = strstr(data, "angular_z:");
    if (ptr != NULL)
    {
        float temp;
        if (sscanf(ptr, "angular_z:%f", &temp))
        {
            angular_z_value = temp;
//            printf("angular_z_value: %f\r\n", angular_z_value);
        }
    }

}



#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
