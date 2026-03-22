#include "ov5640.h"
#include "i2c.h"        // 引入 CubeMX 生成的 hi2c1
#include "tx_api.h"     // 引入 ThreadX，用于高效延时

// 引入外部的 hi2c1 句柄
extern I2C_HandleTypeDef hi2c1;

/**
  * @brief  向 OV5640 写入一个 8 位数据到 16 位地址的寄存器
  * @param  reg: 16位寄存器地址 (例如 0x300A)
  * @param  data: 8位要写入的数据
  */
void OV5640_WriteReg(uint16_t reg, uint8_t data)
{
    // H7 的 I2C 速度极快，超时时间给 100ms 足够了
    HAL_I2C_Mem_Write(&hi2c1, OV5640_ADDR_WRITE, reg, I2C_MEMADD_SIZE_16BIT, &data, 1, 100);
}

/**
  * @brief  从 OV5640 的 16 位地址寄存器读取一个 8 位数据
  * @param  reg: 16位寄存器地址
  * @retval 读取到的 8 位数据
  */
uint8_t OV5640_ReadReg(uint16_t reg)
{
    uint8_t data = 0;
    HAL_I2C_Mem_Read(&hi2c1, OV5640_ADDR_READ, reg, I2C_MEMADD_SIZE_16BIT, &data, 1, 100);
    return data;
}

/**
  * @brief  OV5640 硬件复位与上电序列 (非常重要！)
  * @note   该函数必须在开启了 TIM2 PWM 时钟之后调用
  */
void OV5640_HW_Reset(void)
{
    /* 1. 先将 PWDN 拉高，让摄像头进入掉电模式，确保初始状态确定 */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
    tx_thread_sleep(10); // 延时等待稳定 (ThreadX 下 10 个 tick)

    /* 2. 将 PWDN 拉低，唤醒摄像头 (正常工作模式) */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    tx_thread_sleep(10);

    /* 3. 如果你的板子接了硬件 RESET 引脚 (假设是 PC4)，执行一次硬件复位 */
    // 如果你没接 RESET 引脚，把下面三行注释掉即可
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); // 拉低复位
    // tx_thread_sleep(10);
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);   // 拉高结束复位

    /* 4. 等待内部 LDO 和 PLL 稳定，OV5640 手册要求至少 20ms */
    tx_thread_sleep(30);
}

/**
  * @brief  读取 OV5640 的芯片 ID
  * @retval 16位的芯片 ID，正常应返回 0x5640
  */
uint16_t OV5640_ReadID(void)
{
    uint8_t id_h, id_l;

    id_h = OV5640_ReadReg(OV5640_CHIPID_H); // 读 0x300A
    id_l = OV5640_ReadReg(OV5640_CHIPID_L); // 读 0x300B

    return ((uint16_t)id_h << 8) | id_l;
}
