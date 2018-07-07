/**
  ******************************************************************************
  * @file    stm32f37x_sdadc.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file contains all the functions prototypes for the SDADC firmware 
  *          library.
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
#ifndef __STM32F37X_SDADC_H
#define __STM32F37X_SDADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f37x.h"

/** @addtogroup STM32F37x_StdPeriph_Driver
  * @{
  */

/** @addtogroup SDADC
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  SDADC Init structure definition  
  */

typedef struct
{
  uint32_t SDADC_Channel;                    /*!< Select the regular channel.
                                                  This parameter can be any value of @ref SDADC_Channel_Selection */

  FunctionalState SDADC_ContinuousConvMode;  /*!< Specifies whether the conversion is performed in
                                                  Continuous or Single mode.
                                                  This parameter can be set to ENABLE or DISABLE. */

  FunctionalState SDADC_FastConversionMode;  /*!< Specifies whether the conversion is performed in
                                                  fast mode.
                                                  This parameter can be set to ENABLE or DISABLE. */

}SDADC_InitTypeDef;

/** 
  * @brief  SDADC Analog Inputs Configuration structure definition  
  */

typedef struct
{
  uint32_t SDADC_InputMode;      /*!< Specifies the input structure type (single ended, differential...)
                                      This parameter can be any value of @ref SDADC_InputMode */

  uint32_t SDADC_Gain;           /*!< Specifies the gain setting.
                                      This parameter can be any value of @ref SDADC_Gain */

  uint32_t SDADC_CommonMode;     /*!< Specifies the common mode setting (VSSA, VDDA, VDDA/2).
                                      This parameter can be any value of @ref SDADC_CommonMode */

  uint32_t SDADC_Offset;         /*!< Specifies the 12-bit offset value.
                                      This parameter can be any value lower or equal to 0x00000FFF */
}SDADC_AINStructTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup SDADC_Exported_Constants
  * @{
  */

#define IS_SDADC_ALL_PERIPH(PERIPH) (((PERIPH) == SDADC1) || \
                                     ((PERIPH) == SDADC2) || \
                                     ((PERIPH) == SDADC3))

#define IS_SDADC_SLAVE_PERIPH(PERIPH) (((PERIPH) == SDADC2) || \
                                       ((PERIPH) == SDADC3))

/** @defgroup SDADC_Channel_Selection
  * @{
  */

/* SDADC Channels ------------------------------------------------------------*/
/* The SDADC channels are defined as follow:
   - in 16-bit LSB the channel mask is set
   - in 16-bit MSB the channel number is set 
   e.g. for channel 5 definition:  
        - the channel mask is 0x00000020 (bit 5 is set) 
        - the channel number 5 is 0x00050000 
        --> Consequently, channel 5 definition is 0x00000020 | 0x00050000 = 0x00050020 */
#define SDADC_Channel_0                              ((uint32_t)0x00000001)
#define SDADC_Channel_1                              ((uint32_t)0x00010002)
#define SDADC_Channel_2                              ((uint32_t)0x00020004)
#define SDADC_Channel_3                              ((uint32_t)0x00030008)
#define SDADC_Channel_4                              ((uint32_t)0x00040010)
#define SDADC_Channel_5                              ((uint32_t)0x00050020)
#define SDADC_Channel_6                              ((uint32_t)0x00060040)
#define SDADC_Channel_7                              ((uint32_t)0x00070080)
#define SDADC_Channel_8                              ((uint32_t)0x00080100)

/* Just one channel of the 9 channels can be selected for regular conversion */
#define IS_SDADC_REGULAR_CHANNEL(CHANNEL) (((CHANNEL) == SDADC_Channel_0)  || \
                                           ((CHANNEL) == SDADC_Channel_1)  || \
                                           ((CHANNEL) == SDADC_Channel_2)  || \
                                           ((CHANNEL) == SDADC_Channel_3)  || \
                                           ((CHANNEL) == SDADC_Channel_4)  || \
                                           ((CHANNEL) == SDADC_Channel_5)  || \
                                           ((CHANNEL) == SDADC_Channel_6)  || \
                                           ((CHANNEL) == SDADC_Channel_7)  || \
                                           ((CHANNEL) == SDADC_Channel_8))

/* Any or all of the 9 channels can be selected for injected conversion */
#define IS_SDADC_INJECTED_CHANNEL(CHANNEL) (((CHANNEL) != 0) && ((CHANNEL) <= 0x000F01FF))

