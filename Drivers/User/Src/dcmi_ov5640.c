/***

>>>>> 驱动说明：
    *
    * 1.例程默认配置 OV5640  为 4:3(1280*960) 43帧 的配置（JPG模式2、3情况下帧率会减半）
    * 2.开启了DMA并使能了中断，移植的时候需要移植对应的中断
    *
    *************************************************************************************************************************************************************************************************************************************************************************************LXB*****
***/

#include "dcmi_ov5640.h"  
#include "dcmi_ov5640_cfg.h"  
#include <stdio.h> // Include standard I/O for debugging

DCMI_HandleTypeDef   hdcmi;            // DCMI句柄
DMA_HandleTypeDef    DMA_Handle_dcmi;  // DMA句柄

volatile uint8_t OV5640_FrameState = 0;  // DCMI状态标志，当数据帧传输完成时，会被 HAL_DCMI_FrameEventCallback() 中断回调函数置 1    
volatile uint8_t OV5640_FPS ;          // 帧率

/*****************************************************************************************************************************************
* 函 数 名:   HAL_DCMI_MspInit
*
* 入口参数:   hdcmi - DCMI_HandleTypeDef定义的变量，即表示定义的 DCMI 句柄
*
* 函数功能:   初始化 DCMI 引脚
*
*****************************************************************************************************************************************/
void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   if(hdcmi->Instance==DCMI)
   {
        __HAL_RCC_DCMI_CLK_ENABLE();        // 使能 DCMI 外设时钟

        __HAL_RCC_GPIOE_CLK_ENABLE();// 使能相应的GPIO时钟
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        GPIO_OV5640_PWDN_CLK_ENABLE;    // 使能PWDN 引脚的 GPIO 时钟

        GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_5|GPIO_PIN_4
                                  |GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        OV5640_PWDN_OFF;    // 高电平，进入掉电模式，摄像头停止工作，此时功耗降到最低

        GPIO_InitStruct.Pin         = OV5640_PWDN_PIN;              // PWDN 引脚
        GPIO_InitStruct.Mode    = GPIO_MODE_OUTPUT_PP;          // 推挽输出模式
        GPIO_InitStruct.Pull    = GPIO_PULLUP;                      // 上拉
        GPIO_InitStruct.Speed   = GPIO_SPEED_FREQ_LOW;          // 速度等级低
        HAL_GPIO_Init(OV5640_PWDN_PORT, &GPIO_InitStruct);     // 初始化  
    }
}

/***************************************************************************************************************************************
* 函 数 名: MX_DCMI_Init
*
* 函数功能: 配置DCMI相关参数
*
* 说    明: 8位数据模式，全数据、全帧捕捉，开启中断                             
*
*****************************************************************************************************************************************/
void MX_DCMI_Init(void)
{
   hdcmi.Instance                = DCMI;
   hdcmi.Init.SynchroMode        = DCMI_SYNCHRO_HARDWARE;      // 硬件同步模式，即使用外部的VS、HS信号进行同步
   hdcmi.Init.PCKPolarity        = DCMI_PCKPOLARITY_RISING;    // 像素时钟上升沿有效
   hdcmi.Init.VSPolarity         = DCMI_VSPOLARITY_LOW;        // VS低电平有效
   hdcmi.Init.HSPolarity         = DCMI_HSPOLARITY_LOW;        // HS低电平有效
   hdcmi.Init.CaptureRate        = DCMI_CR_ALL_FRAME;          // 捕获等级，设置每一帧都进行捕获
   hdcmi.Init.ExtendedDataMode   = DCMI_EXTEND_DATA_8B;        // 8位数据模式
   hdcmi.Init.JPEGMode           = DCMI_JPEG_DISABLE;           // 不使用DCMI的JPEG模式
   hdcmi.Init.ByteSelectMode     = DCMI_BSM_ALL;               // DCMI接口捕捉所有数据  
   hdcmi.Init.ByteSelectStart    = DCMI_OEBS_ODD;              // 选择开始字节，从 帧/行 的第一个数据开始捕获
   hdcmi.Init.LineSelectMode     = DCMI_LSM_ALL;               // 捕获所有行
   hdcmi.Init.LineSelectStart    = DCMI_OELS_ODD;              // 选择开始行,在帧开始后捕获第一行
   HAL_DCMI_Init(&hdcmi) ;

   HAL_NVIC_SetPriority(DCMI_IRQn, 0 ,5);    // 设置中断优先级
   HAL_NVIC_EnableIRQ(DCMI_IRQn);             // 开启DCMI中断
   
   printf("[DEBUG] DCMI Initialized.\r\n");
}

