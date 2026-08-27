/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#define RE_A_Pin GPIO_PIN_0
#define RE_A_GPIO_Port GPIOA
#define RE_B_Pin GPIO_PIN_1
#define RE_B_GPIO_Port GPIOA
#define BUZZ_Pin GPIO_PIN_0
#define BUZZ_GPIO_Port GPIOB
#define USS_FL_TRIG_Pin GPIO_PIN_10
#define USS_FL_TRIG_GPIO_Port GPIOB
#define USS_RR_TRIG_Pin GPIO_PIN_12
#define USS_RR_TRIG_GPIO_Port GPIOB
#define USS_FR_TRIG_Pin GPIO_PIN_7
#define USS_FR_TRIG_GPIO_Port GPIOC
#define USS_FL_ECHO_Pin GPIO_PIN_8
#define USS_FL_ECHO_GPIO_Port GPIOA
#define USS_FR_ECHO_Pin GPIO_PIN_9
#define USS_FR_ECHO_GPIO_Port GPIOA
#define USS_RL_ECHO_Pin GPIO_PIN_10
#define USS_RL_ECHO_GPIO_Port GPIOA
#define USS_RR_ECHO_Pin GPIO_PIN_11
#define USS_RR_ECHO_GPIO_Port GPIOA
#define USS_RL_TRIG_Pin GPIO_PIN_3
#define USS_RL_TRIG_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
