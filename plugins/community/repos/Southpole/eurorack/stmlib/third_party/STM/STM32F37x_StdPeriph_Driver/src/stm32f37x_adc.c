/**
  ******************************************************************************
  * @file    stm32f37x_adc.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the Analog to Digital Converter (ADC) peripheral:
  *           + Initialization and Configuration
  *           + Analog Watchdog configuration
  *           + Temperature Sensor, Vrefint (Internal Reference Voltage)
  *             and VBAT (Voltage battery) management
  *           + Regular Channels Configuration
  *           + Regular Channels DMA Configuration
  *           + Injected channels Configuration
  *           + Interrupts and flags management
  *         
  *  @verbatim
================================================================================
                      ##### How to use this driver #####
================================================================================
    [..]
    (#) Enable the ADC interface clock using 
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    (#) ADC pins configuration
       (++) Enable the clock for the ADC GPIOs using the following function:
            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOx, ENABLE);
       (++) Configure these ADC pins in analog mode using GPIO_Init();
    (#) Configure the data alignment using the ADC_Init() function.
    (#) Activate the ADC peripheral using ADC_Cmd() function.

    *** Regular channels group configuration ***
    ============================================
    [..] 
    (+) To configure the ADC regular channels group features, use 
        ADC_Init() and ADC_RegularChannelConfig() functions.
    (+) To activate the continuous mode, use the ADC_ContinuousModeCmd()
        function.
    (+) To configure and activate the Discontinuous mode, use the
        ADC_DiscModeChannelCountConfig() and ADC_DiscModeCmd() functions.
    (+) To read the ADC converted values, use the ADC_GetConversionValue()
        function.

    *** DMA for Regular channels group features configuration ***
    =============================================================
    [..]
    (+) To enable the DMA mode for regular channels group, use the 
        ADC_DMACmd() function.
             
    *** Injected channels group configuration ***
    =============================================
    [..]    
    (+) To configure the ADC Injected channels group features, use 
        ADC_InjectedChannelConfig() function.
    (+) To activate the Injected Discontinuous mode, use the 
        ADC_InjectedDiscModeCmd() function.  
    (+) To activate the AutoInjected mode, use the ADC_AutoInjectedConvCmd() 
        function.
    (+) To read the ADC converted values, use the ADC_GetInjectedConversionValue() function.

  *  @endverbatim
  *
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f37x_adc.h"
#include "stm32f37x_rcc.h"

/** @addtogroup STM32F37x_StdPeriph_Driver
  * @{
  */

/** @defgroup ADC 
  * @brief ADC driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* CR2 register Mask */
#define ADC_CR2_CLEAR_MASK         ((uint32_t)0xFFF1F7FD)
/* ADC SQRx mask */
#define ADC_SQR_SQ_SET             ((uint32_t)0x0000001F)
/* ADC JSQRx mask */
#define ADC_JSQR_JSQ_SET           ((uint32_t)0x0000001F)
/* ADC SMPRx mask */
#define ADC_SMPR_SMP_SET           ((uint32_t)0x00000007)
/* ADC JDRx registers offset */
#define ADC_JDR_OFFSET             ((uint8_t)0x28)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup ADC_Private_Functions
  * @{
  */

/** @defgroup ADC_Group1 Initialization and Configuration functions
 *  @brief   Initialization and Configuration functions 
 *
@verbatim
 ===============================================================================
          ##### Initialization and Configuration functions #####
 ===============================================================================  
    [..] This section provides functions allowing to:
        (+) Scan Conversion Mode (multichannels or one channel) for regular group
        (+) ADC Continuous Conversion Mode (Continuous or Single conversion) for
            regular group
        (+) External trigger Edge and source of regular group, 
        (+) Converted data alignment (left or right)
        (+) The number of ADC conversions that will be done using the sequencer
            for regular channel group
        (+) Enable or disable the ADC peripheral
        (+) Start/Reset the calibration
   
@endverbatim
  * @{
  */

/**
  * @brief  Deinitializes the ADCx peripheral registers to their default reset values.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval None
  */
void ADC_DeInit(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));

  /* Enable ADC1 reset state */
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
  /* Release ADC1 from reset state */
  RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
}

/**
  * @brief  Initializes the ADCx peripheral according to the specified parameters
  *         in the ADC_InitStruct.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_InitStruct: pointer to an ADC_InitTypeDef structure that contains
  *         the configuration information for the specified ADC peripheral.
  * @retval None
  */
void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef* ADC_InitStruct)
{
  uint32_t tmpreg1 = 0;
  uint8_t tmpreg2 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(ADC_InitStruct->ADC_ScanConvMode));
  assert_param(IS_FUNCTIONAL_STATE(ADC_InitStruct->ADC_ContinuousConvMode));
  assert_param(IS_ADC_EXT_TRIG(ADC_InitStruct->ADC_ExternalTrigConv));   
  assert_param(IS_ADC_DATA_ALIGN(ADC_InitStruct->ADC_DataAlign)); 
  assert_param(IS_ADC_REGULAR_LENGTH(ADC_InitStruct->ADC_NbrOfChannel));

  /*---------------------------- ADCx CR1 Configuration -----------------*/
  /* Get the ADCx CR1 value */
  tmpreg1 = ADCx->CR1;
  /* Clear SCAN bit */
  tmpreg1 &= (uint32_t)(~ADC_CR1_SCAN);
  /* Configure ADCx: scan conversion mode */
  /* Set SCAN bit according to ADC_ScanConvMode value */
  tmpreg1 |= (uint32_t)((uint32_t)ADC_InitStruct->ADC_ScanConvMode << 8);
  /* Write to ADCx CR1 */
  ADCx->CR1 = tmpreg1;

  /*---------------------------- ADCx CR2 Configuration -----------------*/
  /* Get the ADCx CR2 value */
  tmpreg1 = ADCx->CR2;
  /* Clear CONT, ALIGN and EXTSEL bits */
  tmpreg1 &= ADC_CR2_CLEAR_MASK;
  /* Configure ADCx: external trigger event and continuous conversion mode */
  /* Set ALIGN bit according to ADC_DataAlign value */
  /* Set EXTSEL bits according to ADC_ExternalTrigConv value */
  /* Set CONT bit according to ADC_ContinuousConvMode value */
  tmpreg1 |= (uint32_t)(ADC_InitStruct->ADC_DataAlign | ADC_InitStruct->ADC_ExternalTrigConv |
            ((uint32_t)ADC_InitStruct->ADC_ContinuousConvMode << 1));
  /* Write to ADCx CR2 */
  ADCx->CR2 = tmpreg1;

  /*---------------------------- ADCx SQR1 Configuration -----------------*/
  /* Get the ADCx SQR1 value */
  tmpreg1 = ADCx->SQR1;
  /* Clear L bits */
  tmpreg1 &= (uint32_t) (~ADC_SQR1_L);
  /* Configure ADCx: regular channel sequence length */
  /* Set L bits according to ADC_NbrOfChannel value */
  tmpreg2 |= (uint8_t) (ADC_InitStruct->ADC_NbrOfChannel - (uint8_t)1);
  tmpreg1 |= (uint32_t)tmpreg2 << 20;
  /* Write to ADCx SQR1 */
  ADCx->SQR1 = tmpreg1;
}

