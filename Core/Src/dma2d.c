/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    dma2d.c
 * @brief   This file provides code for the configuration
 *          of the DMA2D instances.
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
#include "dma2d.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

DMA2D_HandleTypeDef hdma2d;

/* DMA2D init function */
void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.Init.BytesSwap = DMA2D_BYTES_REGULAR;
  hdma2d.Init.LineOffsetMode = DMA2D_LOM_PIXELS;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspInit 0 */

  /* USER CODE END DMA2D_MspInit 0 */
    /* DMA2D clock enable */
    __HAL_RCC_DMA2D_CLK_ENABLE();
  /* USER CODE BEGIN DMA2D_MspInit 1 */

  /* USER CODE END DMA2D_MspInit 1 */
  }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspDeInit 0 */

  /* USER CODE END DMA2D_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DMA2D_CLK_DISABLE();
  /* USER CODE BEGIN DMA2D_MspDeInit 1 */

  /* USER CODE END DMA2D_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void Dma2D_Fill(uint16_t *buffer, uint16_t width, uint16_t height, int lineoffect, uint32_t color)
{
    // while (DMA2D->CR & DMA2D_CR_START) {}
    /* DMA2D配置 */
    DMA2D->CR     = 0x00030000UL;                               // 配置为寄存器到储存器模式
    DMA2D->OCOLR  = color;                                      // 设置填充使用的颜�???
    DMA2D->OMAR   = (uint32_t)buffer;                           // 填充区域的起始内存地�???
    DMA2D->OOR    = lineoffect;                                 // 行偏移，即跳过的像素(像素为单�???)
    DMA2D->OPFCCR = 0x2;                                        // 设置颜色格式
    DMA2D->NLR    = (uint32_t)(width << 16) | (uint16_t)height; // 设置填充区域的宽和高，单位是像素
    /* 启动传输 */
    DMA2D->CR |= DMA2D_CR_START;
    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START) {}
}

void Dma2D_Memcopy(uint32_t pixelformat, const unsigned char *image, uint16_t *buff, int width, int height, int x, int y)
{
    // while (DMA2D->CR & DMA2D_CR_START) {}
    DMA2D->CR      = 0x00000000UL;
    DMA2D->FGMAR   = (uint32_t)image;
    DMA2D->OMAR    = (uint32_t)((uint16_t *)buff + y * 240 + x);
    DMA2D->FGOR    = 0;
    DMA2D->OOR     = 240 - width;
    DMA2D->FGPFCCR = pixelformat;
    DMA2D->NLR     = (uint32_t)(width << 16) | (uint16_t)height;
    DMA2D->CR |= DMA2D_CR_START;

    while (DMA2D->CR & DMA2D_CR_START) {}
}

void Dma2D_Mixcolorsbulk(void *pfg, void *pbg, void *pdst,
                         uint32_t offlinefg, uint32_t offlinebg, uint32_t offlinedist,
                         uint16_t width, uint16_t height,
                         uint8_t opa)
{ // opa 透明�?

    DMA2D->CR = 0x00020000UL; //^ 混合模式

    DMA2D->FGMAR = (uint32_t)pfg;
    DMA2D->BGMAR = (uint32_t)pbg;
    DMA2D->OMAR  = (uint32_t)pdst;

    DMA2D->FGOR = offlinefg;
    DMA2D->BGOR = offlinebg;
    DMA2D->OOR  = offlinedist;

    DMA2D->NLR = (uint32_t)(width << 16) | (uint16_t)height;

    DMA2D->FGPFCCR = RGB565 | (1UL << 16) | ((uint32_t)opa << 24);

    DMA2D->BGPFCCR = RGB565;
    DMA2D->OPFCCR  = RGB565;

    DMA2D->CR |= DMA2D_CR_START;

    while (DMA2D->CR & DMA2D_CR_START) {}
}
/* USER CODE END 1 */