/**
  * @}
  */

/** @defgroup SDADC_Conf 
  * @{
  */
  
#define SDADC_Conf_0                              ((uint32_t)0x00000000) /*!< Configuration 0 selected */
#define SDADC_Conf_1                              ((uint32_t)0x00000001) /*!< Configuration 1 selected */
#define SDADC_Conf_2                              ((uint32_t)0x00000002) /*!< Configuration 2 selected */

#define IS_SDADC_CONF(CONF) (((CONF) == SDADC_Conf_0)  || \
                             ((CONF) == SDADC_Conf_1)  || \
                             ((CONF) == SDADC_Conf_2))

/**
  * @}
  */

/** @defgroup SDADC_InputMode
  * @{
  */

#define SDADC_InputMode_Diff                      ((uint32_t)0x00000000) /*!< Conversions are executed in differential mode */
#define SDADC_InputMode_SEOffset                  SDADC_CONF0R_SE0_0     /*!< Conversions are executed in single ended offset mode */
#define SDADC_InputMode_SEZeroReference           SDADC_CONF0R_SE0       /*!< Conversions are executed in single ended zero-volt reference mode */

#define IS_SDADC_INPUT_MODE(MODE) (((MODE) == SDADC_InputMode_Diff)     || \
                                   ((MODE) == SDADC_InputMode_SEOffset) || \
                                   ((MODE) == SDADC_InputMode_SEZeroReference))
/**
  * @}
  */

/** @defgroup SDADC_Gain 
  * @{
  */

#define SDADC_Gain_1                              ((uint32_t)0x00000000)  /*!< Gain equal to 1 */
#define SDADC_Gain_2                              SDADC_CONF0R_GAIN0_0    /*!< Gain equal to 2 */
#define SDADC_Gain_4                              SDADC_CONF0R_GAIN0_1    /*!< Gain equal to 4 */
#define SDADC_Gain_8                              ((uint32_t)0x00300000)  /*!< Gain equal to 8 */
#define SDADC_Gain_16                             SDADC_CONF0R_GAIN0_2    /*!< Gain equal to 16 */
#define SDADC_Gain_32                             ((uint32_t)0x00500000)  /*!< Gain equal to 32 */
#define SDADC_Gain_1_2                            SDADC_CONF0R_GAIN0      /*!< Gain equal to 1/2 */

#define IS_SDADC_GAIN(GAIN) (((GAIN) == SDADC_Gain_1)  || \
                             ((GAIN) == SDADC_Gain_2)  || \
                             ((GAIN) == SDADC_Gain_4)  || \
                             ((GAIN) == SDADC_Gain_8)  || \
                             ((GAIN) == SDADC_Gain_16)  || \
                             ((GAIN) == SDADC_Gain_32)  || \
                             ((GAIN) == SDADC_Gain_1_2))

/**
  * @}
  */

/** @defgroup SDADC_CommonMode
  * @{
  */

#define SDADC_CommonMode_VSSA                      ((uint32_t)0x00000000) /*!< Select SDADC VSSA as common mode */
#define SDADC_CommonMode_VDDA_2                    SDADC_CONF0R_COMMON0_0 /*!< Select SDADC VDDA/2 as common mode */
#define SDADC_CommonMode_VDDA                      SDADC_CONF0R_COMMON0_1 /*!< Select SDADC VDDA as common mode */

#define IS_SDADC_COMMON_MODE(MODE) (((MODE) == SDADC_CommonMode_VSSA)   || \
                                    ((MODE) == SDADC_CommonMode_VDDA_2) || \
                                    ((MODE) == SDADC_CommonMode_VDDA))

/**
  * @}
  */

/** @defgroup SDADC_Offset
  * @{
  */

#define IS_SDADC_OFFSET_VALUE(VALUE) ((VALUE) <= 0x00000FFF)

/**
  * @}
  */

/** @defgroup SDADC_ExternalTrigger_sources
  * @{
  */