/**
  * @brief  Fills each ADC_InitStruct member with its default value.
  * @param  ADC_InitStruct : pointer to an ADC_InitTypeDef structure which will be initialized.
  * @retval None
  */
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct)
{
  /* Reset ADC init structure parameters values */
  /* initialize the ADC_ScanConvMode member */
  ADC_InitStruct->ADC_ScanConvMode = DISABLE;
  /* Initialize the ADC_ContinuousConvMode member */
  ADC_InitStruct->ADC_ContinuousConvMode = DISABLE;
  /* Initialize the ADC_ExternalTrigConv member */
  ADC_InitStruct->ADC_ExternalTrigConv = ADC_ExternalTrigConv_T19_TRGO;
  /* Initialize the ADC_DataAlign member */
  ADC_InitStruct->ADC_DataAlign = ADC_DataAlign_Right;
  /* Initialize the ADC_NbrOfChannel member */
  ADC_InitStruct->ADC_NbrOfChannel = 1;
}

/**
  * @brief  Enables or disables the specified ADC peripheral.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the ADCx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Set the ADON bit to wake up the ADC from power down mode */
    ADCx->CR2 |= ADC_CR2_ADON;
  }
  else
  {
    /* Disable the selected ADC peripheral */
    ADCx->CR2 &= (uint32_t)(~ADC_CR2_ADON);
  }
}

/**
  * @brief  Starts the selected ADC calibration process.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval None
  */
void ADC_StartCalibration(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Enable the selected ADC calibration process */  
  ADCx->CR2 |= ADC_CR2_CAL;
}

/**
  * @brief  Resets the selected ADC calibration registers.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval None
  */
void ADC_ResetCalibration(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Resets the selected ADC calibration registers */  
  ADCx->CR2 |= ADC_CR2_RSTCAL;
}

/**
  * @}
  */

/** @defgroup ADC_Group2 Analog Watchdog configuration functions
 *  @brief   Analog Watchdog configuration functions 
 *
@verbatim   
 ===============================================================================
          ##### Analog Watchdog configuration functions ##### 
 ===============================================================================  

    [..] This section provides functions allowing to configure the Analog Watchdog
         (AWD) feature in the ADC.
  
    [..] A typical configuration Analog Watchdog is done following these steps :
         (#) The ADC guarded channel(s) is (are) selected using the 
             ADC_AnalogWatchdogSingleChannelConfig() function.
         (#) The Analog watchdog lower and higher threshold are configured using
             the ADC_AnalogWatchdogThresholdsConfig() function.
         (#) The Analog watchdog is enabled and configured to enable the check,
             on one or more channels, using the  ADC_AnalogWatchdogCmd() function.

@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the analog watchdog on single/all regular
  *         or injected channels
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_AnalogWatchdog: the ADC analog watchdog configuration.
  *          This parameter can be one of the following values:
  *            @arg ADC_AnalogWatchdog_SingleRegEnable: Analog watchdog on a single regular channel
  *            @arg ADC_AnalogWatchdog_SingleInjecEnable: Analog watchdog on a single injected channel
  *            @arg ADC_AnalogWatchdog_SingleRegOrInjecEnable: Analog watchdog on a single regular or injected channel
  *            @arg ADC_AnalogWatchdog_AllRegEnable: Analog watchdog on  all regular channel
  *            @arg ADC_AnalogWatchdog_AllInjecEnable: Analog watchdog on  all injected channel
  *            @arg ADC_AnalogWatchdog_AllRegAllInjecEnable: Analog watchdog on all regular and injected channels
  *            @arg ADC_AnalogWatchdog_None: No channel guarded by the analog watchdog
  * @retval None	  
  */
void ADC_AnalogWatchdogCmd(ADC_TypeDef* ADCx, uint32_t ADC_AnalogWatchdog)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_ANALOG_WATCHDOG(ADC_AnalogWatchdog));
  /* Get the old register value */
  tmpreg = ADCx->CR1;
  /* Clear AWDEN, AWDENJ and AWDSGL bits */
  tmpreg &= (uint32_t) (~(ADC_CR1_JAWDEN | ADC_CR1_AWDEN | ADC_CR1_AWDSGL));
  /* Set the analog watchdog enable mode */
  tmpreg |= ADC_AnalogWatchdog;
  /* Store the new register value */
  ADCx->CR1 = tmpreg;
}

/**
  * @brief  Configures the high and low thresholds of the analog watchdog.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  HighThreshold: the ADC analog watchdog High threshold value.
  *          This parameter must be a 12bit value.
  * @param  LowThreshold: the ADC analog watchdog Low threshold value.
  *          This parameter must be a 12bit value.
  * @retval None
  */
