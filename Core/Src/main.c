/***

>>>>> 功能说明：
  *
  * OV5640采集图像并显示到屏幕
  *
  ************************************************************************************************
***/

#include "main.h"
#include "led.h"
#include "usart.h"
#include "lcd_spi_154.h"
#include "dcmi_ov5640.h"  
#include "bsp_fill_light.h"
#include <stdio.h>

#define Camera_Buffer 0x24000000    // 摄像头图像缓冲区
uint16_t Camera_Frame_Buffer[Display_Width * Display_Height] __attribute__((section(".camera_section"), aligned(32)));
/********************************************** 函数声明 *******************************************/

void SystemClock_Config(void);    // 时钟初始化
void MPU_Config(void);          // MPU配置

/***************************************************************************************************
* 函 数 名: main
* 说    明: 主程序
****************************************************************************************************/

int main(void)
{
  MPU_Config();       // MPU配置
  SCB_EnableICache();   // 使能ICache
  SCB_EnableDCache();   // 使能DCache
  HAL_Init();         // 初始化HAL库
  SystemClock_Config(); // 配置系统时钟，主频480MHz
  
  LED_Init();         // 初始化LED引脚
  FillLight_Init();   /* 初始化 PC4 补光灯并强行关灯 */
  USART1_Init();        // USART1初始化 
  
  printf("\r\n\r\n--- System Booting ---\r\n");

  printf("[1] Init SPI & LCD...\r\n");
  SPI_LCD_Init();       // 液晶屏以及SPI初始化 
  
  printf("[2] Init DCMI & OV5640...\r\n");
  DCMI_OV5640_Init();           // DCMI以及OV5640初始化
  
  printf("[3] Download AF Firmware...\r\n");
  OV5640_AF_Download_Firmware();  // 写入自动对焦固件
  OV5640_AF_Trigger_Constant();   // 自动对焦 ，持续触发
  OV5640_Set_Horizontal_Mirror( OV5640_Enable);
  OV5640_Set_Vertical_Flip( OV5640_Disable );   // 取消垂直翻转

  printf("[4] Start DMA Continuous Transfer...\r\n");
  //OV5640_DMA_Transmit_Continuous(Camera_Buffer, Display_BufferSize);  // 启动DMA连续传输
  OV5640_DMA_Transmit_Continuous((uint32_t)Camera_Frame_Buffer, Display_BufferSize);
  
  printf("--- Init Done. Entering Main Loop ---\r\n");

  uint32_t last_heartbeat = HAL_GetTick();
  uint32_t frame_count = 0;
// 1. 再次通过 SCCB 关掉摄像头的灯信号 (因为固件可能重置了它)
// 此时已经过了 AF 下载阶段
uint8_t current_3016 = SCCB_ReadReg_16Bit(0x3016);
SCCB_WriteReg_16Bit(0x3016, current_3016 & 0xFD); // 灭灯位，保时钟位
SCCB_WriteReg_16Bit(0x3019, 0x00);               // 设为输入/高阻
SCCB_WriteReg_16Bit(0x3B00, 0x00);               // 禁用逻辑

// 2. 尝试在不 Init 的情况下，通过寄存器直接拉低 PC4 (防止主控端输出高)
// 这种写法不触碰 GPIO 的配置寄存器，只操作输出寄存器，不会干扰屏幕
__HAL_RCC_GPIOC_CLK_ENABLE();
GPIOC->BSRR = (uint32_t)GPIO_PIN_4 << 16U; // 强制 Reset PC4
  FILL_LIGHT_OFF();
  while (1)
  {
    // --- 1. 心跳侦听：每隔 2 秒打印一次，证明 main 循环没死 ---
    if (HAL_GetTick() - last_heartbeat > 2000) 
    {
        last_heartbeat = HAL_GetTick();
        //printf("[Heartbeat] main loop running. OV5640_FrameState = %d | Total Frames = %d\r\n", 
          //      OV5640_FrameState, frame_count);
    }

    // --- 2. 图像捕获与显示处理 ---
    if ( OV5640_FrameState == 1 ) // 采集到了一帧图像
    {   
        OV5640_FrameState = 0;    // 清零标志位
        frame_count++;
        
        // 打印调试：开始处理这帧图像
        //printf(" -> Frame %d Captured! Updating LCD...\r\n", frame_count);

        // --- 核心修复：强制刷新 D-Cache ---
        // 确保 CPU 读到的不是缓存里的老数据，而是 DMA 刚刚写入物理内存的新数据
        // SCB_InvalidateDCache_by_Addr((uint32_t *)Camera_Buffer, Display_Width * Display_Height * 2);

        // // 将图像数据复制到屏幕
        // LCD_CopyBuffer(0, 0, Display_Width, Display_Height, (uint16_t *)Camera_Buffer); 
        /* 强制刷新 Cache */
        SCB_InvalidateDCache_by_Addr((uint32_t *)Camera_Frame_Buffer, Display_Width * Display_Height * 2);

        /* 将数组数据复制到屏幕 */
        LCD_CopyBuffer(0, 0, Display_Width, Display_Height, Camera_Frame_Buffer);
        
        // 显示文本
        LCD_DisplayString( 84 ,200,"FPS:");
        LCD_DisplayNumber( 132,200, OV5640_FPS,2); // 显示帧率 
        
        LED1_Toggle;  

        // 打印调试：屏幕刷新完毕
        //printf(" -> LCD Update Done. FPS = %d\r\n", OV5640_FPS);
    } 
  }
}

/****************************************************************************************************/
/**
  * @brief  System Clock Configuration
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_SPI4;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_D2PCLK1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }      
}

//  配置MPU
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  HAL_MPU_Disable();    // 先禁止MPU

  MPU_InitStruct.Enable         = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress    = 0x24000000;
  MPU_InitStruct.Size           = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission   = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable    = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable    = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number         = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable   = 0x00;
  MPU_InitStruct.DisableExec    = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);  

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT); // 使能MPU
}

/**
  * @brief  This function is executed in case of error occurrence.
  */
void Error_Handler(void)
{
  __disable_irq();
  printf("!!! Fatal Error: Entered Error_Handler !!!\r\n");
  while (1)
  {
  }
}
