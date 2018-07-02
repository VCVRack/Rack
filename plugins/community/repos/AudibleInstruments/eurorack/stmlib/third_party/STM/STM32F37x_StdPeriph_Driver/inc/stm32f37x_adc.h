/**
  ******************************************************************************
  * @file    stm32f37x_adc.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file contains all the functions prototypes for the ADC firmware 
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
#ifndef __STM32F37X_ADC_H
#define __STM32F37X_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f37x.h"

/** @addtogroup STM32F37x_StdPeriph_Driver
  * @{
  */

/** @addtogroup ADC
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  ADC Init structure definition  
  */

typedef struct
{

  FunctionalState ADC_ScanConvMode;       /*!< Specifies whether the conversion is performed in
                                               Scan (multichannels) or Single (one channel) mode.
                                               This parameter can be set to ENABLE or DISABLE */

  FunctionalState ADC_ContinuousConvMode; /*!< Specifies whether the conversion is performed in
                                               Continuous or Single mode.
                                               This parameter can be set to ENABLE or DISABLE. */

  uint32_t ADC_ExternalTrigConv;          /*!< Defines the external trigger used to start the analog
                                               to digital conversion of regular channels. This parameter
                                               can be a value of @ref ADC_external_trigger_sources_for_regular_channels_conversion */

  uint32_t ADC_DataAlign;                 /*!< Specifies whether the ADC data alignment is left or right.
                                               This parameter can be a value of @ref ADC_data_align */

  uint8_t ADC_NbrOfChannel;               /*!< Specifies the number of ADC channels that will be converted
                                               using the sequencer for regular channel group.
                                               This parameter must range from 1 to 16. */
}ADC_InitTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup ADC_Exported_Constants
  * @{
  */ 

#define IS_ADC_ALL_PERIPH(PERIPH) ((PERIPH) == ADC1)

#define IS_ADC_DMA_PERIPH(PERIPH) ((PERIPH) == ADC1)

/** @defgroup ADC_external_trigger_sources_for_regular_channels_conversion 
  * @{
  */

#define ADC_ExternalTrigConv_T19_TRGO              ((uint32_t)0x00000000)
#define ADC_ExternalTrigConv_T19_CC3               ADC_CR2_EXTSEL_0
#define ADC_ExternalTrigConv_T19_CC4               ADC_CR2_EXTSEL_1
#define ADC_ExternalTrigConv_T2_CC2                ((uint32_t)0x00060000)
#define ADC_ExternalTrigConv_T3_TRGO               ADC_CR2_EXTSEL_2
#define ADC_ExternalTrigConv_T4_CC4                ((uint32_t)0x000A0000)
#define ADC_ExternalTrigConv_Ext_IT11              ((uint32_t)0x000C0000)
#define ADC_ExternalTrigConv_None                  ((uint32_t)0x000E0000)

#define IS_ADC_EXT_TRIG(REGTRIG) (((REGTRIG) == ADC_ExternalTrigConv_T19_TRGO)  || \
                                   ((REGTRIG) == ADC_ExternalTrigConv_T19_CC3)  || \
                                   ((REGTRIG) == ADC_ExternalTrigConv_T19_CC4)  || \
                                   ((REGTRIG) == ADC_ExternalTrigConv_T2_CC2)   || \
                                   ((REGTRIG) == ADC_ExternalTrigConv_T3_TRGO)  || \
                                   ((REGTRIG) == ADC_ExternalTrigConv_T4_CC4)   || \
                                   ((REGTRIG) == ADC_ExternalTrigConv_Ext_IT11) || \
                                   ((REGTRIG) == ADC_ExternalTrigConv_None))

/**
  * @}
  */

/** @defgroup ADC_data_align 
  * @{
  */

#define ADC_DataAlign_Right                        ((uint32_t)0x00000000)
#define ADC_DataAlign_Left                         ADC_CR2_ALIGN

#define IS_ADC_DATA_ALIGN(ALIGN) (((ALIGN) == ADC_DataAlign_Right) || \
                                  ((ALIGN) == ADC_DataAlign_Left))
/**
  * @}
  */

/** @defgroup ADC_channels 
  * @{
  */