void ADC_AnalogWatchdogThresholdsConfig(ADC_TypeDef* ADCx, uint16_t HighThreshold,
                                        uint16_t LowThreshold)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_THRESHOLD(HighThreshold));
  assert_param(IS_ADC_THRESHOLD(LowThreshold));
  /* Set the ADCx high threshold */
  ADCx->HTR = HighThreshold;
  /* Set the ADCx low threshold */
  ADCx->LTR = LowThreshold;
}

/**
  * @brief  Configures the analog watchdog guarded single channel
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure for the analog watchdog. 
  *          This parameter can be one of the following values:
  *            @arg ADC_Channel_0: ADC Channel0 selected
  *            @arg ADC_Channel_1: ADC Channel1 selected
  *            @arg ADC_Channel_2: ADC Channel2 selected
  *            @arg ADC_Channel_3: ADC Channel3 selected
  *            @arg ADC_Channel_4: ADC Channel4 selected
  *            @arg ADC_Channel_5: ADC Channel5 selected
  *            @arg ADC_Channel_6: ADC Channel6 selected
  *            @arg ADC_Channel_7: ADC Channel7 selected
  *            @arg ADC_Channel_8: ADC Channel8 selected
  *            @arg ADC_Channel_9: ADC Channel9 selected
  *            @arg ADC_Channel_10: ADC Channel10 selected
  *            @arg ADC_Channel_11: ADC Channel11 selected
  *            @arg ADC_Channel_12: ADC Channel12 selected
  *            @arg ADC_Channel_13: ADC Channel13 selected
  *            @arg ADC_Channel_14: ADC Channel14 selected
  *            @arg ADC_Channel_15: ADC Channel15 selected
  *            @arg ADC_Channel_16: ADC Channel16 selected
  *            @arg ADC_Channel_17: ADC Channel17 selected
  * @retval None
  */
void ADC_AnalogWatchdogSingleChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CHANNEL(ADC_Channel));
  /* Get the old register value */
  tmpreg = ADCx->CR1;
  /* Clear the Analog watchdog channel select bits */
  tmpreg &= (uint32_t)(~ADC_CR1_AWDCH);
  /* Set the Analog watchdog channel */
  tmpreg |= ADC_Channel;
  /* Store the new register value */
  ADCx->CR1 = tmpreg;
}

/**
  * @}
  */

/** @defgroup ADC_Group3 Temperature Sensor- Vrefint (Internal Reference Voltage) and VBAT management function
 *  @brief   Temperature Sensor- Vrefint (Internal Reference Voltage) and VBAT management function
 *
@verbatim   
 ===============================================================================
 ##### Temperature Sensor, Vrefint and VBAT management function #####
 ===============================================================================  

    [..]  This section provides a function allowing to enable/ disable the internal 
          connections between the ADC and the Temperature Sensor, the Vrefint
          and the VBAT sources.

    [..] A typical configuration to get the Temperature sensor and Vrefint channels 
         voltages is done following these steps :
         (#) Enable the internal connection of Temperature sensor and Vrefint sources 
             with the ADC channels using ADC_TempSensorVrefintCmd() function.
             Enable the internal connection of VBAT using SYSCFG_VBATMonitoringCmd(ENABLE);
         (#) Select the ADC_Channel_TempSensor and/or ADC_Channel_Vrefint and/or 
            ADC_Channel_Vbat using ADC_RegularChannelConfig()
            or ADC_InjectedChannelConfig() functions 
         (#) Get the voltage values, using ADC_GetConversionValue() or  
             ADC_GetInjectedConversionValue().
 
@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the temperature sensor and Vrefint channel.
  * @param  NewState: new state of the temperature sensor.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_TempSensorVrefintCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the temperature sensor and Vrefint channel*/
    ADC1->CR2 |= ADC_CR2_TSVREFE;
  }
  else
  {
    /* Disable the temperature sensor and Vrefint channel*/
    ADC1->CR2 &= (uint32_t) (~ADC_CR2_TSVREFE);
  }
}

/**
  * @}
  */

/** @defgroup ADC_Group4 Regular Channels Configuration functions
 *  @brief   Regular Channels Configuration functions 
 *
@verbatim   
 ===============================================================================
            ##### Regular Channels Configuration functions  #####
 ===============================================================================  

    [..] This section provides functions allowing to manage the ADC regular channels,
         it is composed of 2 sub sections : 
  
         (#) Configuration and management functions for regular channels: This subsection 
             provides functions allowing to configure the ADC regular channels :    
             (++) Configure the rank in the regular group sequencer for each channel
             (++) Configure the sampling time for each channel
             (++) select the conversion Trigger for regular channels
             (++) select the desired EOC event behavior configuration
             (++) Activate the continuous Mode  (*)
             (++) Activate the Discontinuous Mode 
             -@@- Please Note that the following features for regular channels
                  are configured using the ADC_Init() function :
                  (+@@) scan mode activation 
                  (+@@) continuous mode activation (**) 
                  (+@@) External trigger source  
                  (+@@) External trigger edge 
                  (+@@) number of conversion in the regular channels group sequencer.
     
             -@@- (*) and (**) are performing the same configuration
     
         (#) Get the conversion data: This subsection provides an important function in 
             the ADC peripheral since it returns the converted data of the current 
             regular channel. When the Conversion value is read, the EOC Flag is 
             automatically cleared.
  
@endverbatim
  * @{
  */

