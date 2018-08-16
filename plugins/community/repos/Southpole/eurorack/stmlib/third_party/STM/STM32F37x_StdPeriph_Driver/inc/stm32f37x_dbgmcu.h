/**
  ******************************************************************************
  * @file    stm32f37x_dbgmcu.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file contains all the functions prototypes for the DBGMCU firmware library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F37X_DBGMCU_H
#define __STM32F37X_DBGMCU_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f37x.h"

/** @addtogroup STM32F37x_StdPeriph_Driver
  * @{
  */

/** @addtogroup DBGMCU
  * @{
  */ 

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @defgroup DBGMCU_Exported_Constants
  * @{
  */ 
#define DBGMCU_SLEEP                          DBGMCU_CR_DBG_SLEEP
#define DBGMCU_STOP                           DBGMCU_CR_DBG_STOP
#define DBGMCU_STANDBY                        DBGMCU_CR_DBG_STANDBY
#define IS_DBGMCU_PERIPH(PERIPH) ((((PERIPH) & 0xFFFFFFF8) == 0x00) && ((PERIPH) != 0x00))

#define DBGMCU_TIM2_STOP             DBGMCU_APB1_FZ_DBG_TIM2_STOP
#define DBGMCU_TIM3_STOP             DBGMCU_APB1_FZ_DBG_TIM3_STOP
#define DBGMCU_TIM4_STOP             DBGMCU_APB1_FZ_DBG_TIM4_STOP
#define DBGMCU_TIM5_STOP             DBGMCU_APB1_FZ_DBG_TIM5_STOP
#define DBGMCU_TIM6_STOP             DBGMCU_APB1_FZ_DBG_TIM6_STOP
#define DBGMCU_TIM7_STOP             DBGMCU_APB1_FZ_DBG_TIM7_STOP
#define DBGMCU_TIM12_STOP            DBGMCU_APB1_FZ_DBG_TIM12_STOP
#define DBGMCU_TIM13_STOP            DBGMCU_APB1_FZ_DBG_TIM13_STOP
#define DBGMCU_TIM14_STOP            DBGMCU_APB1_FZ_DBG_TIM14_STOP
#define DBGMCU_TIM18_STOP            DBGMCU_APB1_FZ_DBG_TIM18_STOP
#define DBGMCU_RTC_STOP              DBGMCU_APB1_FZ_DBG_RTC_STOP
#define DBGMCU_WWDG_STOP             DBGMCU_APB1_FZ_DBG_WWDG_STOP
#define DBGMCU_IWDG_STOP             DBGMCU_APB1_FZ_DBG_IWDG_STOP
#define DBGMCU_I2C1_SMBUS_TIMEOUT    DBGMCU_APB1_FZ_DBG_I2C1_SMBUS_TIMEOUT
#define DBGMCU_I2C2_SMBUS_TIMEOUT    DBGMCU_APB1_FZ_DBG_I2C2_SMBUS_TIMEOUT
#define DBGMCU_CAN1_STOP             DBGMCU_APB1_FZ_DBG_CAN1_STOP

#define IS_DBGMCU_APB1PERIPH(PERIPH) ((((PERIPH) & 0xFD9FE000) == 0x00) && ((PERIPH) != 0x00))

#define DBGMCU_TIM15_STOP            DBGMCU_APB2_FZ_DBG_TIM15_STOP
#define DBGMCU_TIM16_STOP            DBGMCU_APB2_FZ_DBG_TIM16_STOP
#define DBGMCU_TIM17_STOP            DBGMCU_APB2_FZ_DBG_TIM17_STOP
#define DBGMCU_TIM19_STOP            DBGMCU_APB2_FZ_DBG_TIM19_STOP
#define IS_DBGMCU_APB2PERIPH(PERIPH) ((((PERIPH) & 0xFFFFFFC3) == 0x00) && ((PERIPH) != 0x00))

/**
  * @}
  */ 

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 

/* Device and Revision ID management functions ********************************/
uint32_t DBGMCU_GetREVID(void);
uint32_t DBGMCU_GetDEVID(void);

/* Peripherals Configuration functions ****************************************/
void DBGMCU_Config(uint32_t DBGMCU_Periph, FunctionalState NewState);
void DBGMCU_APB1PeriphConfig(uint32_t DBGMCU_Periph, FunctionalState NewState);
void DBGMCU_APB2PeriphConfig(uint32_t DBGMCU_Periph, FunctionalState NewState);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F37X_DBGMCU_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
