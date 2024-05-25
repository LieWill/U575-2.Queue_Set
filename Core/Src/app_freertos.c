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
#include "MyHead.h"
#include "CST816T.h"
#include <string.h>
#include "st7789v.h"
#include "image.h"
#include "dma2d.h"
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
extern int fps;
extern uint16_t buffer[240 * 280];
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
void GetTouch(void *argument);
void LcdShow(void *argument);
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
    // xTaskCreate(OledShowTask, "OledShowTask", 256, NULL, osPriorityNormal, NULL);
    xTaskCreate(LcdShow, "LcdShow", 256, NULL, osPriorityNormal, NULL);
    xTaskCreate(RtcGetTime, "RtcGetTime", 256, NULL, osPriorityNormal, NULL);
    xTaskCreate(GetTouch, "GetTouch", 256, NULL, osPriorityNormal, NULL);
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
//* ??????????????????????????????
//!! ????????????????????????
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
        if (wake) {
            OLED_Printf(4, 1, "fps=%d  ", fps);
            wake = 0;
            fps  = 0;
        }
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
        //^ ????????
        if (GetTime.Hours == 0 && GetDate.Month == 0 && GetDate.Date == 0)
            xEventGroupSetBits(RTCEvent, SendDate);
        //^ ?????????????????
        if (xEventGroupWaitBits(RTCEvent, SendDate, pdTRUE, pdFALSE, 10) != SendDate) {
            xQueueSend(DateQueue, &GetDate, 0);
        }
        xQueueSend(TimeQueue, &GetTime, 0);
        osDelay(500);
    }
}

/**
 * @brief ?????str????????target
 * @param str ????????
 * @param target ?????
 * @note str?target??????????????'*'???????????????
 * @return ????1??????0
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

void GetTouch(void *argument)
{
    uint16_t x = 0, y = 0, Gesture = 0;
    int touch       = 0;
    TouchType Touch = {0};
    Touch.X         = 100;
    while (1) {
        if (CST816_GetAction(&x, &y, &Gesture)) {
            if (touch == 0) {
                touch = 1;
                OLED_Clear();
            }
            // osDelay(10);
            OLED_Printf(1, 1, "x=%d  ", x);
            OLED_Printf(2, 1, "y=%d  ", y);
            OLED_Printf(3, 1, "Gesture=%d  ", Gesture);
        } else {
            if (touch == 1) {
                touch = 0;
                OLED_Clear();
            }
            OLED_Printf(1, 1, "NO TOUCH");
        }
        osDelay(0);
    }
}

uint16_t buffer2[240 * 280] = {0};

void LcdShow(void *argument)
{
    int cot = 0;
    while (1) {
        float speed = 5;
        for (int i = 210; i >= 0 && i <= 210; i -= speed, speed -= 0.1) {
            if (cot % 2 == 1) {
                Dma2D_Fill(buffer, 240, 280, 0, RGB888toRGB565(56, 52, 41));
                Dma2D_Memcopy(RGB565, gImage_MyHead, buffer, 240, 240, 0, 20);
                // Dma2D_Memcopy(RGB565, gImage_1, buffer, 60, 60, 75, i);
                Dma2D_Mixcolorsbulk(gImage_1, gImage_MyHead + 50 + i * 240, buffer + 240 * 10 + 50 + i * 240, 0, 180, 180, 60, 60, 255);
                LCD_drawImage(buffer2, 0, 0, 240, 280);
                fps++;
                cot++;
            } else {
                Dma2D_Fill(buffer2, 240, 280, 0, RGB888toRGB565(56, 52, 41));
                Dma2D_Memcopy(RGB565, gImage_MyHead, buffer2, 240, 240, 0, 20);
                // Dma2D_Memcopy(RGB565, gImage_1, buffer, 60, 60, 75, i);
                Dma2D_Mixcolorsbulk(gImage_1, gImage_MyHead + 50 + i * 240, buffer2 + 240 * 10 + 50 + i * 240, 0, 180, 180, 60, 60, 255);
                LCD_drawImage(buffer, 0, 0, 240, 280);
                fps++;
                cot++;
            }
        }
    }
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    wake++;
    // fps = 0;
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