/**
  * @brief  Configures for the selected ADC regular channel its corresponding
  *         rank in the sequencer and its sample time.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure. 
  *          This parameter can be one of the following values:
  *            @arg ADC_Channel_0: ADC Channel0 selected
  *            @arg ADC_Channel_1: ADC Channel1 selected
  *            @arg ADC_Channel_2: ADC Channel2 selected
  *            @arg ADC_Channel_3: ADC Channel3 selected
  *            @arg ADC_Channel_4: ADC Channel4 selected
  *            @arg ADC_Channel_5: ADC Channel5 selected
  *            @arg ADC_Channel_6: ADC Channel6 selected
  *            @arg ADC_Channel_7: ADC Channel7 selected
  *            @arg ADC_Channel_8: ADC Channel8 selected
  *            @arg ADC_Channel_9: ADC Channel9 selected
  *            @arg ADC_Channel_10: ADC Channel10 selected
  *            @arg ADC_Channel_11: ADC Channel11 selected
  *            @arg ADC_Channel_12: ADC Channel12 selected
  *            @arg ADC_Channel_13: ADC Channel13 selected
  *            @arg ADC_Channel_14: ADC Channel14 selected
  *            @arg ADC_Channel_15: ADC Channel15 selected
  *            @arg ADC_Channel_16: ADC Channel16 selected
  *            @arg ADC_Channel_17: ADC Channel17 selected
  * @param  Rank: The rank in the regular group sequencer. This parameter must be between 1 to 16.
  * @param  ADC_SampleTime: The sample time value to be set for the selected channel. 
  *          This parameter can be one of the following values:
  *            @arg ADC_SampleTime_1Cycles5: Sample time equal to 1.5 cycles
  *            @arg ADC_SampleTime_7Cycles5: Sample time equal to 7.5 cycles
  *            @arg ADC_SampleTime_13Cycles5: Sample time equal to 13.5 cycles
  *            @arg ADC_SampleTime_28Cycles5: Sample time equal to 28.5 cycles	
  *            @arg ADC_SampleTime_41Cycles5: Sample time equal to 41.5 cycles	
  *            @arg ADC_SampleTime_55Cycles5: Sample time equal to 55.5 cycles	
  *            @arg ADC_SampleTime_71Cycles5: Sample time equal to 71.5 cycles	
  *            @arg ADC_SampleTime_239Cycles5: Sample time equal to 239.5 cycles	
  * @retval None
  */
void ADC_RegularChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime)
{
  uint32_t tmpreg1 = 0, tmpreg2 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CHANNEL(ADC_Channel));
  assert_param(IS_ADC_REGULAR_RANK(Rank));
  assert_param(IS_ADC_SAMPLE_TIME(ADC_SampleTime));
  /* if ADC_Channel_10 ... ADC_Channel_17 is selected */
  if (ADC_Channel > ADC_Channel_9)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR1;
    /* Calculate the mask to clear */
    tmpreg2 = ADC_SMPR_SMP_SET << (3 * (ADC_Channel - 10));
    /* Clear the old channel sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * (ADC_Channel - 10));
    /* Set the new channel sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR1 = tmpreg1;
  }
  else /* ADC_Channel include in ADC_Channel_[0..9] */
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR2;
    /* Calculate the mask to clear */
    tmpreg2 = ADC_SMPR_SMP_SET << (3 * ADC_Channel);
    /* Clear the old channel sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * ADC_Channel);
    /* Set the new channel sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR2 = tmpreg1;
  }
  /* For Rank 1 to 6 */
  if (Rank < 7)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR3;
    /* Calculate the mask to clear */
    tmpreg2 = ADC_SQR_SQ_SET << (5 * (Rank - 1));
    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 1));
    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SQR3 = tmpreg1;
  }
  /* For Rank 7 to 12 */
  else if (Rank < 13)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR2;
    /* Calculate the mask to clear */
    tmpreg2 = ADC_SQR_SQ_SET << (5 * (Rank - 7));
    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 7));
    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SQR2 = tmpreg1;
  }
  /* For Rank 13 to 16 */
  else
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SQR1;
    /* Calculate the mask to clear */
    tmpreg2 = ADC_SQR_SQ_SET << (5 * (Rank - 13));
    /* Clear the old SQx bits for the selected rank */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_Channel << (5 * (Rank - 13));
    /* Set the SQx bits for the selected rank */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SQR1 = tmpreg1;
  }
}

/**
  * @brief  Enables or disables the ADCx conversion through external trigger.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC external trigger start of conversion.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_ExternalTrigConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC conversion on external event */
    ADCx->CR2 |= ADC_CR2_EXTTRIG;
  }
  else
  {
    /* Disable the selected ADC conversion on external event */
    ADCx->CR2 &= (uint32_t) (~ADC_CR2_EXTTRIG);
  }
}

/**
  * @brief  Enables or disables the selected ADC software start conversion .
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval None
  */
void ADC_SoftwareStartConv(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));

  /* Enable the selected ADC conversion on external event and start the selected
     ADC conversion */
    ADCx->CR2 |= (uint32_t)(ADC_CR2_SWSTART | ADC_CR2_EXTTRIG);

}

/**
  * @brief  Gets the selected ADC Software start conversion Status.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval The new state of ADC software start conversion (SET or RESET).
  */
FlagStatus ADC_GetSoftwareStartConvStatus(ADC_TypeDef* ADCx)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Check the status of SWSTART bit */
  if ((ADCx->CR2 & ADC_CR2_SWSTART) != (uint32_t)RESET)
  {
    /* SWSTART bit is set */
    bitstatus = SET;
  }
  else
  {
    /* SWSTART bit is reset */
    bitstatus = RESET;
  }
  /* Return the SWSTART bit status */
  return  bitstatus;
}

/**
  * @brief  Enables or disables the ADC continuous conversion mode 
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC continuous conversion mode
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_ContinuousModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC continuous conversion mode */
    ADCx->CR2 |= (uint32_t)ADC_CR2_CONT;
  }
  else
  {
    /* Disable the selected ADC continuous conversion mode */
    ADCx->CR2 &= (uint32_t)(~ADC_CR2_CONT);
  }
}

/**
  * @brief  Configures the discontinuous mode for the selected ADC regular
  *         group channel.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  Number: specifies the discontinuous mode regular channel
  *         count value. This number must be between 1 and 8.
  * @retval None
  */
void ADC_DiscModeChannelCountConfig(ADC_TypeDef* ADCx, uint8_t Number)
{
  uint32_t tmpreg1 = 0;
  uint32_t tmpreg2 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_REGULAR_DISC_NUMBER(Number));
  /* Get the old register value */
  tmpreg1 = ADCx->CR1;
  /* Clear the old discontinuous mode channel count */
  tmpreg1 &= (uint32_t)(~ADC_CR1_DISCNUM);
  /* Set the discontinuous mode channel count */
  tmpreg2 = Number - 1;
  tmpreg1 |= tmpreg2 << 13;
  /* Store the new register value */
  ADCx->CR1 = tmpreg1;
}

