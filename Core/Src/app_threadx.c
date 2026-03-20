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
TX_THREAD my_led_thread;       // 你原有的舵机线程
TX_THREAD my_ultrasonic_thread; // 新加的超声波线程

/* 2. 定义这个线程专属的内存堆栈 */
#define MY_STACK_SIZE 1024
uint8_t my_led_stack[MY_STACK_SIZE];
uint8_t my_ultrasonic_stack[MY_STACK_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* 堆栈空间 */

/* 声明线程入口函数 */
void My_LED_Thread_Entry(ULONG thread_input);
void My_Ultrasonic_Thread_Entry(ULONG thread_input);
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


  /* --- 必须在这里创建线程，否则它们永远不会运行 --- */

  // 1. 创建舵机线程
  tx_thread_create(&my_led_thread, "Smooth Servo", My_LED_Thread_Entry, 0,
                   my_led_stack, MY_STACK_SIZE, 15, 15, TX_NO_TIME_SLICE, TX_AUTO_START);

  // 2. 创建超声波线程
  tx_thread_create(&my_ultrasonic_thread, "Ultrasonic", My_Ultrasonic_Thread_Entry, 0,
                   my_ultrasonic_stack, MY_STACK_SIZE, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
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

/* 简单的微秒延时函数（H7 跑得快，可以用循环模拟，或者用 DWT 计数器） */
void delay_us(uint32_t us) {
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while(ticks--);
}

/* 全局变量，方便其他线程（如舵机）调用 */


/* 定义引脚，方便以后修改 */
#define TRIG_PIN  GPIO_PIN_2
#define ECHO_PIN  GPIO_PIN_3
#define GPIO_PORT GPIOC

float current_distance_cm = 0;

void My_Ultrasonic_Thread_Entry(ULONG thread_input)
{
    (void)thread_input;

    /* --- 1. 初始化 DWT 计数器 (H7 内核专用) --- */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    printf("Ultrasonic Online! Trig:PC2, Echo:PC3\r\n");

    while(1)
    {
        uint32_t start_tick = 0;
        uint32_t stop_tick = 0;
        uint32_t timeout = 0;

        // --- 2. 触发信号 ---
        // 先拉低 5us，确保电平干净
        HAL_GPIO_WritePin(GPIO_PORT, TRIG_PIN, GPIO_PIN_RESET);
        for(volatile int i=0; i<2000; i++);

        // 发送至少 10us 的高电平触发脉冲
        HAL_GPIO_WritePin(GPIO_PORT, TRIG_PIN, GPIO_PIN_SET);
        for(volatile int i=0; i<15000; i++); // H7 480MHz 需较多循环
        HAL_GPIO_WritePin(GPIO_PORT, TRIG_PIN, GPIO_PIN_RESET);

        // --- 3. 等待 Echo 上升沿 (信号开始) ---
        timeout = 5000000; // 适当加长超时时间
        while(HAL_GPIO_ReadPin(GPIO_PORT, ECHO_PIN) == GPIO_PIN_RESET && timeout > 0) {
            timeout--;
        }
        start_tick = DWT->CYCCNT;

        // --- 4. 等待 Echo 下降沿 (信号结束) ---
        // 注意：这里的 timeout 必须重新赋初值
        timeout = 5000000;
        while(HAL_GPIO_ReadPin(GPIO_PORT, ECHO_PIN) == GPIO_PIN_SET && timeout > 0) {
            timeout--;
        }
        stop_tick = DWT->CYCCNT;

        // --- 5. 计算逻辑 ---
        if (timeout > 0)
        {
            uint32_t diff = stop_tick - start_tick;

            // 排除掉极小的杂波 (比如你之前看到的 190 Ticks)
            if (diff < 1000) {
                 // printf("Glitch detected\r\n");
                 continue;
            }

            float duration_us = (float)diff / (SystemCoreClock / 1000000.0f);
            current_distance_cm = (duration_us * 0.0343f) / 2.0f;

            // 打印结果，如果浮点打不出，请改用 (int)current_distance_cm
            printf("Dist: %.2f cm | Ticks: %lu\r\n", current_distance_cm, diff);
        }
        else
        {
            // 如果一直打印 Timeout，说明没收到回响
            printf("Sensor Timeout! Check Echo on PC3 and GND.\r\n");
        }

        /* 每 100ms 测一次，给舵机留出处理时间 */
        tx_thread_sleep(100);
    }
}

#define SMOOTH_FACTOR    0.03f
#define EVADE_FACTOR     0.20f
#define CENTER_PULSE     1500

void My_LED_Thread_Entry(ULONG thread_input)
{
    (void)thread_input;
    float current_f = 1500.0f;
    uint32_t target_pulse = CENTER_PULSE;

    // 新增：警报状态机 (0: 安全, 1: 正在避障)
    uint8_t is_evading = 0;

    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
    printf("Servo Guard Mode Fixed! No more twitching.\r\n");

    while(1)
    {
        /* --- 1. 危险判定 --- */
        if (current_distance_cm > 2.0f && current_distance_cm < 10.0f)
        {
            // 关键修复：只有在“刚发现”障碍物时，才决定往哪躲！
            if (is_evading == 0)
            {
                if (current_f >= 1500.0f) target_pulse = 600;
                else target_pulse = 2400;

                is_evading = 1; // 锁定状态，装死！
            }

            // 执行避障动作
            if (fabsf(current_f - (float)target_pulse) > 1.0f)
            {
                current_f += ((float)target_pulse - current_f) * EVADE_FACTOR;
                __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, (uint32_t)current_f);
            }
            else
            {
                // 到达避障位置后，如果手还不走，它就死死盯着/躲着，不动如山
                // 这里什么都不做，就是最好的处理
            }
        }
        /* --- 2. 安全恢复 --- */
        else if (current_distance_cm >= 12.0f)
        {
            if (is_evading == 1)
            {
                // 刚把手拿开，解除锁定，目标设回正前方
                is_evading = 0;
                target_pulse = CENTER_PULSE;
                // 这里可以加个小延时，模拟“确认安全后再动”的感觉
                tx_thread_sleep(500);
            }

            // 丝滑地转回正前方
            if (fabsf(current_f - (float)target_pulse) > 1.0f)
            {
                current_f += ((float)target_pulse - current_f) * SMOOTH_FACTOR;
                __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, (uint32_t)current_f);
            }
        }

        /* 基础休眠，保证 ThreadX 调度 */
        tx_thread_sleep(5);
    }
}


/* USER CODE END 2 */