#define SDADC_ExternalTrigInjecConv_T13_CC1               ((uint32_t)0x00000000) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T14_CC1               ((uint32_t)0x00000100) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T16_CC1               ((uint32_t)0x00000000) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T17_CC1               ((uint32_t)0x00000000) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T12_CC1               ((uint32_t)0x00000100) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T12_CC2               ((uint32_t)0x00000100) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T15_CC2               ((uint32_t)0x00000200) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T2_CC3                ((uint32_t)0x00000200) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T2_CC4                ((uint32_t)0x00000200) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T3_CC1                ((uint32_t)0x00000300) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T3_CC2                ((uint32_t)0x00000300) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T3_CC3                ((uint32_t)0x00000300) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T4_CC1                ((uint32_t)0x00000400) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T4_CC2                ((uint32_t)0x00000400) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T4_CC3                ((uint32_t)0x00000400) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_T19_CC2               ((uint32_t)0x00000500) /*!< Trigger source for SDADC1 */
#define SDADC_ExternalTrigInjecConv_T19_CC3               ((uint32_t)0x00000500) /*!< Trigger source for SDADC2 */
#define SDADC_ExternalTrigInjecConv_T19_CC4               ((uint32_t)0x00000500) /*!< Trigger source for SDADC3 */
#define SDADC_ExternalTrigInjecConv_Ext_IT11              ((uint32_t)0x00000700) /*!< Trigger source for SDADC1, SDADC2 and SDADC3 */
#define SDADC_ExternalTrigInjecConv_Ext_IT15              ((uint32_t)0x00000600) /*!< Trigger source for SDADC1, SDADC2 and SDADC3 */

#define IS_SDADC_EXT_INJEC_TRIG(INJTRIG) (((INJTRIG) == SDADC_ExternalTrigInjecConv_T13_CC1) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T14_CC1) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T16_CC1) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T17_CC1) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T12_CC1) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T12_CC2)  || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T15_CC2)  || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T2_CC3)  || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T2_CC4)  || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T3_CC1)  || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T3_CC2)  || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T3_CC3) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T4_CC1) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T4_CC2) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T4_CC3) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T19_CC2) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T19_CC3) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_T19_CC4) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_Ext_IT11) || \
                                          ((INJTRIG) == SDADC_ExternalTrigInjecConv_Ext_IT15))
/**
  * @}
  */

/** @defgroup SDADC_external_trigger_edge_for_injected_channels_conversion 
  * @{
  */ 
#define SDADC_ExternalTrigInjecConvEdge_None               ((uint32_t) 0x00000000)
#define SDADC_ExternalTrigInjecConvEdge_Rising             SDADC_CR2_JEXTEN_0
#define SDADC_ExternalTrigInjecConvEdge_Falling            SDADC_CR2_JEXTEN_1
#define SDADC_ExternalTrigInjecConvEdge_RisingFalling      SDADC_CR2_JEXTEN

#define IS_SDADC_EXT_INJEC_TRIG_EDGE(EDGE) (((EDGE) == SDADC_ExternalTrigInjecConvEdge_None)    || \
                                            ((EDGE) == SDADC_ExternalTrigInjecConvEdge_Rising)  || \
                                            ((EDGE) == SDADC_ExternalTrigInjecConvEdge_Falling) || \
                                            ((EDGE) == SDADC_ExternalTrigInjecConvEdge_RisingFalling))
/**
  * @}
  */ 

/** @defgroup SDADC_DMATransfer_modes 
  * @{
  */ 
#define SDADC_DMATransfer_Regular                SDADC_CR1_RDMAEN          /*!< DMA requests enabled for regular conversions */
#define SDADC_DMATransfer_Injected               SDADC_CR1_JDMAEN          /*!< DMA requests enabled for injected conversions */

#define IS_SDADC_DMA_TRANSFER(TRANSFER)  (((TRANSFER) == SDADC_DMATransfer_Regular)  || \
                                          ((TRANSFER) == SDADC_DMATransfer_Injected))
/**
  * @}
  */

/** @defgroup SDADC_CalibrationSequence 
  * @{
  */ 
#define SDADC_CalibrationSequence_1                   ((uint32_t)0x00000000) /*!< One calibration sequence to calculate offset of conf0 (OFFSET0[11:0]) */
#define SDADC_CalibrationSequence_2                   SDADC_CR2_CALIBCNT_0   /*!< Two calibration sequences to calculate offset of conf0 and conf1 (OFFSET0[11:0] and OFFSET1[11:0]) */
#define SDADC_CalibrationSequence_3                   SDADC_CR2_CALIBCNT_1   /*!< Three calibration sequences to calculate offset of conf0, conf1 and conf2 (OFFSET0[11:0], OFFSET1[11:0], and OFFSET2[11:0]) */

