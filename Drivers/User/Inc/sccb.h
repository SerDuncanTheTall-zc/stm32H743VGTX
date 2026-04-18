#ifndef __CAMERA_SCCB_H
#define __CAMERA_SCCB_H

#include "stm32h7xx_hal.h"

/* 状态定义 */
#ifndef SUCCESS
#define SUCCESS      1
#endif
#ifndef ERROR
#define ERROR        0
#endif

#define OV5640_DEVICE_ADDRESS     0x78

/*----------------------------------------- 引脚配置宏 (PB8/PB9) -----------------------------------*/

#define SCCB_SCL_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SCCB_SCL_PORT              GPIOB
#define SCCB_SCL_PIN               GPIO_PIN_8
        
#define SCCB_SDA_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SCCB_SDA_PORT              GPIOB
#define SCCB_SDA_PIN               GPIO_PIN_9

/*------------------------------------------ IIC相关定义 ------------------------------------------*/

#define SCCB_DelayVaule  500 // H7主频480MHz，初始设大点确保稳定

/*-------------------------------------------- IO口操作 -------------------------------------------*/   

#define SCCB_SCL(n) (n ? (SCCB_SCL_PORT->BSRR = SCCB_SCL_PIN) : (SCCB_SCL_PORT->BSRR = (uint32_t)SCCB_SCL_PIN << 16U))
#define SCCB_SDA(n) (n ? (SCCB_SDA_PORT->BSRR = SCCB_SDA_PIN) : (SCCB_SDA_PORT->BSRR = (uint32_t)SCCB_SDA_PIN << 16U))
#define READ_SDA    ((SCCB_SDA_PORT->IDR & SCCB_SDA_PIN) != 0)

/*------------------------------------------- 函数声明 --------------------------------------------*/               

void    SCCB_GPIO_Config(void);
void    SCCB_Start(void);
void    SCCB_Stop(void);
uint8_t SCCB_WriteByte(uint8_t data);
uint8_t SCCB_ReadByte(void);
void    SCCB_NoACK(void);

uint8_t SCCB_WriteReg_16Bit(uint16_t addr, uint8_t value);
uint8_t SCCB_ReadReg_16Bit(uint16_t addr);
uint8_t SCCB_WriteBuffer_16Bit(uint16_t addr, uint8_t *pData, uint32_t size);

#endif