/***************************************************************************************************************************************
* 函 数 名: OV5640_DMA_Init
*
* 函数功能: 配置 DMA 相关参数
*
* 说    明: 使用的是DMA2，外设到存储器模式、数据位宽32bit、并开启中断                       
*
*****************************************************************************************************************************************/
void OV5640_DMA_Init(void)
{
   __HAL_RCC_DMA2_CLK_ENABLE();   // 使能DMA2时钟

   DMA_Handle_dcmi.Instance                     = DMA2_Stream7;               // DMA2数据流7      
   DMA_Handle_dcmi.Init.Request                 = DMA_REQUEST_DCMI;           // DMA请求来自DCMI
   DMA_Handle_dcmi.Init.Direction               = DMA_PERIPH_TO_MEMORY;       // 外设到存储器模式
   DMA_Handle_dcmi.Init.PeriphInc               = DMA_PINC_DISABLE;           // 外设地址禁止自增
   DMA_Handle_dcmi.Init.MemInc                  = DMA_MINC_ENABLE;             // 存储器地址自增
   DMA_Handle_dcmi.Init.PeriphDataAlignment     = DMA_PDATAALIGN_WORD;        // DCMI数据位宽，32位  
   DMA_Handle_dcmi.Init.MemDataAlignment        = DMA_MDATAALIGN_WORD;        // 存储器数据位宽，32位
   DMA_Handle_dcmi.Init.Mode                    = DMA_CIRCULAR;               // 循环模式                   
   DMA_Handle_dcmi.Init.Priority                = DMA_PRIORITY_LOW;           // 优先级低
   DMA_Handle_dcmi.Init.FIFOMode                = DMA_FIFOMODE_ENABLE;        // 使能fifo
   DMA_Handle_dcmi.Init.FIFOThreshold           = DMA_FIFO_THRESHOLD_FULL;    // 全fifo模式，4*32bit大小
   DMA_Handle_dcmi.Init.MemBurst                = DMA_MBURST_SINGLE;          // 单次传输
   DMA_Handle_dcmi.Init.PeriphBurst             = DMA_PBURST_SINGLE;          // 单次传输

   HAL_DMA_Init(&DMA_Handle_dcmi);                        // 配置DMA
   __HAL_LINKDMA(&hdcmi, DMA_Handle, DMA_Handle_dcmi);    // 关联DCMI句柄
    
   HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);         // 设置中断优先级
   HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);                 // 使能中断
   
   printf("[DEBUG] DMA2_Stream7 Initialized.\r\n");
}

/***************************************************************************************************************************************
* 函 数 名: OV5640_Delay
*****************************************************************************************************************************************/
void OV5640_Delay(uint32_t Delay)
{
    volatile uint16_t i;
    while (Delay --)                
    {
        for (i = 0; i < 40000; i++);
    }   
}

/***************************************************************************************************************************************
* 函 数 名: DCMI_OV5640_Init
* 函数功能: 初始SCCB、DCMI、DMA以及配置OV5640
*****************************************************************************************************************************************/
int8_t DCMI_OV5640_Init(void)
{
    uint16_t    Device_ID;      // 定义变量存储器件ID
    
   SCCB_GPIO_Config();                     // SCCB引脚初始化
    MX_DCMI_Init();                        // 初始化DCMI配置引脚
   OV5640_DMA_Init();                     // 初始化DMA配置
    OV5640_Reset();                      // 执行软件复位
    Device_ID =  OV5640_ReadID();             // 读取器件ID

    if( Device_ID == 0x5640 )       // 进行匹配
    {
        printf ("OV5640 OK, ID:0x%X\r\n",Device_ID);           // 匹配通过

        OV5640_Config();                                                                    // 配置各项参数
        OV5640_Set_Framesize(OV5640_Width,OV5640_Height);       //  设置OV5640输出的图像大小
        OV5640_DCMI_Crop( Display_Width, Display_Height, OV5640_Width, OV5640_Height ); // 将输出图像裁剪成适应屏幕的大小，JPG模式不需要裁剪
                
        return OV5640_Success;   // 返回成功标志        
    }
    else
    {
        printf ("OV5640 ERROR!!!!!  ID:0x%X\r\n",Device_ID);     // 读取ID错误
        return  OV5640_Error;    // 返回错误标志
    }   
}