#define ADC_Channel_0                               ((uint8_t)0x00)
#define ADC_Channel_1                               ((uint8_t)0x01)
#define ADC_Channel_2                               ((uint8_t)0x02)
#define ADC_Channel_3                               ((uint8_t)0x03)
#define ADC_Channel_4                               ((uint8_t)0x04)
#define ADC_Channel_5                               ((uint8_t)0x05)
#define ADC_Channel_6                               ((uint8_t)0x06)
#define ADC_Channel_7                               ((uint8_t)0x07)
#define ADC_Channel_8                               ((uint8_t)0x08)
#define ADC_Channel_9                               ((uint8_t)0x09)
#define ADC_Channel_10                              ((uint8_t)0x0A)
#define ADC_Channel_11                              ((uint8_t)0x0B)
#define ADC_Channel_12                              ((uint8_t)0x0C)
#define ADC_Channel_13                              ((uint8_t)0x0D)
#define ADC_Channel_14                              ((uint8_t)0x0E)
#define ADC_Channel_15                              ((uint8_t)0x0F)
#define ADC_Channel_16                              ((uint8_t)0x10)
#define ADC_Channel_17                              ((uint8_t)0x11)
#define ADC_Channel_18                              ((uint8_t)0x12)

#define ADC_Channel_TempSensor                      ((uint8_t)ADC_Channel_16)
#define ADC_Channel_Vrefint                         ((uint8_t)ADC_Channel_17)
#define ADC_Channel_Vbat                            ((uint8_t)ADC_Channel_18)

#define IS_ADC_CHANNEL(CHANNEL) (((CHANNEL) == ADC_Channel_0)  || \
                                 ((CHANNEL) == ADC_Channel_1)  || \
                                 ((CHANNEL) == ADC_Channel_2)  || \
                                 ((CHANNEL) == ADC_Channel_3)  || \
                                 ((CHANNEL) == ADC_Channel_4)  || \
                                 ((CHANNEL) == ADC_Channel_5)  || \
                                 ((CHANNEL) == ADC_Channel_6)  || \
                                 ((CHANNEL) == ADC_Channel_7)  || \
                                 ((CHANNEL) == ADC_Channel_8)  || \
                                 ((CHANNEL) == ADC_Channel_9)  || \
                                 ((CHANNEL) == ADC_Channel_10) || \
                                 ((CHANNEL) == ADC_Channel_11) || \
                                 ((CHANNEL) == ADC_Channel_12) || \
                                 ((CHANNEL) == ADC_Channel_13) || \
                                 ((CHANNEL) == ADC_Channel_14) || \
                                 ((CHANNEL) == ADC_Channel_15) || \
                                 ((CHANNEL) == ADC_Channel_16) || \
                                 ((CHANNEL) == ADC_Channel_17) || \
                                 ((CHANNEL) == ADC_Channel_18))
/**
  * @}
  */

/** @defgroup ADC_sampling_time 
  * @{
  */

#define ADC_SampleTime_1Cycles5                    ((uint8_t)0x00)
#define ADC_SampleTime_7Cycles5                    ADC_SMPR2_SMP0_0
#define ADC_SampleTime_13Cycles5                   ADC_SMPR2_SMP0_1
#define ADC_SampleTime_28Cycles5                   ((uint8_t)0x03)
#define ADC_SampleTime_41Cycles5                   ADC_SMPR2_SMP0_2
#define ADC_SampleTime_55Cycles5                   ((uint8_t)0x05)
#define ADC_SampleTime_71Cycles5                   ((uint8_t)0x06)
#define ADC_SampleTime_239Cycles5                  ((uint8_t)0x07)

#define IS_ADC_SAMPLE_TIME(TIME) (((TIME) == ADC_SampleTime_1Cycles5) || \
                                  ((TIME) == ADC_SampleTime_7Cycles5) || \
                                  ((TIME) == ADC_SampleTime_13Cycles5) || \
                                  ((TIME) == ADC_SampleTime_28Cycles5) || \
                                  ((TIME) == ADC_SampleTime_41Cycles5) || \
                                  ((TIME) == ADC_SampleTime_55Cycles5) || \
                                  ((TIME) == ADC_SampleTime_71Cycles5) || \
                                  ((TIME) == ADC_SampleTime_239Cycles5))
/**
  * @}
  */

/** @defgroup ADC_external_trigger_sources_for_injected_channels_conversion 
  * @{
  */

#define ADC_ExternalTrigInjecConv_T19_CC1           ((uint32_t)0x00000000)
#define ADC_ExternalTrigInjecConv_T19_CC2           ADC_CR2_JEXTSEL_0
#define ADC_ExternalTrigInjecConv_T2_TRGO           ADC_CR2_JEXTSEL_1
#define ADC_ExternalTrigInjecConv_T2_CC1            ((uint32_t)0x00003000)
#define ADC_ExternalTrigInjecConv_T3_CC4            ADC_CR2_JEXTSEL_2
#define ADC_ExternalTrigInjecConv_T4_TRGO           ((uint32_t)0x00005000)
#define ADC_ExternalTrigInjecConv_Ext_IT15          ((uint32_t)0x00006000)
#define ADC_ExternalTrigInjecConv_None              ((uint32_t)0x00007000)