/**
  * @brief  Enables or disables the discontinuous mode on regular group
  *         channel for the specified ADC
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC discontinuous mode
  *         on regular group channel.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_DiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC regular discontinuous mode */
    ADCx->CR1 |= ADC_CR1_DISCEN;
  }
  else
  {
    /* Disable the selected ADC regular discontinuous mode */
    ADCx->CR1 &= (uint32_t)(~ADC_CR1_DISCEN);
  }
}

/**
  * @brief  Returns the last ADCx conversion result data for regular channel.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval The Data conversion value.
  */
uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Return the selected ADC conversion value */
  return (uint16_t) ADCx->DR;
}

/**
  * @}
  */

/** @defgroup ADC_Group5 Regular Channels DMA Configuration functions
 *  @brief   Regular Channels DMA Configuration functions 
 *
@verbatim   
 ===============================================================================
          ##### Regular Channels DMA Configuration functions #####
 =============================================================================== 

    [..] This section provides functions allowing to configure the DMA for
         ADC regular channels. Since converted regular channel values are stored
         into a unique data register, it is useful to use DMA for conversion of
         more than one regular channel. This avoids the loss of the data already
         stored in the ADC Data register. 
         When the DMA mode is enabled (using the ADC_DMACmd() function), after
         each conversion of a regular channel, a DMA request is generated.

@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the specified ADC DMA request.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC DMA transfer.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_DMACmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_DMA_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC DMA request */
    ADCx->CR2 |= ADC_CR2_DMA;
  }
  else
  {
    /* Disable the selected ADC DMA request */
    ADCx->CR2 &= (uint32_t) (~ADC_CR2_DMA);
  }
}

/**
  * @}
  */

/** @defgroup ADC_Group6 Injected channels Configuration functions
 *  @brief   Injected channels Configuration functions 
 *
@verbatim   
 ===============================================================================
            ##### Injected channels Configuration functions #####
 ===============================================================================  

    [..] This section provide functions allowing to configure the ADC Injected
         channels, it is composed of 2 sub sections : 
         (#) Configuration functions for Injected channels: This subsection provides 
             functions allowing to configure the ADC injected channels :    
             (++) Configure the rank in the injected group sequencer for each channel
             (++) Configure the sampling time for each channel    
             (++) Activate the Auto injected Mode  
             (++) Activate the Discontinuous Mode 
             (++) Scan mode activation  
             (++) External/software trigger source   
             (++) External trigger edge 
             (++) Injected channels sequencer.
    
         (#) Get the Specified Injected channel conversion data: This subsection 
             provides an important function in the ADC peripheral since it returns
             the converted data of the specific injected channel.

@endverbatim
  * @{
  */ 

/**
  * @brief  Configures for the selected ADC injected channel its corresponding
  *         rank in the sequencer and its sample time.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_Channel: the ADC channel to configure. 
  *          This parameter can be one of the following values:
  *            @arg ADC_Channel_0: ADC Channel0 selected
  *            @arg ADC_Channel_1: ADC Channel1 selected
  *            @arg ADC_Channel_2: ADC Channel2 selected
  *            @arg ADC_Channel_3: ADC Channel3 selected
  *            @arg ADC_Channel_4: ADC Channel4 selected
  *            @arg ADC_Channel_5: ADC Channel5 selected
  *            @arg ADC_Channel_6: ADC Channel6 selected
  *            @arg ADC_Channel_7: ADC Channel7 selected
  *            @arg ADC_Channel_8: ADC Channel8 selected
  *            @arg ADC_Channel_9: ADC Channel9 selected
  *            @arg ADC_Channel_10: ADC Channel10 selected
  *            @arg ADC_Channel_11: ADC Channel11 selected
  *            @arg ADC_Channel_12: ADC Channel12 selected
  *            @arg ADC_Channel_13: ADC Channel13 selected
  *            @arg ADC_Channel_14: ADC Channel14 selected
  *            @arg ADC_Channel_15: ADC Channel15 selected
  *            @arg ADC_Channel_16: ADC Channel16 selected
  *            @arg ADC_Channel_17: ADC Channel17 selected
  * @param  Rank: The rank in the injected group sequencer. This parameter must be between 1 and 4.
  * @param  ADC_SampleTime: The sample time value to be set for the selected channel. 
  *          This parameter can be one of the following values:
  *            @arg ADC_SampleTime_1Cycles5: Sample time equal to 1.5 cycles
  *            @arg ADC_SampleTime_7Cycles5: Sample time equal to 7.5 cycles
  *            @arg ADC_SampleTime_13Cycles5: Sample time equal to 13.5 cycles
  *            @arg ADC_SampleTime_28Cycles5: Sample time equal to 28.5 cycles	
  *            @arg ADC_SampleTime_41Cycles5: Sample time equal to 41.5 cycles	
  *            @arg ADC_SampleTime_55Cycles5: Sample time equal to 55.5 cycles	
  *            @arg ADC_SampleTime_71Cycles5: Sample time equal to 71.5 cycles	
  *            @arg ADC_SampleTime_239Cycles5: Sample time equal to 239.5 cycles	
  * @retval None
  */