/***************************************************************************************************************************************
* 函 数 名: OV5640_DMA_Transmit_Continuous
*****************************************************************************************************************************************/
void OV5640_DMA_Transmit_Continuous(uint32_t DMA_Buffer,uint32_t DMA_BufferSize)
{
   DMA_Handle_dcmi.Init.Mode  = DMA_CIRCULAR;  // 循环模式                  
   HAL_DMA_Init(&DMA_Handle_dcmi);    // 配置DMA
   printf("[DEBUG] Starting DCMI DMA Continuous Transfer. Buffer: 0x%X\r\n", DMA_Buffer);
   HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)DMA_Buffer,DMA_BufferSize);
}

/***************************************************************************************************************************************
* 函 数 名: OV5640_DMA_Transmit_Snapshot
*****************************************************************************************************************************************/
void OV5640_DMA_Transmit_Snapshot(uint32_t DMA_Buffer,uint32_t DMA_BufferSize)
{
   DMA_Handle_dcmi.Init.Mode  = DMA_NORMAL;  // 正常模式                    
   HAL_DMA_Init(&DMA_Handle_dcmi);    // 配置DMA
   HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)DMA_Buffer,DMA_BufferSize);
}

void OV5640_DCMI_Suspend(void) 
{
   HAL_DCMI_Suspend(&hdcmi);    // 挂起DCMI
}

void  OV5640_DCMI_Resume(void) 
{
   (&hdcmi)->State = HAL_DCMI_STATE_BUSY;       // 变更DCMI标志
   (&hdcmi)->Instance->CR |= DCMI_CR_CAPTURE;   // 开启DCMI捕获
}

void  OV5640_DCMI_Stop(void) 
{
   HAL_DCMI_Stop(&hdcmi);
}

int8_t OV5640_DCMI_Crop(uint16_t Displey_XSize,uint16_t Displey_YSize,uint16_t Sensor_XSize,uint16_t Sensor_YSize )
{
    uint16_t DCMI_X_Offset,DCMI_Y_Offset;   
    uint16_t DCMI_CAPCNT;       
    uint16_t DCMI_VLINE;            

    if( (Displey_XSize>=Sensor_XSize)|| (Displey_YSize>=Sensor_YSize) )
    {
        return OV5640_Error; 
    }
    DCMI_X_Offset = Sensor_XSize - Displey_XSize; 
    DCMI_Y_Offset = (Sensor_YSize - Displey_YSize)/2-1; 
    DCMI_CAPCNT = Displey_XSize*2-1;    
    DCMI_VLINE = Displey_YSize-1;       
    
    HAL_DCMI_ConfigCrop (&hdcmi,DCMI_X_Offset,DCMI_Y_Offset,DCMI_CAPCNT,DCMI_VLINE);// 设置裁剪窗口
    HAL_DCMI_EnableCrop(&hdcmi);        // 使能裁剪
    
    return OV5640_Success;
}

void OV5640_Reset(void)
{
    OV5640_Delay(30);  
    OV5640_PWDN_ON;  
    OV5640_Delay(5);    
    OV5640_Delay(20);    
    SCCB_WriteReg_16Bit(0x3103, 0x11);  
    SCCB_WriteReg_16Bit(0x3008, 0x82);  
    OV5640_Delay(5);  
}

uint16_t OV5640_ReadID(void)
{
   uint8_t PID_H,PID_L;     
   PID_H = SCCB_ReadReg_16Bit(OV5640_ChipID_H); 
   PID_L = SCCB_ReadReg_16Bit(OV5640_ChipID_L); 
    return(PID_H<<8)|PID_L; 
}