#define IS_ADC_EXT_INJEC_TRIG(INJTRIG) (((INJTRIG) == ADC_ExternalTrigInjecConv_T19_CC1) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T19_CC2) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T2_TRGO) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T2_CC1)  || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T3_CC4)  || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_T4_TRGO) || \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_Ext_IT15)|| \
                                        ((INJTRIG) == ADC_ExternalTrigInjecConv_None))

/**
  * @}
  */

/** @defgroup ADC_injected_channel_selection 
  * @{
  */

#define ADC_InjectedChannel_1                       ((uint8_t)0x14)
#define ADC_InjectedChannel_2                       ((uint8_t)0x18)
#define ADC_InjectedChannel_3                       ((uint8_t)0x1C)
#define ADC_InjectedChannel_4                       ((uint8_t)0x20)
#define IS_ADC_INJECTED_CHANNEL(CHANNEL) (((CHANNEL) == ADC_InjectedChannel_1) || \
                                          ((CHANNEL) == ADC_InjectedChannel_2) || \
                                          ((CHANNEL) == ADC_InjectedChannel_3) || \
                                          ((CHANNEL) == ADC_InjectedChannel_4))
/**
  * @}
  */

/** @defgroup ADC_analog_watchdog_selection 
  * @{
  */

#define ADC_AnalogWatchdog_SingleRegEnable         ((uint32_t)0x00800200)
#define ADC_AnalogWatchdog_SingleInjecEnable       ((uint32_t)0x00400200)
#define ADC_AnalogWatchdog_SingleRegOrInjecEnable  ((uint32_t)0x00C00200)
#define ADC_AnalogWatchdog_AllRegEnable            ADC_CR1_AWDEN
#define ADC_AnalogWatchdog_AllInjecEnable          ADC_CR1_JAWDEN
#define ADC_AnalogWatchdog_AllRegAllInjecEnable    ((uint32_t)0x00C00000)
#define ADC_AnalogWatchdog_None                    ((uint32_t)0x00000000)

#define IS_ADC_ANALOG_WATCHDOG(WATCHDOG) (((WATCHDOG) == ADC_AnalogWatchdog_SingleRegEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_SingleInjecEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_SingleRegOrInjecEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_AllRegEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_AllInjecEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_AllRegAllInjecEnable) || \
                                          ((WATCHDOG) == ADC_AnalogWatchdog_None))
/**
  * @}
  */

/** @defgroup ADC_interrupts_definition 
  * @{
  */

#define ADC_IT_EOC                                 ((uint16_t)0x0220)
#define ADC_IT_AWD                                 ((uint16_t)0x0140)
#define ADC_IT_JEOC                                ((uint16_t)0x0480)

#define IS_ADC_IT(IT) ((((IT) & (uint16_t)0xF81F) == 0x00) && ((IT) != 0x00))

#define IS_ADC_GET_IT(IT) (((IT) == ADC_IT_EOC) || ((IT) == ADC_IT_AWD) || \
                           ((IT) == ADC_IT_JEOC))
/**
  * @}
  */

/** @defgroup ADC_flags_definition 
  * @{
  */

#define ADC_FLAG_AWD                               ADC_SR_AWD
#define ADC_FLAG_EOC                               ADC_SR_EOC
#define ADC_FLAG_JEOC                              ADC_SR_JEOC
#define ADC_FLAG_JSTRT                             ADC_SR_JSTRT
#define ADC_FLAG_STRT                              ADC_SR_STRT

#define IS_ADC_CLEAR_FLAG(FLAG) ((((FLAG) & (uint8_t)0xE0) == 0x00) && ((FLAG) != 0x00))
#define IS_ADC_GET_FLAG(FLAG) (((FLAG) == ADC_FLAG_AWD) || ((FLAG) == ADC_FLAG_EOC) || \
                               ((FLAG) == ADC_FLAG_JEOC) || ((FLAG)== ADC_FLAG_JSTRT) || \
                               ((FLAG) == ADC_FLAG_STRT))
/**
  * @}
  */

/** @defgroup ADC_thresholds 
  * @{
  */

#define IS_ADC_THRESHOLD(THRESHOLD) ((THRESHOLD) <= 0xFFF)