void ADC_InjectedChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime)
{
  uint32_t tmpreg1 = 0, tmpreg2 = 0, tmpreg3 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CHANNEL(ADC_Channel));
  assert_param(IS_ADC_INJECTED_RANK(Rank));
  assert_param(IS_ADC_SAMPLE_TIME(ADC_SampleTime));
  /* if ADC_Channel_10 ... ADC_Channel_17 is selected */
  if (ADC_Channel > ADC_Channel_9)
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR1;
    /* Calculate the mask to clear */
    tmpreg2 = ADC_SMPR_SMP_SET << (3*(ADC_Channel - 10));
    /* Clear the old channel sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3*(ADC_Channel - 10));
    /* Set the new channel sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR1 = tmpreg1;
  }
  else /* ADC_Channel include in ADC_Channel_[0..9] */
  {
    /* Get the old register value */
    tmpreg1 = ADCx->SMPR2;
    /* Calculate the mask to clear */
    tmpreg2 = ADC_SMPR_SMP_SET << (3 * ADC_Channel);
    /* Clear the old channel sample time */
    tmpreg1 &= ~tmpreg2;
    /* Calculate the mask to set */
    tmpreg2 = (uint32_t)ADC_SampleTime << (3 * ADC_Channel);
    /* Set the new channel sample time */
    tmpreg1 |= tmpreg2;
    /* Store the new register value */
    ADCx->SMPR2 = tmpreg1;
  }
  /* Rank configuration */
  /* Get the old register value */
  tmpreg1 = ADCx->JSQR;
  /* Get JL value: Number = JL+1 */
  tmpreg3 =  (tmpreg1 & ADC_JSQR_JL)>> 20;
  /* Calculate the mask to clear: ((Rank-1)+(4-JL-1)) */
  tmpreg2 = ADC_JSQR_JSQ_SET << (5 * (uint8_t)((Rank + 3) - (tmpreg3 + 1)));
  /* Clear the old JSQx bits for the selected rank */
  tmpreg1 &= ~tmpreg2;
  /* Calculate the mask to set: ((Rank-1)+(4-JL-1)) */
  tmpreg2 = (uint32_t)ADC_Channel << (5 * (uint8_t)((Rank + 3) - (tmpreg3 + 1)));
  /* Set the JSQx bits for the selected rank */
  tmpreg1 |= tmpreg2;
  /* Store the new register value */
  ADCx->JSQR = tmpreg1;
}

/**
  * @brief  Configures the sequencer length for injected channels
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  Length: The sequencer length. 
  *          This parameter must be a number between 1 to 4.
  * @retval None
  */
void ADC_InjectedSequencerLengthConfig(ADC_TypeDef* ADCx, uint8_t Length)
{
  uint32_t tmpreg1 = 0;
  uint32_t tmpreg2 = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_INJECTED_LENGTH(Length));
  
  /* Get the old register value */
  tmpreg1 = ADCx->JSQR;
  /* Clear the old injected sequence length JL bits */
  tmpreg1 &= (uint32_t)(~ADC_JSQR_JL);
  /* Set the injected sequence length JL bits */
  tmpreg2 = Length - 1; 
  tmpreg1 |= tmpreg2 << 20;
  /* Store the new register value */
  ADCx->JSQR = tmpreg1;
}

/**
  * @brief  Set the injected channels conversion value offset
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_InjectedChannel: the ADC injected channel to set its offset. 
  *          This parameter can be one of the following values:
  *            @arg ADC_InjectedChannel_1: Injected Channel1 selected
  *            @arg ADC_InjectedChannel_2: Injected Channel2 selected
  *            @arg ADC_InjectedChannel_3: Injected Channel3 selected
  *            @arg ADC_InjectedChannel_4: Injected Channel4 selected
  * @param  ADC_Offset: the offset value for the selected ADC injected channel
  *          This parameter must be a 12bit value.
  * @retval None
  */
void ADC_SetInjectedOffset(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel, uint16_t ADC_Offset)
{
  __IO uint32_t tmp = 0;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_INJECTED_CHANNEL(ADC_InjectedChannel));
  assert_param(IS_ADC_OFFSET(ADC_Offset));  
  
  tmp = (uint32_t)ADCx;
  tmp += ADC_InjectedChannel;
  
  /* Set the selected injected channel data offset */
  *(__IO uint32_t *) tmp = (uint32_t)ADC_Offset;
}

/**
  * @brief  Configures the ADCx external trigger for injected channels conversion.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_ExternalTrigInjecConv: specifies the ADC trigger to start injected conversion. 
  *          This parameter can be one of the following values:
  *            @arg ADC_ExternalTrigInjecConv_T1_TRGO: Timer1 TRGO event selected (for ADC1, ADC2 and ADC3)
  *            @arg ADC_ExternalTrigInjecConv_T1_CC4: Timer1 capture compare4 selected (for ADC1, ADC2 and ADC3)
  *            @arg ADC_ExternalTrigInjecConv_T2_TRGO: Timer2 TRGO event selected (for ADC1 and ADC2)
  *            @arg ADC_ExternalTrigInjecConv_T2_CC1: Timer2 capture compare1 selected (for ADC1 and ADC2)
  *            @arg ADC_ExternalTrigInjecConv_T3_CC4: Timer3 capture compare4 selected (for ADC1 and ADC2)
  *            @arg ADC_ExternalTrigInjecConv_T4_TRGO: Timer4 TRGO event selected (for ADC1 and ADC2)
  *            @arg ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4: External interrupt line 15 or Timer8
  *                                                       capture compare4 event selected (for ADC1 and ADC2)                       
  *            @arg ADC_ExternalTrigInjecConv_T4_CC3: Timer4 capture compare3 selected (for ADC3 only)
  *            @arg ADC_ExternalTrigInjecConv_T8_CC2: Timer8 capture compare2 selected (for ADC3 only)                         
  *            @arg ADC_ExternalTrigInjecConv_T8_CC4: Timer8 capture compare4 selected (for ADC3 only)
  *            @arg ADC_ExternalTrigInjecConv_T5_TRGO: Timer5 TRGO event selected (for ADC3 only)                         
  *            @arg ADC_ExternalTrigInjecConv_T5_CC4: Timer5 capture compare4 selected (for ADC3 only)                        
  *            @arg ADC_ExternalTrigInjecConv_None: Injected conversion started by software and not
  *                                          by external trigger (for ADC1, ADC2 and ADC3)
  * @retval None
  */
void ADC_ExternalTrigInjectedConvConfig(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConv)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_EXT_INJEC_TRIG(ADC_ExternalTrigInjecConv));
  /* Get the old register value */
  tmpreg = ADCx->CR2;
  /* Clear the old external event selection for injected group */
  tmpreg &= (uint32_t) (~ADC_CR2_JEXTSEL);
  /* Set the external event selection for injected group */
  tmpreg |= ADC_ExternalTrigInjecConv;
  /* Store the new register value */
  ADCx->CR2 = tmpreg;
}

