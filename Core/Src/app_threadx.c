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
extern TIM_HandleTypeDef htim2;
extern I2C_HandleTypeDef hi2c1;

void My_Camera_Thread_Entry(ULONG thread_input)
{
    (void)thread_input;
    uint16_t camera_id = 0;
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
    tx_thread_sleep(100);
    printf("\r\n--- [DEBUG] Camera System Diagnostics Start ---\r\n");

    /* 1. 启动 24MHz XCLK */
    if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) == HAL_OK) {
        printf("[OK] XCLK (24MHz) is pulsing on PA5.\r\n");
    } else {
        printf("[FAIL] PWM Start failed! Check TIM2 configuration.\r\n");
    }

    /* 2. 执行硬件复位序列 */
    // 务必确认 PD14(PWDN) 被拉低，PC4(RESET) 如果接了也要操作
    printf("[INFO] Executing HW Power-up & Reset...\r\n");
    OV5640_HW_Reset();
    tx_thread_sleep(20);

    /* 3. 核心：I2C 全地址扫描 */
    // 如果 I2C 线没连通，这个扫描会一个地址都抓不到
    printf("--- [I2C SCANNER START] ---\r\n");
    uint8_t found_devices = 0;
    for (uint16_t i = 1; i < 128; i++)
    {
        // 尝试检查设备是否准备好（地址左移一位）
        if (HAL_I2C_IsDeviceReady(&hi2c1, (i << 1), 3, 10) == HAL_OK)
        {
            printf("[FOUND] I2C Device detected at Address: 0x%02X\r\n", i << 1);
            found_devices++;
        }
    }
    if (found_devices == 0) {
        printf("[CRITICAL] No I2C devices responded. Check SDA/SCL wiring!\r\n");
    }
    printf("--- [I2C SCANNER FINISHED] ---\r\n");

    /* 4. 尝试读取 Chip ID */
    printf("[INFO] Attempting to read OV5640 ID (Reg 0x300A)...\r\n");
    camera_id = OV5640_ReadID();

    if (camera_id == 0x5640)
    {
        printf("\r\n************************************\r\n");
        printf("  SUCCESS! OV5640 ID: 0x%04X\r\n", camera_id);
        printf("************************************\r\n");
    }
    else
    {
        printf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("  FAILED! Read ID: 0x%04X (Expected 0x5640)\r\n", camera_id);
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");

        // 如果扫描到了 0x78 但 ID 读出来是 0，说明通讯通了但数据不对
        // 如果扫描啥都没发现，说明物理连接依然不通
    }

    while(1) { tx_thread_sleep(1000); }
}

/* USER CODE END 2 */
