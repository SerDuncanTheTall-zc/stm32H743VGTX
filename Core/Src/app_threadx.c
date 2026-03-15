/* USER CODE BEGIN Header */
// ... 你的文件头省略 ...
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"   // 必须包含！为了使用 HAL_GPIO_TogglePin
#include <stdio.h>  // 必须包含！为了使用 printf
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

  tx_kernel_enter(); // 一旦执行这句，系统就被 ThreadX 接管了

  /* USER CODE BEGIN  Kernel_Start_Error */

  /* USER CODE END  Kernel_Start_Error */
}

/* USER CODE BEGIN 2 */
/* 5. 真正干活的地方！也就是你的“多任务版 while(1)” */
void My_LED_Thread_Entry(ULONG thread_input)
{
    /* 强行避免编译器报 "变量未使用" 的警告 */
    (void)thread_input;

    /* 这个 while(1) 只属于这个线程，不会卡死别的线程 */
    while(1)
    {
        /* 翻转 PC13 蓝灯 */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

        /* 串口打印 */
        printf("ThreadX is running! LED Toggled jojo chao .\r\n");

        /* ThreadX 专属休眠函数：休眠 100 个系统心跳 (Ticks) */
        /* 如果你的系统心跳默认是 1000Hz(1ms)，那这里就是 100ms */
        tx_thread_sleep(100);
    }
}
/* USER CODE END 2 */
