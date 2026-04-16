/**
 ****************************************************************************************************
 * @file        bsp_fill_light.h
 * @author      User (Modified for PA5)
 * @version     V1.1
 * @brief       OV5640 补光灯驱动代码 (PA5)
 ****************************************************************************************************
 */

#ifndef __BSP_FILL_LIGHT_H
#define __BSP_FILL_LIGHT_H

#include "stm32h7xx_hal.h"

/*------------------------------------------ 补光灯引脚配置 ----------------------------------*/

#define FILL_LIGHT_GPIO_PORT                GPIOA
#define FILL_LIGHT_GPIO_PIN                 GPIO_PIN_5
#define FILL_LIGHT_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)

/*------------------------------------------ 补光灯控制宏 ------------------------------------*/

/**
 * 注意：根据补光灯控制电路原理图
 * 信号线输出 高电平 (1) -> MOSFET 导通 -> 灯亮
 * 信号线输出 低电平 (0) -> MOSFET 截止 -> 灯灭
 */

#define FILL_LIGHT(x)   do{ x ? \
                          HAL_GPIO_WritePin(FILL_LIGHT_GPIO_PORT, FILL_LIGHT_GPIO_PIN, GPIO_PIN_SET) : \
                          HAL_GPIO_WritePin(FILL_LIGHT_GPIO_PORT, FILL_LIGHT_GPIO_PIN, GPIO_PIN_RESET); \
                      }while(0)

#define FILL_LIGHT_ON()      FILL_LIGHT(1)
#define FILL_LIGHT_OFF()     FILL_LIGHT(0)
#define FILL_LIGHT_TOGGLE()  do{ HAL_GPIO_TogglePin(FILL_LIGHT_GPIO_PORT, FILL_LIGHT_GPIO_PIN); }while(0)

/*------------------------------------------ 函数声明 ---------------------------------------*/

void FillLight_Init(void);

#endif