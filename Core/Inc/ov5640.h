#ifndef __OV5640_H
#define __OV5640_H

#include "main.h"

/* OV5640 I2C 设备地址 */
#define OV5640_ADDR_WRITE  0x78
#define OV5640_ADDR_READ   0x79

/* 核心测试寄存器：设备 ID */
#define OV5640_CHIPID_H    0x300A
#define OV5640_CHIPID_L    0x300B

/* 函数声明 */
uint8_t  OV5640_ReadReg(uint16_t reg);
void     OV5640_WriteReg(uint16_t reg, uint8_t data);
void     OV5640_HW_Reset(void);
uint16_t OV5640_ReadID(void);

#endif /* __OV5640_H */