/**
  * @}
  */

/** @defgroup ADC_injected_offset 
  * @{
  */

#define IS_ADC_OFFSET(OFFSET) ((OFFSET) <= 0xFFF)

/**
  * @}
  */

/** @defgroup ADC_injected_length 
  * @{
  */

#define IS_ADC_INJECTED_LENGTH(LENGTH) (((LENGTH) >= 0x1) && ((LENGTH) <= 0x4))

/**
  * @}
  */

/** @defgroup ADC_injected_rank 
  * @{
  */

#define IS_ADC_INJECTED_RANK(RANK) (((RANK) >= 0x1) && ((RANK) <= 0x4))

/**
  * @}
  */ 


/** @defgroup ADC_regular_length 
  * @{
  */

#define IS_ADC_REGULAR_LENGTH(LENGTH) (((LENGTH) >= 0x1) && ((LENGTH) <= 0x10))
/**
  * @}
  */

/** @defgroup ADC_regular_rank 
  * @{
  */

#define IS_ADC_REGULAR_RANK(RANK) (((RANK) >= 0x1) && ((RANK) <= 0x10))

/**
  * @}
  */

/** @defgroup ADC_regular_discontinuous_mode_number 
  * @{
  */

#define IS_ADC_REGULAR_DISC_NUMBER(NUMBER) (((NUMBER) >= 0x1) && ((NUMBER) <= 0x8))

/**
  * @}
  */

/**
  * @}
  */


/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/*  Function used to set the ADC configuration to the default reset state *****/
void ADC_DeInit(ADC_TypeDef* ADCx);

/* Initialization and Configuration functions *********************************/
void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef* ADC_InitStruct);
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct);
void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_StartCalibration(ADC_TypeDef* ADCx);
void ADC_ResetCalibration(ADC_TypeDef* ADCx);

/* Analog Watchdog configuration functions ************************************/
void ADC_AnalogWatchdogCmd(ADC_TypeDef* ADCx, uint32_t ADC_AnalogWatchdog);
void ADC_AnalogWatchdogThresholdsConfig(ADC_TypeDef* ADCx, uint16_t HighThreshold, uint16_t LowThreshold);
void ADC_AnalogWatchdogSingleChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel);

/* Temperature Sensor & Vrefint (Voltage Reference internal) management function */
void ADC_TempSensorVrefintCmd(FunctionalState NewState);

/* Regular Channels Configuration functions ***********************************/
void ADC_RegularChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);
void ADC_ExternalTrigConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_SoftwareStartConv(ADC_TypeDef* ADCx);
FlagStatus ADC_GetSoftwareStartConvStatus(ADC_TypeDef* ADCx);
void ADC_ContinuousModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_DiscModeChannelCountConfig(ADC_TypeDef* ADCx, uint8_t Number);
void ADC_DiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx);

/* Regular Channels DMA Configuration functions *******************************/
void ADC_DMACmd(ADC_TypeDef* ADCx, FunctionalState NewState);

/* Injected channels Configuration functions **********************************/
void ADC_InjectedChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);
void ADC_InjectedSequencerLengthConfig(ADC_TypeDef* ADCx, uint8_t Length);
void ADC_SetInjectedOffset(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel, uint16_t ADC_Offset);
void ADC_ExternalTrigInjectedConvConfig(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConv);
void ADC_ExternalTrigInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_SoftwareStartInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
FlagStatus ADC_GetSoftwareStartInjectedConvCmdStatus(ADC_TypeDef* ADCx);
void ADC_AutoInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_InjectedDiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState);
uint16_t ADC_GetInjectedConversionValue(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel);

/* Interrupts and flags management functions **********************************/
void ADC_ITConfig(ADC_TypeDef* ADCx, uint16_t ADC_IT, FunctionalState NewState);
FlagStatus ADC_GetFlagStatus(ADC_TypeDef* ADCx, uint8_t ADC_FLAG);
void ADC_ClearFlag(ADC_TypeDef* ADCx, uint8_t ADC_FLAG);
ITStatus ADC_GetITStatus(ADC_TypeDef* ADCx, uint16_t ADC_IT);
void ADC_ClearITPendingBit(ADC_TypeDef* ADCx, uint16_t ADC_IT);
FlagStatus ADC_GetCalibrationStatus(ADC_TypeDef* ADCx);
FlagStatus ADC_GetResetCalibrationStatus(ADC_TypeDef* ADCx);

#ifdef __cplusplus
}
#endif

#endif /*__STM32F37X_ADC_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
