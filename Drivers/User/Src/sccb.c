#include "sccb.h"

/**
 * @brief 软件模拟延时
 * H7必须使用 volatile 关键字防止被 O3 优化彻底删掉循环
 */
void SCCB_Delay(void)
{
    volatile uint32_t i = SCCB_DelayVaule;
    while(i--);
}

/**
 * @brief 初始化 SCCB 使用的 GPIO (开漏模式)
 */
void SCCB_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    SCCB_SCL_CLK_ENABLE();
    SCCB_SDA_CLK_ENABLE();

    /* SCL & SDA 均设为开漏输出 + 内部上拉 */
    /* 注意：只有开漏模式才能支持双向通信且不会产生电平冲突 */
    GPIO_InitStruct.Pin    = SCCB_SCL_PIN;
    GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_OD; 
    GPIO_InitStruct.Pull   = GPIO_PULLUP;
    GPIO_InitStruct.Speed  = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SCCB_SCL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin    = SCCB_SDA_PIN;
    HAL_GPIO_Init(SCCB_SDA_PORT, &GPIO_InitStruct);

    SCCB_SCL(1);
    SCCB_SDA(1);
    SCCB_Delay();
}

void SCCB_Start(void)
{
    SCCB_SDA(1);
    SCCB_SCL(1);
    SCCB_Delay();
    SCCB_SDA(0);
    SCCB_Delay();
    SCCB_SCL(0);
    SCCB_Delay();
}

void SCCB_Stop(void)
{
    SCCB_SCL(0);
    SCCB_Delay();
    SCCB_SDA(0);
    SCCB_Delay();
    SCCB_SCL(1);
    SCCB_Delay();
    SCCB_SDA(1);
    SCCB_Delay();
}

void SCCB_NoACK(void)
{
    SCCB_SDA(1);
    SCCB_Delay();
    SCCB_SCL(1);
    SCCB_Delay();
    SCCB_SCL(0);
    SCCB_Delay();
}

/**
 * @brief 写入一个字节并返回应答位
 * @return 0: 成功(ACK), 1: 失败(NACK)
 */
uint8_t SCCB_WriteByte(uint8_t data)
{
    uint8_t i, ack;
    for(i = 0; i < 8; i++)
    {
        if(data & 0x80) SCCB_SDA(1);
        else            SCCB_SDA(0);
        SCCB_Delay();
        SCCB_SCL(1);
        SCCB_Delay();
        SCCB_SCL(0);
        data <<= 1;
    }
    
    /* 第9位：释放SDA，读取从机应答 */
    SCCB_SDA(1); 
    SCCB_Delay();
    SCCB_SCL(1);
    SCCB_Delay();
    if(READ_SDA) ack = 1; // NACK
    else         ack = 0; // ACK
    SCCB_SCL(0);
    SCCB_Delay();
    return ack;
}

uint8_t SCCB_ReadByte(void)
{
    uint8_t i, data = 0;
    SCCB_SDA(1); // 释放总线
    for(i = 0; i < 8; i++)
    {
        data <<= 1;
        SCCB_SCL(1);
        SCCB_Delay();
        if(READ_SDA) data++;
        SCCB_SCL(0);
        SCCB_Delay();
    }
    return data;
}

/* --- OV5640 专用 16位地址写 --- */
uint8_t SCCB_WriteReg_16Bit(uint16_t addr, uint8_t value)
{
    SCCB_Start();
    if(SCCB_WriteByte(OV5640_DEVICE_ADDRESS) != 0) { SCCB_Stop(); return ERROR; }
    if(SCCB_WriteByte(addr >> 8) != 0)              { SCCB_Stop(); return ERROR; }
    if(SCCB_WriteByte(addr & 0xFF) != 0)            { SCCB_Stop(); return ERROR; }
    if(SCCB_WriteByte(value) != 0)                  { SCCB_Stop(); return ERROR; }
    SCCB_Stop();
    return SUCCESS;
}

/* --- OV5640 专用 16位地址读 --- */
uint8_t SCCB_ReadReg_16Bit(uint16_t addr)
{
    uint8_t val;
    // 1. 写周期：指定寄存器地址
    SCCB_Start();
    if(SCCB_WriteByte(OV5640_DEVICE_ADDRESS) != 0) { SCCB_Stop(); return 0; }
    if(SCCB_WriteByte(addr >> 8) != 0)              { SCCB_Stop(); return 0; }
    if(SCCB_WriteByte(addr & 0xFF) != 0)            { SCCB_Stop(); return 0; }
    SCCB_Stop();

    SCCB_Delay(); // 间歇

    // 2. 读周期：获取数据
    SCCB_Start(); 
    if(SCCB_WriteByte(OV5640_DEVICE_ADDRESS | 0x01) != 0) { SCCB_Stop(); return 0; }
    val = SCCB_ReadByte();
    SCCB_NoACK(); // 读完一个字节必须发 NACK
    SCCB_Stop();
    return val;
}

/* --- 批量写入函数 --- */
uint8_t SCCB_WriteBuffer_16Bit(uint16_t addr, uint8_t *pData, uint32_t size)
{
    SCCB_Start();
    if(SCCB_WriteByte(OV5640_DEVICE_ADDRESS) != 0) { SCCB_Stop(); return ERROR; }
    if(SCCB_WriteByte(addr >> 8) != 0)              { SCCB_Stop(); return ERROR; }
    if(SCCB_WriteByte(addr & 0xFF) != 0)            { SCCB_Stop(); return ERROR; }
    for(uint32_t i = 0; i < size; i++)
    {
        if(SCCB_WriteByte(pData[i]) != 0)           { SCCB_Stop(); return ERROR; }
    }
    SCCB_Stop();
    return SUCCESS;
}