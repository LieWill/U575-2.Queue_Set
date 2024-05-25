/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    dma2d.h
 * @brief   This file contains all the function prototypes for
 *          the dma2d.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DMA2D_H__
#define __DMA2D_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern DMA2D_HandleTypeDef hdma2d;

/* USER CODE BEGIN Private defines */
#define RGB565 0x02
/* USER CODE END Private defines */

void MX_DMA2D_Init(void);

/* USER CODE BEGIN Prototypes */
void Dma2D_Fill(uint16_t *buffer, uint16_t width, uint16_t height, int LCD_PIXEL_WIDTH, uint32_t color);
void Dma2D_Memcopy(uint32_t pixelformat, const unsigned char *image, uint16_t *buff, int width, int height, int x, int y);
void Dma2D_Mixcolorsbulk(void *pfg, void *pbg, void *pdst,
                         uint32_t offlinefg, uint32_t offlinebg, uint32_t offlinedist,
                         uint16_t width, uint16_t height,
                         uint8_t opa);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __DMA2D_H__ */

