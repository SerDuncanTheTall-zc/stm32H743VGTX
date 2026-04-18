/**
 ****************************************************************************************************
 * @file        bsp_fill_light.c
 * @author      User (Modified for PA5)
 * @version     V1.1
 * @brief       OV5640 补光灯驱动代码 (PA5)
 ****************************************************************************************************
 */

#include "bsp_fill_light.h"

/**
 * @brief       初始化补光灯 IO 口 (PA5)
 * @param       无
 * @retval      无
 */
void FillLight_Init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    /* 1. 使能 GPIOA 时钟 */
    FILL_LIGHT_GPIO_CLK_ENABLE();

    /* 2. 初始化引脚电平：默认拉低 (确保初始化瞬间灯不会闪烁) */
    HAL_GPIO_WritePin(FILL_LIGHT_GPIO_PORT, FILL_LIGHT_GPIO_PIN, GPIO_PIN_RESET);

    /* 3. 配置引脚参数 */
    gpio_init_struct.Pin   = FILL_LIGHT_GPIO_PIN;            /* PA5 */
    gpio_init_struct.Mode  = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_init_struct.Pull  = GPIO_PULLDOWN;                  /* 开启内部下拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;            /* 低速即可 */
    
    HAL_GPIO_Init(FILL_LIGHT_GPIO_PORT, &gpio_init_struct);  /* 执行初始化 */
    
    /* 4. 再次确认关闭补光灯 */
    FILL_LIGHT_OFF();
}