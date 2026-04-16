/**
 ****************************************************************************************************
 * @file        bsp_fill_light.c
 * @author      User (Inspired by ALIENTEK)
 * @version     V1.0
 * @brief       OV5640 补光灯驱动代码 (PC4)
 ****************************************************************************************************
 */

#include "bsp_fill_light.h"

/**
 * @brief       初始化补光灯 IO 口
 * @param       无
 * @retval      无
 */
void FillLight_Init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    /* 1. 使能 GPIOC 时钟 */
    FILL_LIGHT_GPIO_CLK_ENABLE();

    /* 2. 初始化 PC4 状态：默认拉低，确保开机时灯是灭的 */
    HAL_GPIO_WritePin(FILL_LIGHT_GPIO_PORT, FILL_LIGHT_GPIO_PIN, GPIO_PIN_RESET);

    /* 3. 配置引脚参数 */
    gpio_init_struct.Pin   = FILL_LIGHT_GPIO_PIN;            /* PC4 */
    gpio_init_struct.Mode  = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_init_struct.Pull  = GPIO_PULLDOWN;                  /* 强制下拉保持稳定 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;           /* 补光灯开关不需要高速 */
    
    //HAL_GPIO_Init(FILL_LIGHT_GPIO_PORT, &gpio_init_struct);  /* 初始化 */
    
    /* 4. 二次确认关闭补光灯 */
    FILL_LIGHT_OFF();
}