#define IS_SDADC_CALIB_SEQUENCE(SEQUENCE)  (((SEQUENCE) == SDADC_CalibrationSequence_1)  || \
                                            ((SEQUENCE) == SDADC_CalibrationSequence_2)  || \
                                            ((SEQUENCE) == SDADC_CalibrationSequence_3))
/**
  * @}
  */

/** @defgroup SDADC_VREF
  * @{
  */

#define SDADC_VREF_Ext                            ((uint32_t)0x00000000) /*!< The reference voltage is forced externally using VREF pin */
#define SDADC_VREF_VREFINT1                       SDADC_CR1_REFV_0       /*!< The reference voltage is forced internally to 1.22V VREFINT */
#define SDADC_VREF_VREFINT2                       SDADC_CR1_REFV_1       /*!< The reference voltage is forced internally to 1.8V VREFINT */
#define SDADC_VREF_VDDA                           SDADC_CR1_REFV         /*!< The reference voltage is forced internally to VDDA */

#define IS_SDADC_VREF(VREF) (((VREF) == SDADC_VREF_Ext)      || \
                             ((VREF) == SDADC_VREF_VREFINT1) || \
                             ((VREF) == SDADC_VREF_VREFINT2) || \
                             ((VREF) == SDADC_VREF_VDDA))

/**
  * @}
  */

/** @defgroup SDADC_interrupts_definition
  * @{
  */

#define SDADC_IT_EOCAL                               ((uint32_t)0x00000001) /*!< End of calibration flag */
#define SDADC_IT_JEOC                                ((uint32_t)0x00000002) /*!< End of injected conversion flag */
#define SDADC_IT_JOVR                                ((uint32_t)0x00000004) /*!< Injected conversion overrun flag */
#define SDADC_IT_REOC                                ((uint32_t)0x00000008) /*!< End of regular conversion flag */
#define SDADC_IT_ROVR                                ((uint32_t)0x00000010) /*!< Regular conversion overrun flag */

#define IS_SDADC_IT(IT) ((((IT) & (uint32_t)0xFFFFFFE0) == 0x00000000) && ((IT) != 0x00000000))

#define IS_SDADC_GET_IT(IT) (((IT) == SDADC_IT_EOCAL) || ((IT) == SDADC_IT_JEOC) || \
                             ((IT) == SDADC_IT_JOVR)  || ((IT) == SDADC_IT_REOC) || \
                             ((IT) == SDADC_IT_ROVR))

#define IS_SDADC_CLEAR_IT(IT) ((((IT) & (uint32_t)0xFFFFFFEA) == 0x00000000) && ((IT) != 0x00000000))

/**
  * @}
  */

/** @defgroup SDADC_flags_definition
  * @{
  */

#define SDADC_FLAG_EOCAL                             ((uint32_t)0x00000001) /*!< End of calibration flag */
#define SDADC_FLAG_JEOC                              ((uint32_t)0x00000002) /*!< End of injected conversion flag */
#define SDADC_FLAG_JOVR                              ((uint32_t)0x00000004) /*!< Injected conversion overrun flag */
#define SDADC_FLAG_REOC                              ((uint32_t)0x00000008) /*!< End of regular conversion flag */
#define SDADC_FLAG_ROVR                              ((uint32_t)0x00000010) /*!< Regular conversion overrun flag */
#define SDADC_FLAG_CALIBIP                           ((uint32_t)0x00001000) /*!< Calibration in progress status */
#define SDADC_FLAG_JCIP                              ((uint32_t)0x00002000) /*!< Injected conversion in progress status */
#define SDADC_FLAG_RCIP                              ((uint32_t)0x00004000) /*!< Regular conversion in progress status */
#define SDADC_FLAG_STABIP                            ((uint32_t)0x00008000) /*!< Stabilization in progress status */
#define SDADC_FLAG_INITRDY                           ((uint32_t)0x80000000) /*!< Initialization mode is ready */

#define IS_SDADC_CLEAR_FLAG(FLAG) ((((FLAG) & (uint32_t)0xFFFFFFE0) == 0x00000000) && ((FLAG) != 0x00000000))