/***************************************************************************************************************************************
* 函 数 名: OV5640_Config
* 函数功能: 配置 OV5640 各个寄存器参数，并关闭闪光灯
*****************************************************************************************************************************************/
/**
 * @brief       配置 OV5640 寄存器
 * @note        在初始化循环结束后，强制关闭补光灯信号，防止硬件发烫且不干扰图像传输
 * @param       无
 * @retval      无
 */
/**
 * @brief       配置 OV5640 寄存器
 * @note        在执行完标准初始化后，强制关闭补光灯信号 (STROBE)
 * 使用的 SCCB 函数必须是优化后修复了 Start 信号冲突的版本
 */
void OV5640_Config(void)
{
    uint32_t i; 
    uint8_t read_reg; 
    uint8_t temp_val;

    /* 1. 执行商家提供的标准初始化序列 */
    printf("[System] Starting OV5640 Register Configuration...\r\n");
    for(i=0; i<(sizeof(OV5640_INIT_Config)/4); i++)
    {
        SCCB_WriteReg_16Bit(OV5640_INIT_Config[i][0], OV5640_INIT_Config[i][1]); 
        
        /* 调试用：读取校验，确保寄存器真的写进去了 */
        read_reg = SCCB_ReadReg_16Bit(OV5640_INIT_Config[i][0]);    
        if(OV5640_INIT_Config[i][1] != read_reg )   
        {
            printf("Config Error at index: %d | Reg: 0x%X | Target: 0x%X | Read: 0x%X\r\n",
                    i, OV5640_INIT_Config[i][0], OV5640_INIT_Config[i][1], read_reg);
        }
    }
    
    /* 2. 【核心关灯补丁】精准操作寄存器，不伤屏幕 */
    
    // A. 先关闭逻辑触发 (0x3B00)
    // if (SCCB_WriteReg_16Bit(0x3B00, 0x00) == SUCCESS) {
    //     printf("[Light] Flash Logic Disabled.\r\n");
    // }

    // // B. 精准操作 0x3016 (只灭灯位 Bit 1，强保时钟位 Bit 0)
    // temp_val = SCCB_ReadReg_16Bit(0x3016);
    // // 逻辑：清零 Bit 1 (&0xFD)，置位 Bit 0 (|0x01)
    // if (SCCB_WriteReg_16Bit(0x3016, (temp_val & 0xFD) | 0x01) == SUCCESS) {
    //     printf("[Light] Physical Strobe Pin Disabled. PCLK Safe.\r\n");
    // }

    // // C. 最狠的一招：将 STROBE 引脚设为输入/高阻态 (0x3019)
    // // 配合底板上的 10K 下拉电阻，强制让电平归零
    // if (SCCB_WriteReg_16Bit(0x3019, 0x00) == SUCCESS) {
    //     printf("[Light] Pin 0x3019 set to Input (High-Z).\r\n");
    // }

    // printf("[System] OV5640 Config Completed.\r\n");
}

