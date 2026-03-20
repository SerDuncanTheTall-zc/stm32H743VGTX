/* USER CODE BEGIN Header */
// ... 你的文件头省略 ...
/* USER CODE END Header */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"   // 必须包含！为了使用 HAL_GPIO_TogglePin
#include <stdio.h>  // 必须包含！为了使用 printf
#include "ov5640.h"
extern TIM_HandleTypeDef htim2;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* 1. 定义一个线程控制块 (相当于线程的“身份证”) */
TX_THREAD my_camera_thread;

/* 2. 定义相机线程专属的内存堆栈 */
#define CAMERA_STACK_SIZE 2048
uint8_t my_camera_stack[CAMERA_STACK_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* 堆栈空间 */

/* 声明线程入口函数 */
void My_Camera_Thread_Entry(ULONG thread_input);

/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */

  /* USER CODE END App_ThreadX_MEM_POOL */

  /* USER CODE BEGIN App_ThreadX_Init */


  /* 创建相机线程 */
    tx_thread_create(&my_camera_thread, "Camera Init Thread", My_Camera_Thread_Entry, 0,
                     my_camera_stack, CAMERA_STACK_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN  Before_Kernel_Start */

  /* USER CODE END  Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN  Kernel_Start_Error */

  /* USER CODE END  Kernel_Start_Error */
}

/* USER CODE BEGIN 2 */

/**
  * @brief  相机主线程：负责上电、读 ID、初始化
  */
void My_Camera_Thread_Entry(ULONG thread_input)
{
    (void)thread_input;
    uint16_t camera_id = 0;

    // 稍微延时一下，等外设和串口完全准备好
    tx_thread_sleep(100);
    printf("\r\n=================================\r\n");
    printf("--- Camera Thread Started ---\r\n");
    printf("=================================\r\n");

    /* 1. 启动 24MHz PWM 时钟 (给摄像头提供心跳) */
    // 这一步非常关键，没有 XCLK，OV5640 就像个死石头，连 I2C 都不会回复
    if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) == HAL_OK) {
        printf("[OK] TIM2 PWM Started (Providing 24MHz XCLK to PA5)\r\n");
    } else {
        printf("[FAIL] TIM2 PWM Start Error!\r\n");
    }

    /* 2. 硬件开机与复位序列 */
    printf("--- Executing Hardware Reset Sequence ---\r\n");
    OV5640_HW_Reset();
    printf("[OK] Hardware Reset Completed.\r\n");

    /* 3. 宣判命运的时刻：通过 I2C 读取芯片 ID */
    printf("--- Attempting to read OV5640 ID via I2C ---\r\n");
    camera_id = OV5640_ReadID();

    if (camera_id == 0x5640)
    {
        printf("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
        printf("[SUCCESS] OV5640 Detected! ID: 0x%04X\r\n", camera_id);
        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");

        // TODO: 这里是接下来要加入的初始化寄存器数组写入步骤
        // printf("Writing Init Registers...\r\n");
        // OV5640_Config();
    }
    else
    {
        printf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("[FAILED] Camera not found. Read ID: 0x%04X\r\n", camera_id);
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");

        printf("--- Troubleshooting Checklist ---\r\n");
        printf("1. Is the FPC Cable inserted correctly (gold fingers facing the right way)?\r\n");
        printf("2. Is the I2C correctly configured with Pull-ups (PB8, PB9)?\r\n");
        printf("3. Is the TIM2 clock actually outputting 24MHz on PA5?\r\n");
        printf("4. Is the PWDN pin (PD14) correctly pulled LOW to turn the camera ON?\r\n");

        // 读不到 ID，程序在此挂起，不再往下执行，防止 I2C 报错刷屏
        while(1) {
            tx_thread_sleep(1000);
        }
    }

    /* 4. 如果成功了，线程进入空闲等待状态，等待后续任务（如启动 DCMI 捕获） */
    while(1)
    {
        // 以后这里可以用来监控帧率、或者处理图像数据
        tx_thread_sleep(1000);
    }
}

/* USER CODE END 2 */