#define IS_SDADC_GET_FLAG(FLAG) (((FLAG) == SDADC_FLAG_EOCAL) || ((FLAG) == SDADC_FLAG_JEOC)   || \
                                 ((FLAG) == SDADC_FLAG_JOVR)  || ((FLAG)== SDADC_FLAG_REOC)    || \
                                 ((FLAG) == SDADC_FLAG_ROVR)  || ((FLAG)== SDADC_FLAG_CALIBIP) || \
                                 ((FLAG) == SDADC_FLAG_JCIP)  || ((FLAG)== SDADC_FLAG_RCIP)    || \
                                 ((FLAG) == SDADC_FLAG_STABIP)  || ((FLAG)== SDADC_FLAG_INITRDY))


/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/* Function used to set the SDADC configuration to the default reset state ****/
void SDADC_DeInit(SDADC_TypeDef* SDADCx);

/* Initialization and Configuration functions *********************************/
void SDADC_Init(SDADC_TypeDef* SDADCx, SDADC_InitTypeDef* SDADC_InitStruct);
void SDADC_StructInit(SDADC_InitTypeDef* SDADC_InitStruct);
void SDADC_AINInit(SDADC_TypeDef* SDADCx, uint32_t SDADC_Conf, SDADC_AINStructTypeDef* SDADC_AINStruct);
void SDADC_AINStructInit(SDADC_AINStructTypeDef* SDADC_AINStruct);
void SDADC_ChannelConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_Channel, uint32_t SDADC_Conf);
void SDADC_Cmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);
void SDADC_InitModeCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);
void SDADC_FastConversionCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);
void SDADC_VREFSelect(uint32_t SDADC_VREF);
void SDADC_CalibrationSequenceConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_CalibrationSequence);
void SDADC_StartCalibration(SDADC_TypeDef* SDADCx);

/* Regular Channels Configuration functions ***********************************/
void SDADC_ChannelSelect(SDADC_TypeDef* SDADCx, uint32_t SDADC_Channel);
void SDADC_ContinuousModeCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);
void SDADC_SoftwareStartConv(SDADC_TypeDef* SDADCx);
int16_t SDADC_GetConversionValue(SDADC_TypeDef* SDADCx);
void SDADC_RegularSynchroSDADC1(SDADC_TypeDef* SDADCx, FunctionalState NewState);
uint32_t SDADC_GetConversionSDADC12Value(void);
uint32_t SDADC_GetConversionSDADC13Value(void);

/* Injected channels Configuration functions **********************************/
void SDADC_SoftwareStartInjectedConv(SDADC_TypeDef* SDADCx);
void SDADC_InjectedChannelSelect(SDADC_TypeDef* SDADCx, uint32_t SDADC_Channel);
void SDADC_DelayStartInjectedConvCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);
void SDADC_InjectedContinuousModeCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);
void SDADC_ExternalTrigInjectedConvConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_ExternalTrigInjecConv);
void SDADC_ExternalTrigInjectedConvEdgeConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_ExternalTrigInjecConvEdge);
uint32_t SDADC_GetInjectedChannel(SDADC_TypeDef* SDADCx);
int16_t SDADC_GetInjectedConversionValue(SDADC_TypeDef* SDADCx, uint32_t* SDADC_Channel);
void SDADC_InjectedSynchroSDADC1(SDADC_TypeDef* SDADCx, FunctionalState NewState);
uint32_t SDADC_GetInjectedConversionSDADC12Value(void);
uint32_t SDADC_GetInjectedConversionSDADC13Value(void);

/* Power saving functions *****************************************************/
void SDADC_PowerDownCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);
void SDADC_StandbyCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);
void SDADC_SlowClockCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState);

/* Regular/Injected Channels DMA Configuration functions **********************/
void SDADC_DMAConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_DMATransfer, FunctionalState NewState);

/* Interrupts and flags management functions **********************************/
void SDADC_ITConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT, FunctionalState NewState);
FlagStatus SDADC_GetFlagStatus(SDADC_TypeDef* SDADCx, uint32_t SDADC_FLAG);
void SDADC_ClearFlag(SDADC_TypeDef* SDADCx, uint32_t SDADC_FLAG);
ITStatus SDADC_GetITStatus(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT);
void SDADC_ClearITPendingBit(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT);

#ifdef __cplusplus
}
#endif

#endif /*__STM32F37X_SDADC_H */

/**
  * @}
  */

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
