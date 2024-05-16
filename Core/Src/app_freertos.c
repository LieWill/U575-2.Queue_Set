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
#include <stdio.h>
#include "usart.h"
#include "OLED.H"
#include "rtc.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define SendDate 0x02
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
QueueHandle_t TimeQueue, DateQueue;
RTC_TimeTypeDef SetTime;
RTC_DateTypeDef SetDate;
QueueSetHandle_t QueueSet1;
EventGroupHandle_t RTCEvent;
uint8_t RX_Data[100];
int wake = 0;

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
char StringDetect(char *str, char *target);
/* USER CODE END FunctionPrototypes */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
    OLED_Init();
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RX_Data, 100);
    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    // xSemaphoreCreateBinary(Semaphore1);
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    TimeQueue = xQueueCreate(1, sizeof(RTC_TimeTypeDef));
    DateQueue = xQueueCreate(1, sizeof(RTC_DateTypeDef));
    if (TimeQueue == NULL || DateQueue == NULL) {
        OLED_Printf(1, 1, "Queue Create Failed");
        while (1);
    }
    QueueSet1 = xQueueCreateSet(2);
    if (QueueSet1 == NULL) {
        OLED_Printf(1, 1, "QueueSet Failed");
        while (1);
    }
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
    RTCEvent = xEventGroupCreate();
    if (RTCEvent == NULL) {
        OLED_Printf(1, 1, "EventGroup Failed");
        while (1);
    }
    BaseType_t err = xQueueAddToSet(TimeQueue, QueueSet1);
    err            = xQueueAddToSet(DateQueue, QueueSet1);
    if (err != pdPASS) {
        OLED_Printf(1, 1, "Add failed");
        while (1);
    }
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
//* 把队列添加到队列集后，队列集会自动检测队列中的数据是否接收。
//!! 注意：读队列前，需要读队列集信息，不然无法读队列
void OledShowTask(void *argument)
{
    RTC_TimeTypeDef GetTime = {0};
    RTC_DateTypeDef GetDate = {0};
    OLED_Printf(1, 1, "The Date");
    for (;;) {
        QueueHandle_t temp = xQueueSelectFromSet(QueueSet1, portMAX_DELAY);
        if (temp == TimeQueue) {
            xQueueReceive(TimeQueue, &GetTime, portMAX_DELAY);
            OLED_Printf(3, 1, "%02d:%02d:%02d", GetTime.Hours, GetTime.Minutes, GetTime.Seconds);
        } else if (temp == DateQueue) {
            xQueueReceive(DateQueue, &GetDate, portMAX_DELAY);
            OLED_Printf(2, 1, "%02d/%02d/%02d", GetDate.Year, GetDate.Month, GetDate.Date);
        }
        OLED_Printf(4, 1, "wake=%d", wake);
        osDelay(100);
    }
}
void RtcGetTime(void *argument)
{
    RTC_TimeTypeDef GetTime = {0};
    RTC_DateTypeDef GetDate = {0};
    xEventGroupSetBits(RTCEvent, SendDate);
    for (;;) {
        HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &GetDate, RTC_FORMAT_BIN);
        //^ 每一天的日期更新
        if (GetTime.Hours == 0 && GetDate.Month == 0 && GetDate.Date == 0)
            xEventGroupSetBits(RTCEvent, SendDate);
        //^ 等待事件组超时就返回相应标志位的值
        if (xEventGroupWaitBits(RTCEvent, SendDate, pdTRUE, pdFALSE, 10) != SendDate) {
            xQueueSend(DateQueue, &GetDate, 0);
        }
        xQueueSend(TimeQueue, &GetTime, 0);
        osDelay(500);
    }
}

/**
 * @brief 检查字符串str中是否包含字符串target
 * @param str 需要检查的字符串
 * @param target 目标字符串
 * @note str和target中包含的字符串可以包含通配符'*'，表示任意字符或任意长度字符串
 * @return 相同返回1，不相同返回0
 */
char StringDetect(char *str, char *target)
{
    for (int i = 0, j = 0; str[i] != '\0'; i++, j++) {
        if (str[i] == '*') {
            i++;
            for (; target[j] != '\0'; j++) {
                if (str[i] == target[j])
                    break;
            }
        }
        if (str[i] != target[j])
            return 0;
    }
    return 1;
}

//* 中断程序
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    wake++;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart1) {
        if (StringDetect("SD:*-*-*", (char *)RX_Data)) {
            int year = 0;
            sscanf((char *)RX_Data, "SD:%d-%d-%d", (int *)&year, (int *)&SetDate.Month, (int *)&SetDate.Date);
            SetDate.Year = year;
            HAL_RTC_SetDate(&hrtc, &SetDate, RTC_FORMAT_BIN);
            xEventGroupSetBitsFromISR(RTCEvent, SendDate, NULL);
            HAL_UART_Transmit(&huart1, (uint8_t *)"Data setting successful\n", 24, 100);
        } else if (StringDetect("ST:*:*:*", (char *)RX_Data)) {
            sscanf((char *)RX_Data, "ST:%d:%d:%d", (int *)&SetTime.Hours, (int *)&SetTime.Minutes, (int *)&SetTime.Seconds);
            HAL_RTC_SetTime(&hrtc, &SetTime, RTC_FORMAT_BIN);
            HAL_UART_Transmit(&huart1, (uint8_t *)"Time setting successful\n", 24, 100);
        } else {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Command Not Found\n", 18, 100);
        }
        HAL_UARTEx_ReceiveToIdle_DMA(huart, RX_Data, 100);
    }
}
/* USER CODE END Application */