/**
  * @brief  Enables or disables the ADCx injected channels conversion through
  *         external trigger
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC external trigger start of
  *         injected conversion.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_ExternalTrigInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC external event selection for injected group */
    ADCx->CR2 |= ADC_CR2_JEXTTRIG;
  }
  else
  {
    /* Disable the selected ADC external event selection for injected group */
    ADCx->CR2 &= (uint32_t)(~ADC_CR2_JEXTTRIG);
  }
}

/**
  * @brief  Enables or disables the selected ADC start of the injected 
  *         channels conversion.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC software start injected conversion.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_SoftwareStartInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC conversion for injected group on external event and start the selected
       ADC injected conversion */
    ADCx->CR2 |= (ADC_CR2_JSWSTART | ADC_CR2_JEXTTRIG);
  }
  else
  {
    /* Disable the selected ADC conversion on external event for injected group and stop the selected
       ADC injected conversion */
    ADCx->CR2 &= (uint32_t) ~(ADC_CR2_JSWSTART | ADC_CR2_JEXTTRIG);
  }
}

/**
  * @brief  Gets the selected ADC Software start injected conversion Status.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval The new state of ADC software start injected conversion (SET or RESET).
  */
FlagStatus ADC_GetSoftwareStartInjectedConvCmdStatus(ADC_TypeDef* ADCx)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Check the status of JSWSTART bit */
  if ((ADCx->CR2 & ADC_CR2_JSWSTART) != (uint32_t)RESET)
  {
    /* JSWSTART bit is set */
    bitstatus = SET;
  }
  else
  {
    /* JSWSTART bit is reset */
    bitstatus = RESET;
  }
  /* Return the JSWSTART bit status */
  return  bitstatus;
}

/**
  * @brief  Enables or disables the selected ADC automatic injected group
  *         conversion after regular one.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC auto injected conversion
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_AutoInjectedConvCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC automatic injected group conversion */
    ADCx->CR1 |= ADC_CR1_JAUTO;
  }
  else
  {
    /* Disable the selected ADC automatic injected group conversion */
    ADCx->CR1 &= (uint32_t)(~ADC_CR1_JAUTO);
  }
}

/**
  * @brief  Enables or disables the discontinuous mode for injected group
  *         channel for the specified ADC
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  NewState: new state of the selected ADC discontinuous mode
  *         on injected group channel.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_InjectedDiscModeCmd(ADC_TypeDef* ADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC injected discontinuous mode */
    ADCx->CR1 |= ADC_CR1_JDISCEN;
  }
  else
  {
    /* Disable the selected ADC injected discontinuous mode */
    ADCx->CR1 &= (uint32_t) (~ADC_CR1_JDISCEN);
  }
}

/**
  * @brief  Returns the ADC injected channel conversion result
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_InjectedChannel: the converted ADC injected channel.
  *          This parameter can be one of the following values:
  *            @arg ADC_InjectedChannel_1: Injected Channel1 selected
  *            @arg ADC_InjectedChannel_2: Injected Channel2 selected
  *            @arg ADC_InjectedChannel_3: Injected Channel3 selected
  *            @arg ADC_InjectedChannel_4: Injected Channel4 selected
  * @retval The Data conversion value.
  */
uint16_t ADC_GetInjectedConversionValue(ADC_TypeDef* ADCx, uint8_t ADC_InjectedChannel)
{
  __IO uint32_t tmp = 0;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_INJECTED_CHANNEL(ADC_InjectedChannel));

  tmp = (uint32_t)ADCx;
  tmp += ADC_InjectedChannel + ADC_JDR_OFFSET;
  
  /* Returns the selected injected channel conversion data value */
  return (uint16_t) (*(__IO uint32_t*)  tmp);   
}

/**
  * @}
  */

/** @defgroup ADC_Group7 Interrupts and flags management functions
 *  @brief   Interrupts and flags management functions
 *
@verbatim   
 ===============================================================================
            ##### Interrupts and flags management functions #####
 =============================================================================== 

    [..] This section provides functions allowing to configure the ADC Interrupts,
         get the status and clear flags and Interrupts pending bits.
  
    [..] The ADC provide 4 Interrupts sources and 9 Flags which can be divided
         into 3 groups:
  
  *** Flags and Interrupts for ADC regular channels ***
  =====================================================
    [..]
        (+)Flags :
           (##) ADC_FLAG_EOC : Regular channel end of conversion to indicate 
                the end of sequence of regular GROUP conversions
           (##) ADC_FLAG_STRT: Regular channel start to indicate when regular
                CHANNEL conversion starts.

        (+)Interrupts :
           (##) ADC_IT_EOC : specifies the interrupt source for Regular channel
                end of conversion event.
  
  
  *** Flags and Interrupts for ADC Injected channels ***
  ======================================================
    [..]
        (+)Flags :
           (##) ADC_FLAG_JEOC : Injected channel end of conversion to indicate
                at the end of injected GROUP conversion
           (##) ADC_FLAG_JSTRT : Injected channel start to indicate when injected
                GROUP conversion starts.

        (+)Interrupts :
           (##) ADC_IT_JEOC : specifies the interrupt source for Injected channel
                end of conversion event.

  *** General Flags and Interrupts for the ADC ***
  ================================================
    [..]
        (+)Flags :
           (##) ADC_FLAG_AWD : Analog watchdog + to indicate if the converted voltage
                              crosses the programmed thresholds values.
        (+)Interrupts :
           (##) ADC_IT_AWD : specifies the interrupt source for Analog watchdog event.  

    [..] The user should identify which mode will be used in his application to
         manage the ADC controller events: Polling mode or Interrupt mode.
  
    [..] In the Polling Mode it is advised to use the following functions:
         (+) ADC_GetFlagStatus() : to check if flags events occur. 
         (+) ADC_ClearFlag()     : to clear the flags events.
      
    [..] In the Interrupt Mode it is advised to use the following functions:
         (+) ADC_ITConfig()       : to enable or disable the interrupt source.
         (+) ADC_GetITStatus()    : to check if Interrupt occurs.
         (+) ADC_ClearITPendingBit() : to clear the Interrupt pending Bit 
                                       (corresponding Flag). 
@endverbatim
  * @{
  */ 