void OV5640_Set_Pixformat(uint8_t pixformat)
{
   uint8_t OV5640_Reg;  

    if( pixformat == Pixformat_JPEG )
    {
        SCCB_WriteReg_16Bit(OV5640_FORMAT_CONTROL,      0x30);  
        SCCB_WriteReg_16Bit(OV5640_FORMAT_CONTROL_MUX,  0x00);  
        SCCB_WriteReg_16Bit(OV5640_JPEG_MODE_SELECT, 0x02);     
        SCCB_WriteReg_16Bit(OV5640_JPEG_VFIFO_CTRL00, 0xA0);        
        SCCB_WriteReg_16Bit(OV5640_JPEG_VFIFO_HSIZE_H, OV5640_Width>>8);            
        SCCB_WriteReg_16Bit(OV5640_JPEG_VFIFO_HSIZE_L, (uint8_t)OV5640_Width);  
        SCCB_WriteReg_16Bit(OV5640_JPEG_VFIFO_VSIZE_H, OV5640_Height>>8);           
        SCCB_WriteReg_16Bit(OV5640_JPEG_VFIFO_VSIZE_L, (uint8_t)OV5640_Height); 
    }
    else if( pixformat == Pixformat_GRAY )
    {
        SCCB_WriteReg_16Bit(OV5640_FORMAT_CONTROL,      0x10);  
        SCCB_WriteReg_16Bit(OV5640_FORMAT_CONTROL_MUX,  0x00);         
    }
    else    // RGB565
    {
        SCCB_WriteReg_16Bit(OV5640_FORMAT_CONTROL,      0x6F);   
        SCCB_WriteReg_16Bit(OV5640_FORMAT_CONTROL_MUX,  0x01);     
    }
    
   OV5640_Reg = SCCB_ReadReg_16Bit(0x3821);   
    SCCB_WriteReg_16Bit(0x3821,(OV5640_Reg & 0xDF) | ((pixformat == Pixformat_JPEG) ? 0x20 : 0x00));
     
   OV5640_Reg = SCCB_ReadReg_16Bit(0x3002);   
    SCCB_WriteReg_16Bit(0x3002,(OV5640_Reg & 0xE3) | ((pixformat == Pixformat_JPEG) ? 0x00 : 0x1C));
     
   OV5640_Reg = SCCB_ReadReg_16Bit(0x3006);   
    SCCB_WriteReg_16Bit(0x3006,(OV5640_Reg & 0xD7) | ((pixformat == Pixformat_JPEG) ? 0x28 : 0x00));
}

void OV5640_Set_JPEG_QuantizationScale(uint8_t scale)
{
    SCCB_WriteReg_16Bit(0x4407, scale);     
}

int8_t OV5640_Set_Framesize(uint16_t width,uint16_t height)
{
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0X03);   
    
    SCCB_WriteReg_16Bit(OV5640_TIMING_DVPHO_H,width>>8);            
    SCCB_WriteReg_16Bit(OV5640_TIMING_DVPHO_L,width&0xff);
    SCCB_WriteReg_16Bit(OV5640_TIMING_DVPVO_H,height>>8);       
    SCCB_WriteReg_16Bit(OV5640_TIMING_DVPVO_L,height&0xff);

    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0X13);       
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0Xa3);       
    
    return OV5640_Success; 
}

int8_t OV5640_Set_Horizontal_Mirror( int8_t ConfigState )
{
   uint8_t OV5640_Reg;  
   OV5640_Reg = SCCB_ReadReg_16Bit(OV5640_TIMING_Mirror);   
   if ( ConfigState == OV5640_Enable )   
   { 
      OV5640_Reg |= 0X06;  
   } 
   else                    
   {
      OV5640_Reg &= 0xF9;   
   }
   return  SCCB_WriteReg_16Bit(OV5640_TIMING_Mirror,OV5640_Reg);   
}

int8_t OV5640_Set_Vertical_Flip( int8_t ConfigState )
{
   uint8_t OV5640_Reg;  
   OV5640_Reg = SCCB_ReadReg_16Bit(OV5640_TIMING_Flip);          
   if ( ConfigState == OV5640_Enable )   
   { 
        OV5640_Reg |= 0X06;       
   } 
   else   
   {
      OV5640_Reg &= 0xF9;   
   }
   return  SCCB_WriteReg_16Bit(OV5640_TIMING_Flip,OV5640_Reg);   
}


void OV5640_Set_Brightness(int8_t Brightness)
{
    Brightness = Brightness+4;
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0X03);   
    SCCB_WriteReg_16Bit( 0x5587, OV5640_Brightness_Config[Brightness][0]);  
    SCCB_WriteReg_16Bit( 0x5588, OV5640_Brightness_Config[Brightness][1]);
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0X13);       
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0Xa3);       
}

void OV5640_Set_Contrast(int8_t Contrast)
{
    Contrast = Contrast+3;
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0X03);   
    SCCB_WriteReg_16Bit( 0x5586, OV5640_Contrast_Config[Contrast][0]);  
    SCCB_WriteReg_16Bit( 0x5585, OV5640_Contrast_Config[Contrast][1]);
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0X13);       
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0Xa3);       
}

