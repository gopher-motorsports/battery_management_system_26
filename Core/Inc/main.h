/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MCU_FAULT_Pin GPIO_PIN_0
#define MCU_FAULT_GPIO_Port GPIOA
#define MCU_HEART_Pin GPIO_PIN_2
#define MCU_HEART_GPIO_Port GPIOA
#define PORTB_CS_Pin GPIO_PIN_3
#define PORTB_CS_GPIO_Port GPIOA
#define PORTA_CS_Pin GPIO_PIN_4
#define PORTA_CS_GPIO_Port GPIOA
#define ISO_SPI_SCK_Pin GPIO_PIN_5
#define ISO_SPI_SCK_GPIO_Port GPIOA
#define ISO_SPI_MISO_Pin GPIO_PIN_6
#define ISO_SPI_MISO_GPIO_Port GPIOA
#define ISO_SPI_MOSI_Pin GPIO_PIN_7
#define ISO_SPI_MOSI_GPIO_Port GPIOA
#define PACK_MON_CS_N_Pin GPIO_PIN_12
#define PACK_MON_CS_N_GPIO_Port GPIOB
#define PACK_MON_MISO_Pin GPIO_PIN_14
#define PACK_MON_MISO_GPIO_Port GPIOB
#define PACK_MON_MOSI_Pin GPIO_PIN_15
#define PACK_MON_MOSI_GPIO_Port GPIOB
#define PACK_MON_SCK_Pin GPIO_PIN_7
#define PACK_MON_SCK_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define BMS_FAULT_READ_Pin GPIO_PIN_10
#define BMS_FAULT_READ_GPIO_Port GPIOC
#define BMS_FAULT_Pin GPIO_PIN_11
#define BMS_FAULT_GPIO_Port GPIOC
#define IMD_FAULT_READ_Pin GPIO_PIN_12
#define IMD_FAULT_READ_GPIO_Port GPIOC
#define BMS_INB_N_Pin GPIO_PIN_2
#define BMS_INB_N_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