/**
  * @brief  Enables or disables the specified ADC interrupts.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt sources to be enabled or disabled. 
  *          This parameter can be any combination of the following values:
  *            @arg ADC_IT_EOC: End of conversion interrupt mask
  *            @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *            @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  * @param  NewState: new state of the specified ADC interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ADC_ITConfig(ADC_TypeDef* ADCx, uint16_t ADC_IT, FunctionalState NewState)
{
  uint8_t itmask = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  assert_param(IS_ADC_IT(ADC_IT));
  /* Get the ADC IT index */
  itmask = (uint8_t)ADC_IT;
  if (NewState != DISABLE)
  {
    /* Enable the selected ADC interrupts */
    ADCx->CR1 |= itmask;
  }
  else
  {
    /* Disable the selected ADC interrupts */
    ADCx->CR1 &= (~(uint32_t)itmask);
  }
}

/**
  * @brief  Checks whether the specified ADC flag is set or not.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_FLAG: specifies the flag to check. 
  *          This parameter can be one of the following values:
  *            @arg ADC_FLAG_AWD: Analog watchdog flag
  *            @arg ADC_FLAG_EOC: End of conversion flag
  *            @arg ADC_FLAG_JEOC: End of injected group conversion flag
  *            @arg ADC_FLAG_JSTRT: Start of injected group conversion flag
  *            @arg ADC_FLAG_STRT: Start of regular group conversion flag
  * @retval The new state of ADC_FLAG (SET or RESET).
  */
FlagStatus ADC_GetFlagStatus(ADC_TypeDef* ADCx, uint8_t ADC_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_GET_FLAG(ADC_FLAG));
  /* Check the status of the specified ADC flag */
  if ((ADCx->SR & ADC_FLAG) != (uint8_t)RESET)
  {
    /* ADC_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* ADC_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the ADC_FLAG status */
  return  bitstatus;
}

/**
  * @brief  Clears the ADCx's pending flags.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_FLAG: specifies the flag to clear. 
  *          This parameter can be any combination of the following values:
  *            @arg ADC_FLAG_AWD: Analog watchdog flag
  *            @arg ADC_FLAG_EOC: End of conversion flag
  *            @arg ADC_FLAG_JEOC: End of injected group conversion flag
  *            @arg ADC_FLAG_JSTRT: Start of injected group conversion flag
  *            @arg ADC_FLAG_STRT: Start of regular group conversion flag
  * @retval None
  */
void ADC_ClearFlag(ADC_TypeDef* ADCx, uint8_t ADC_FLAG)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_CLEAR_FLAG(ADC_FLAG));
  /* Clear the selected ADC flags */
  ADCx->SR = ~(uint32_t)ADC_FLAG;
}

/**
  * @brief  Checks whether the specified ADC interrupt has occurred or not.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt source to check. 
  *          This parameter can be one of the following values:
  *            @arg ADC_IT_EOC: End of conversion interrupt mask
  *            @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *            @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  * @retval The new state of ADC_IT (SET or RESET).
  */
ITStatus ADC_GetITStatus(ADC_TypeDef* ADCx, uint16_t ADC_IT)
{
  ITStatus bitstatus = RESET;
  uint32_t itmask = 0, enablestatus = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_GET_IT(ADC_IT));
  /* Get the ADC IT index */
  itmask = ADC_IT >> 8;
  /* Get the ADC_IT enable bit status */
  enablestatus = (ADCx->CR1 & (uint8_t)ADC_IT) ;
  /* Check the status of the specified ADC interrupt */
  if (((ADCx->SR & itmask) != (uint32_t)RESET) && enablestatus)
  {
    /* ADC_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* ADC_IT is reset */
    bitstatus = RESET;
  }
  /* Return the ADC_IT status */
  return  bitstatus;
}

/**
  * @brief  Clears the ADCx's interrupt pending bits.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupt pending bit to clear.
  *          This parameter can be any combination of the following values:
  *            @arg ADC_IT_EOC: End of conversion interrupt mask
  *            @arg ADC_IT_AWD: Analog watchdog interrupt mask
  *            @arg ADC_IT_JEOC: End of injected conversion interrupt mask
  * @retval None
  */
void ADC_ClearITPendingBit(ADC_TypeDef* ADCx, uint16_t ADC_IT)
{
  uint8_t itmask = 0;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  assert_param(IS_ADC_IT(ADC_IT));
  /* Get the ADC IT index */
  itmask = (uint8_t)(ADC_IT >> 8);
  /* Clear the selected ADC interrupt pending bits */
  ADCx->SR = ~(uint32_t)itmask;
}

/**
  * @brief  Gets the selected ADC calibration status.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval The new state of ADC calibration (SET or RESET).
  */
FlagStatus ADC_GetCalibrationStatus(ADC_TypeDef* ADCx)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Check the status of CAL bit */
  if ((ADCx->CR2 & ADC_CR2_CAL) != (uint32_t)RESET)
  {
    /* CAL bit is set: calibration on going */
    bitstatus = SET;
  }
  else
  {
    /* CAL bit is reset: end of calibration */
    bitstatus = RESET;
  }
  /* Return the CAL bit status */
  return  bitstatus;
}

/**
  * @brief  Gets the selected ADC reset calibration registers status.
  * @param  ADCx: where x can be 1 to select the ADC peripheral.
  * @retval The new state of ADC reset calibration registers (SET or RESET).
  */
FlagStatus ADC_GetResetCalibrationStatus(ADC_TypeDef* ADCx)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_ADC_ALL_PERIPH(ADCx));
  /* Check the status of RSTCAL bit */
  if ((ADCx->CR2 & ADC_CR2_RSTCAL) != (uint32_t)RESET)
  {
    /* RSTCAL bit is set */
    bitstatus = SET;
  }
  else
  {
    /* RSTCAL bit is reset */
    bitstatus = RESET;
  }
  /* Return the RSTCAL bit status */
  return  bitstatus;
}

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