void OV5640_Set_Effect(uint8_t effect_Mode)
{
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0X03);   
    SCCB_WriteReg_16Bit( 0x5580, OV5640_Effect_Config[effect_Mode][0]); 
    SCCB_WriteReg_16Bit( 0x5583, OV5640_Effect_Config[effect_Mode][1]);
    SCCB_WriteReg_16Bit( 0x5584, OV5640_Effect_Config[effect_Mode][2]); 
    SCCB_WriteReg_16Bit( 0x5003, OV5640_Effect_Config[effect_Mode][3]); 
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0X13);       
    SCCB_WriteReg_16Bit(OV5640_GroupAccess,0Xa3);       
}

int8_t OV5640_AF_Download_Firmware(void)
{ 
    uint8_t  AF_Status = 0;     
    uint16_t i = 0;                 
    uint16_t OV5640_MCU_Addr = 0x8000;  
    
    SCCB_WriteReg_16Bit(0x3000, 0x20);  
    SCCB_WriteBuffer_16Bit( OV5640_MCU_Addr,(uint8_t *)OV5640_AF_Firmware,sizeof(OV5640_AF_Firmware) );
    SCCB_WriteReg_16Bit(0x3000,0x00);  
    
    for(i=0;i<100;i++)  
    {
        AF_Status = SCCB_ReadReg_16Bit(OV5640_AF_FW_STATUS);    
        if( AF_Status == 0x7E)
        {
            printf("AF Firmware Initializing... >>>\r\n");
        }           
        if( AF_Status == 0x70)  
        {
            printf("AF Firmware Download Success!\r\n");
            return OV5640_Success;  
        }           
    }
    printf("AF Firmware Download FAILED!!! ERROR!!\r\n");
    return OV5640_Error;    
}  

int8_t OV5640_AF_QueryStatus(void)
{
    uint8_t  AF_Status = 0;     
    AF_Status = SCCB_ReadReg_16Bit(OV5640_AF_FW_STATUS);    
    printf("AF_Status:0x%x\r\n",AF_Status);
    if( (AF_Status == 0x10)||(AF_Status == 0x20) )      
    {
        return OV5640_AF_End;   
    }
    else
    {
        return OV5640_AF_Focusing;  
    }
}

void OV5640_AF_Trigger_Constant(void)
{
    SCCB_WriteReg_16Bit(0x3022,0x04);   
}

void OV5640_AF_Trigger_Single(void)
{
    SCCB_WriteReg_16Bit(OV5640_AF_CMD_MAIN,0x03);   
}

void OV5640_AF_Release(void)
{
    SCCB_WriteReg_16Bit(OV5640_AF_CMD_MAIN,0x08);   
}

/***************************************************************************************************************************************
* 函 数 名: HAL_DCMI_FrameEventCallback
* 函数功能: 帧回调函数，每传输一帧数据，会进入该中断服务函数
*****************************************************************************************************************************************/
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    static uint32_t DCMI_Tick = 0;          
   static uint8_t  DCMI_Frame_Count = 0;    

    if(HAL_GetTick() - DCMI_Tick >= 1000)    
    {
        DCMI_Tick = HAL_GetTick();        
        OV5640_FPS = DCMI_Frame_Count;   
        DCMI_Frame_Count = 0;            
    }
    DCMI_Frame_Count ++;    

   OV5640_FrameState = 1;  
   
   // 每次捕捉到一帧，通过串口打印，便于排查假死问题
   //printf("[DEBUG] FrameEventCallback triggered. Frame Captured!\r\n");  
}

/***************************************************************************************************************************************
* 函 数 名: HAL_DCMI_ErrorCallback
* 函数功能: 错误回调函数，加入 OVR 自动恢复机制
*****************************************************************************************************************************************/
void  HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
    uint32_t err_code = HAL_DCMI_GetError(hdcmi);
    printf("!!! DCMI ERROR OCCURRED !!! Error Code: 0x%X\r\n", err_code);
    
    // 如果是 DMA FIFO 溢出错误 (HAL_DCMI_ERROR_OVR)
    if (err_code == HAL_DCMI_ERROR_OVR)
    {
        printf(">>> Trying to restart DCMI due to Overrun (OVR) Error...\r\n");
        HAL_DCMI_Suspend(hdcmi);
        HAL_DCMI_Resume(hdcmi);
    }
}
