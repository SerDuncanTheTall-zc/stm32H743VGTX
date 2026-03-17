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

extern TIM_HandleTypeDef htim5;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEMO_STACK_SIZE 1024 // 定义线程分配的内存堆栈大小
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* 1. 定义一个线程控制块 (相当于线程的“身份证”) */
TX_THREAD my_led_thread;

/* 2. 定义这个线程专属的内存堆栈 */
uint8_t my_thread_stack[DEMO_STACK_SIZE];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* 3. 声明我们的线程入口函数 */
void My_LED_Thread_Entry(ULONG thread_input);
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

  /* 4. 创建并启动线程 */
  tx_thread_create(&my_led_thread,             // 线程控制块的指针
                   "My LED and Print Thread",  // 随便起个名字，调试用
                   My_LED_Thread_Entry,        // 告诉系统这个线程要去跑哪个函数！
                   0,                          // 传给线程的参数（暂时不用，写0）
                   my_thread_stack,            // 分配给它的堆栈内存
                   DEMO_STACK_SIZE,            // 堆栈大小
                   15,                         // 优先级 (0~31，越小越优先)
                   15,                         // 抢占阈值 (通常和优先级一样)
                   TX_NO_TIME_SLICE,           // 时间片轮转设置 (这里不用)
                   TX_AUTO_START);             // 创建完立刻自动启动！

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
/* 5. 真正干活的地方！也就是你的“多任务版 while(1)” */
void My_LED_Thread_Entry(ULONG thread_input)
{
    (void)thread_input; 

    /* 启动 TIM5 的 Channel 1 的 PWM 输出 */
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
    
    printf("Servo Thread Started!\r\n");

    // while(1)
    // {
    //     /* 转到 0度 (脉宽 500us) */
    //     __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 500);
    //     printf("Position: 0 deg\r\n");
    //     tx_thread_sleep(1000); /* 停顿 1 秒 (假设1个Tick=1ms) */

    //     /* 转到 90度 (脉宽 1500us) */
    //     __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 1500);
    //     printf("Position: 90 deg\r\n");
    //     tx_thread_sleep(1000);

    //     /* 转到 180度 (脉宽 2500us) */
    //     __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 2500);
    //     printf("Position: 180 deg\r\n");
    //     tx_thread_sleep(1000);
        
    //     /* 转回 90度 (脉宽 1500us) */
    //     __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 1500);
    //     printf("Position: 90 deg\r\n");
    //     tx_thread_sleep(1000);
    // }
}

/*

*/


/* USER CODE END 2 */
