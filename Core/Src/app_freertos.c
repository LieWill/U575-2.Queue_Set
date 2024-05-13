/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_freertos.c
 * Description        : FreeRTOS applicative file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "OLED.H"
#include "rtc.h"
#include "queue.h"
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
/* USER CODE BEGIN Variables */
QueueHandle_t TimeQueue;
QueueHandle_t DateQueue;
RTC_TimeTypeDef SetTime;
RTC_DateTypeDef SetDate;
QueueSetHandle_t QueueSet1;
uint8_t RX_Data[100];

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name       = "defaultTask",
    .priority   = (osPriority_t)osPriorityNormal,
    .stack_size = 128 * 4};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void OledShowTask(void *argument);
void RtcGetTime(void *argument);
/* USER CODE END FunctionPrototypes */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    TimeQueue = xQueueCreate(1, sizeof(RTC_TimeTypeDef));
    DateQueue = xQueueCreate(1, sizeof(RTC_DateTypeDef));
    QueueSet1 = xQueueCreateSet(2);
    // xQueueAddToSet(TimeQueue, QueueSet1);
    // xQueueAddToSet(DateQueue, QueueSet1);
    /* USER CODE END RTOS_QUEUES */
    /* creation of defaultTask */
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    xTaskCreate(OledShowTask, "OledShowTask", 1024, NULL, osPriorityNormal, NULL);
    xTaskCreate(RtcGetTime, "RtcGetTime", 256, NULL, osPriorityNormal, NULL);
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief Function implementing the defaultTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    /* USER CODE BEGIN defaultTask */
    /* Infinite loop */
    for (;;) {
        osDelay(1);
    }
    /* USER CODE END defaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void OledShowTask(void *argument)
{
    RTC_TimeTypeDef GetTime = {0};
    RTC_DateTypeDef GetDate = {0};
    int n                   = 0;
    for (;;) {
        xQueueReceive(TimeQueue, &GetTime, portMAX_DELAY);
        xQueueReceive(DateQueue, &GetDate, portMAX_DELAY);
        OLED_Printf(1, 1, "n = %d", n++);
        OLED_Printf(2, 1, "%02d/%02d/%02d", GetDate.Date, GetDate.Month, GetDate.Year);
        OLED_Printf(3, 1, "%02d:%02d:%02d", GetTime.Hours, GetTime.Minutes, GetTime.Seconds);
        osDelay(100);
    }
}
void RtcGetTime(void *argument)
{

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RX_Data, 100);
    RTC_TimeTypeDef GetTime = {0};
    RTC_DateTypeDef GetDate = {0};
    for (;;) {
        HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &GetDate, RTC_FORMAT_BIN);
        xQueueSend(TimeQueue, &GetTime, 0);
        xQueueSend(DateQueue, &GetDate, 0);
        //HAL_UART_Transmit(&huart1, "hell\n", 6,100);
        osDelay(500);
    }
}

//* 中断程序
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart1) {
        HAL_UART_Transmit_DMA(huart, RX_Data, Size);
        if (RX_Data[0] == 'S' && RX_Data[1] == 'D') {
            sscanf(RX_Data, "SD:%d-%d-%d", (int *)&SetDate.Year, (int *)&SetDate.Month, (int *)&SetDate.Date);
            HAL_RTC_SetDate(&hrtc, &SetDate, RTC_FORMAT_BIN);
        } else if (RX_Data[0] == 'S' && RX_Data[1] == 'T') {
            sscanf(RX_Data, "ST:%d:%d:%d", (int *)&SetTime.Hours, (int*)&SetTime.Minutes, (int *)&SetTime.Seconds);
            HAL_RTC_SetTime(&hrtc, &SetTime, RTC_FORMAT_BIN);
        }
        HAL_UARTEx_ReceiveToIdle_DMA(huart, RX_Data, 100);
    }
}
/* USER CODE END Application */
