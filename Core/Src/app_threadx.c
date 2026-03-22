#include "app_threadx.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "ov5640.h"
#include "dcmi_ov5640_cfg.h"

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;
extern DCMI_HandleTypeDef hdcmi;

#define CAMERA_BUFFER_ADDR 0x24000000
#define DMA_TRANSFER_WORDS 65535

TX_THREAD my_camera_thread;
#define CAMERA_STACK_SIZE 2048
uint8_t my_camera_stack[CAMERA_STACK_SIZE];
TX_SEMAPHORE frame_ready_sem;

// 辅助函数
void OV5640_WriteReg_Helper(uint16_t reg, uint8_t data) {
    OV5640_WriteReg(reg, data);
}

void UART_BinaryDump(uint8_t *ptr, uint32_t len) {
    uint32_t sent = 0;
    while (sent < len) {
        // 缩段发送长度，防止 uint16_t 溢出
        uint16_t chunk_size = (len - sent > 1000) ? 1000 : (uint16_t)(len - sent);

        // 使用阻塞模式发送
        if(HAL_UART_Transmit(&huart1, ptr + sent, chunk_size, 1000) == HAL_OK) {
            sent += chunk_size;
        }
        // 这里不用 tx_thread_sleep，用最轻量的微秒级等待，防止串口硬件缓冲区炸掉
        for(volatile int i=0; i<2000; i++);
    }
}



// 🚀 修正后的加载函数名
void OV5640_Init_Vendor_RGB565(void)
{
    printf("[INFO] Loading Vendor's RGB565 Configuration...\r\n");
    uint16_t reg_count = sizeof(OV5640_INIT_Config) / sizeof(OV5640_INIT_Config[0]);
    for(int i = 0; i < reg_count; i++) {
        OV5640_WriteReg_Helper(OV5640_INIT_Config[i][0], (uint8_t)OV5640_INIT_Config[i][1]);
    }
    printf("[OK] Vendor Configuration Loaded.\r\n");
}

// 🚀 修正后的主线程
void My_Camera_Thread_Entry(ULONG thread_input)
{
    (void)thread_input;
    uint16_t camera_id = 0;

    tx_thread_sleep(100);
    printf("\r\n--- [ACTION START] Memory-Only Capture Mode --- \r\n");

    /* 1. 硬件启动 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    OV5640_HW_Reset();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
    tx_thread_sleep(50);

    /* 2. ID 验证 */
    camera_id = OV5640_ReadID();
    if (camera_id != 0x5640) {
        printf("[FAILED] OV5640 ID: 0x%04X\r\n", camera_id);
        while(1) tx_thread_sleep(100);
    }
    printf("[SUCCESS] OV5640 Detected.\r\n");

    /* 3. 配置摄像头 */
    OV5640_WriteReg_Helper(0x3016, 0x02); // 关灯
    OV5640_WriteReg_Helper(0x3019, 0x00);

    // 调用修正后的函数名
    OV5640_Init_Vendor_RGB565();
    tx_thread_sleep(500);


    printf(">>> System Ready. Entering Loop...\r\n");

        while(1)
        {
            printf("[Step 1] Clearing Buffer...\r\n");
            // 如果这里崩了，说明 0x24000000 内存没开启或地址写错了

            memset((void*)CAMERA_BUFFER_ADDR, 0x00, 1024);

            printf("[Step 2] Clearing Flags...\r\n");
            __HAL_DCMI_CLEAR_FLAG(&hdcmi, DCMI_FLAG_FRAMERI | DCMI_FLAG_VSYNCRI);

            printf("[Step 3] DMA Starting...\r\n");
            // 如果这里崩了，说明 DCMI 或 DMA 的配置（比如 MPU）有冲突
            HAL_StatusTypeDef status = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, CAMERA_BUFFER_ADDR, DMA_TRANSFER_WORDS);
            printf("[Step 4] DMA Status: %d. Waiting for VSYNC...\r\n", status);

            // 如果这里一直转圈不打印 [Step 5]，说明硬件没给信号（极性反了）
            if (tx_semaphore_get(&frame_ready_sem, 1000) == TX_SUCCESS)
            {
                printf("[Step 5] Success! Frame Captured.\r\n");
                __NOP(); // 断点位置
            }
            else
            {
                printf("[Step 5] Timeout! VSYNC signal NOT detected.\r\n");
                HAL_DCMI_Stop(&hdcmi);
            }

            tx_thread_sleep(2000);
        }
}

// 回调与系统
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi) {
    tx_semaphore_put(&frame_ready_sem);
}
void MX_ThreadX_Init(void) {
  tx_kernel_enter();
}
UINT App_ThreadX_Init(VOID *memory_ptr) {
  tx_semaphore_create(&frame_ready_sem, "Frame Sem", 0);
  tx_thread_create(&my_camera_thread, "Cam Thread", My_Camera_Thread_Entry, 0,
                   my_camera_stack, CAMERA_STACK_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
  return TX_SUCCESS;
}
