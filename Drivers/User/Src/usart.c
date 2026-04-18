/**
 ******************************************************************************
 * @file    usart.c
 * @author  jojochao
 * @brief   USART1 初始化及 GCC printf 重定向
 ******************************************************************************
 */

#include "usart.h"

UART_HandleTypeDef huart1;

/**
  * @brief  串口硬件底层初始化 (MSP)
  * @param  huart: UART 句柄
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();      // 开启 USART1 时钟
        GPIO_USART1_TX_CLK_ENABLE();        // 开启 TX 引脚时钟
        GPIO_USART1_RX_CLK_ENABLE();        // 开启 RX 引脚时钟

        GPIO_InitStruct.Pin       = USART1_TX_PIN;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(USART1_TX_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = USART1_RX_PIN;
        HAL_GPIO_Init(USART1_RX_PORT, &GPIO_InitStruct);        
    }
}

/**
  * @brief  USART1 初始化配置
  */
void USART1_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = USART1_BaudRate;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        // 这里可以添加错误处理
    }

    /* H7 系列特有的 FIFO 配置，默认关闭以提高调试打印的稳定性 */
    HAL_UARTEx_DisableFifoMode(&huart1);
}

/**
  * @brief  重定向 printf 到串口 (GCC 环境专用)
  * @param  file: 未使用
  * @param  ptr: 要发送的数据指针
  * @param  len: 数据长度
  * @retval 发送的长度
  */
int _write(int file, char *ptr, int len)
{
    /* 使用 HAL 阻塞模式发送，确保调试信息完整输出 */
    /* HAL_MAX_DELAY 保证不会因为超时导致打印中断 */
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* 如果后续需要使用 scanf，可以一并实现 _read */
int _read(int file, char *ptr, int len)
{
    HAL_UART_Receive(&huart1, (uint8_t *)ptr, 1, HAL_MAX_DELAY);
    return 1;
}