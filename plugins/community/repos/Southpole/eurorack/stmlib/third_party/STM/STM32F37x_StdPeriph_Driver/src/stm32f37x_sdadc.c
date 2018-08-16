/**
  ******************************************************************************
  * @file    stm32f37x_sdadc.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the Sigma-Delta Analog to Digital Convertor
  *          (SDADC) peripherals:
  *           + Initialization and Configuration
  *           + Regular Channels Configuration
  *           + Injected channels Configuration
  *           + Power saving
  *           + Regular/Injected Channels DMA Configuration
  *           + Interrupts and flags management
  *         
  *  @verbatim
================================================================================
                      ##### How to use this driver #####
================================================================================
    [..]
    (#) Enable the SDADC analog interface by calling
        PWR_SDADCAnalogCmd(PWR_SDADCAnalog_x, Enable);
    (#) Enable the SDADC APB clock to get write access to SDADC registers using
        RCC_APB1PeriphClockCmd() function
        e.g.  To enable access to SDADC1 registers use
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SDADC1, ENABLE);
    (#) The SDADCs are clocked by APB1.
        In order to get the SDADC running at the typical frequency (6 MHz
        in fast mode), use SDADC prescaler by calling RCC_SDADCCLKConfig() function
        e.g. if APB1 is clocked at 72MHz, to get the SDADC running at 6MHz
        configure the SDADC prescaler at 12 by calling 
        RCC_SDADCCLKConfig(RCC_SDADCCLK_SYSCLK_Div12);
    (#) If required, perform the following configurations:
        (++) Select the reference voltage using SDADC_VREFSelect() function
        (++) Enable the power-down and standby modes using SDADC_PowerDownCmd()
             and SDADC_StandbyCmd() functions respectively
        (++) Enable the slow clock mode (SDADC running at 1.5 MHz) using
             RCC_SDADCCLKConfig() and SDADC_SlowClockCmd() function
             -@@- These configurations are allowed only when the SDADC is disabled.

    (#) Enable the SDADC peripheral using SDADC_Cmd() function.
    (#) Enter initialization mode using SDADC_InitModeCmd() function
        then wait for INITRDY flag to be set to confirm that the SDADC
        is in initialization mode.
    (#) Configure the analog inputs: gain, single ended mode, offset value and
        commmon mode using SDADC_AINInit().
        There are three possible configuration: SDADC_Conf_0, SDADC_Conf_1 and SDADC_Conf_2
    (#) Associate the selected configuration to the channel using SDADC_ChannelConfig()
    (#) For Regular channels group configuration
        (++) use SDADC_Init() function to select the SDADC channel to be used
             for regular conversion, the continuous mode...
             -@@- Only software trigger or synchro with SDADC1 are possible
                  for regular conversion
    (#) For Injected channels group configuration
        (++) Select the SDADC channel to be used for injected conversion 
             using SDADC_InjectedChannelSelect()
        (++) Select the external trigger SDADC_ExternalTrigInjectedConvConfig()
             and the edge (rising, falling or both) using
             SDADC_ExternalTrigInjectedConvEdgeConfig()
             -@@- Software trigger and synchro with SDADC1 are possible
    (#)  Exit initialization mode using SDADC_InitModeCmd() function

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
#include "stm32f37x_sdadc.h"
#include "stm32f37x_rcc.h"

/** @addtogroup STM32F37x_StdPeriph_Driver
  * @{
  */

/** @defgroup SDADC 
  * @brief SDADC driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* CR2 register Mask */
#define CR2_CLEAR_MASK            ((uint32_t)0xFE30FFFF)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup SDADC_Private_Functions
  * @{
  */

/** @defgroup SDADC_Group1 Initialization and Configuration functions
 *  @brief   Initialization and Configuration functions 
 *
@verbatim    
 ===============================================================================
          ##### Initialization and Configuration functions #####
 ===============================================================================
    [..] This section provides functions allowing to:
        (+) Configure the SDADC analog inputs (gain, offset, single ended...)
        (+) Select the SDADC regular conversion channels
        (+) Enter/Exit the SDADC initialization mode
        (+) SDADC fast conversion conversion mode configuration
        (+) Select the reference voltage
        (+) Enable/disable the SDADC peripheral
        (+) Configure and start the SDADC calibration

@endverbatim
  * @{
  */

/**
  * @brief  Deinitializes SDADCx peripheral registers to their default reset values.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval None
  */
void SDADC_DeInit(SDADC_TypeDef* SDADCx)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));

  if(SDADCx == SDADC1)
  {
    /* Enable SDADC1 reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDADC1, ENABLE);
    /* Release SDADC1 from reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDADC1, DISABLE);
  }
  else if(SDADCx == SDADC2)
  {
    /* Enable SDADC2 reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDADC2, ENABLE);
    /* Release SDADC2 from reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDADC2, DISABLE);
  }
  else
  {
    /* Enable SDADC3 reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDADC3, ENABLE);
    /* Release SDADC3 from reset state */
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDADC3, DISABLE);
  }
}

/**
  * @brief  Initializes the SDADCx peripheral according to the specified parameters
  *         in the SDADC_InitStruct.
  * @note   SDADC_FastConversionMode can be modified only if the SDADC is disabled
  *         or the INITRDY flag is set. Otherwise the configuration can't be modified.
  * @note   Channel selection and continuous mode configuration affect only the 
  *         regular channel.
  * @note   Fast conversion mode is regardless of regular/injected conversion mode.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_InitStruct: pointer to an SDADC_InitTypeDef structure that contains
  *         the configuration information for the specified SDADC peripheral.
  * @retval None
  */
void SDADC_Init(SDADC_TypeDef* SDADCx, SDADC_InitTypeDef* SDADC_InitStruct)
{
  uint32_t tmpcr2 = 0;

  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_REGULAR_CHANNEL(SDADC_InitStruct->SDADC_Channel)); 
  assert_param(IS_FUNCTIONAL_STATE(SDADC_InitStruct->SDADC_ContinuousConvMode)); 
  assert_param(IS_FUNCTIONAL_STATE(SDADC_InitStruct->SDADC_FastConversionMode));

  /*---------------------------- SDADCx CR2 Configuration --------------------*/
  /* Get the SDADCx_CR2 value */
  tmpcr2 = SDADCx->CR2;

  /* Clear FAST, RCONT and RCH bits */
  tmpcr2 &= CR2_CLEAR_MASK;
  /* Configure SDADCx: continuous mode for regular conversion, 
     regular channel and fast conversion mode */
  /* Set RCONT bit according to SDADC_ContinuousConvMode value */
  /* Set FAST bit according to SDADC_FastConversionMode value */
  /* Select the regular channel according to SDADC_Channel value */
  tmpcr2 |= (uint32_t)(((uint32_t)SDADC_InitStruct->SDADC_ContinuousConvMode<<22) |
                       (SDADC_InitStruct->SDADC_FastConversionMode<<(uint32_t)24) |
                       (SDADC_InitStruct->SDADC_Channel & SDADC_CR2_RCH));

  /* Write to SDADCx_CR2 */
  SDADCx->CR2 = tmpcr2;
}

/**
  * @brief  Fills each SDADC_InitStruct member with its default value.  
  * @param  SDADC_InitStruct: pointer to an SDADC_InitTypeDef structure which will 
  *         be initialized.
  * @retval None
  */
void SDADC_StructInit(SDADC_InitTypeDef* SDADC_InitStruct)                            
{
  /* Reset SDADC init structure parameters values */

  /* Initialize the SDADC_Channel member */
  SDADC_InitStruct->SDADC_Channel = SDADC_Channel_0;

  /* Initialize the SDADC_ContinuousConvMode member */
  SDADC_InitStruct->SDADC_ContinuousConvMode = DISABLE;

  /* Initialize the SDADC_FastConversionMode member */
  SDADC_InitStruct->SDADC_FastConversionMode = DISABLE;
}

/**
  * @brief  Configures the analog input mode.
  * @note   This function can be used only if the SDADC is disabled 
  *         or the INITRDY flag is set. Otherwise the configuration can't be modified.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_AINStruct: pointer to an SDADC_AINStructTypeDef structure that contains 
  *         the analog inputs configuration information for the specified SDADC peripheral.
  * @retval None
  */
void SDADC_AINInit(SDADC_TypeDef* SDADCx, uint32_t SDADC_Conf, SDADC_AINStructTypeDef* SDADC_AINStruct)
{
  uint32_t tmp = 0;

  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_CONF(SDADC_Conf));
  assert_param(IS_SDADC_INPUT_MODE(SDADC_AINStruct->SDADC_InputMode));
  assert_param(IS_SDADC_GAIN(SDADC_AINStruct->SDADC_Gain));
  assert_param(IS_SDADC_COMMON_MODE(SDADC_AINStruct->SDADC_CommonMode));
  assert_param(IS_SDADC_OFFSET_VALUE(SDADC_AINStruct->SDADC_Offset));

  /* Get the ASDACx address */
  tmp = (uint32_t)((uint32_t)SDADCx + 0x00000020);
  /* Get the ASDACx CONFxR value: depending SDADC_Conf, analog input configuration
     is set to CONF0R, CONF1R or CONF2R register */
  tmp = (uint32_t)(SDADC_Conf << 2) + tmp;

  /* Set the analog input configuration to the selected CONFxR register */
  *(__IO uint32_t *) (tmp) = (uint32_t) (SDADC_AINStruct->SDADC_InputMode |
                                         SDADC_AINStruct->SDADC_Gain |
                                         SDADC_AINStruct->SDADC_CommonMode |
                                         SDADC_AINStruct->SDADC_Offset);
}

/**
  * @brief  Fills each SDADC_AINStruct member with its default value.
  * @param  SDADC_AINStruct: pointer to an SDADC_AINStructTypeDef structure which will 
  *         be initialized.
  * @retval None
  */
void SDADC_AINStructInit(SDADC_AINStructTypeDef* SDADC_AINStruct)
{
  /* Reset SDADC AIN configuration parameters values */

  /* Initialize the SDADC_Input member */
  SDADC_AINStruct->SDADC_InputMode = SDADC_InputMode_Diff;

  /* Initialize the SDADC_Gain member */
  SDADC_AINStruct->SDADC_Gain = SDADC_Gain_1;

  /* Initialize the SDADC_CommonMode member */
  SDADC_AINStruct->SDADC_CommonMode = SDADC_CommonMode_VSSA;

  /* Initialize the SDADC_Offset member */
  SDADC_AINStruct->SDADC_Offset = 0;
}

/**
  * @brief  Configures the SDADCx channel.
  * @note   SDADC channel configuration can be modified only if the SDADC is disabled
  *         or the INITRDY flag is set. Otherwise the configuration can't be modified.  
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_Channel: The SDADC injected channel.
  *          This parameter can be one of the following values:
  *            @arg SDADC_Channel_0: SDADC Channel 0 selected
  *            @arg SDADC_Channel_1: SDADC Channel 1 selected
  *            @arg SDADC_Channel_2: SDADC Channel 2 selected
  *            @arg SDADC_Channel_3: SDADC Channel 3 selected
  *            @arg SDADC_Channel_4: SDADC Channel 4 selected
  *            @arg SDADC_Channel_5: SDADC Channel 5 selected
  *            @arg SDADC_Channel_6: SDADC Channel 6 selected
  *            @arg SDADC_Channel_7: SDADC Channel 7 selected
  *            @arg SDADC_Channel_8: SDADC Channel 8 selected
  * @param  SDADC_Conf: The SDADC input configuration.
  *          This parameter can be one of the following values:
  *            @arg SDADC_Conf_0: SDADC Conf 0 selected
  *            @arg SDADC_Conf_1: SDADC Conf 1 selected
  *            @arg SDADC_Conf_2: SDADC Conf 2 selected
  * @note   The SDADC configuration (Conf 0, Conf 1, Conf 2) should be performed
  *         using SDADC_AINInit()
  * @retval None
  */
void SDADC_ChannelConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_Channel, uint32_t SDADC_Conf)
{
  uint32_t channelnum = 0;

  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_INJECTED_CHANNEL(SDADC_Channel));
  assert_param(IS_SDADC_CONF(SDADC_Conf));

  /*----------------------- SDADCx CONFCHRx Configuration --------------------*/
  if(SDADC_Channel != SDADC_Channel_8)
  {
    /* Get channel number */
    channelnum = (uint32_t)(SDADC_Channel>>16);
    /* Set the channel configuration */
    SDADCx->CONFCHR1 |= (uint32_t) (SDADC_Conf << (channelnum << 2));
  }
  else
  {
    SDADCx->CONFCHR2 = (uint32_t) (SDADC_Conf);
  }
}

/**
  * @brief  Enables or disables the specified SDADC peripheral.
  * @note   When disabled, power down mode is entered, the flags and the data
  *         are cleared.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the SDADCx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SDADC_Cmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Set the ADON bit to enable the SDADC */
    SDADCx->CR2 |= (uint32_t)SDADC_CR2_ADON;
  }
  else
  {
    /* Reset the ADON bit to disable the SDADC */
    SDADCx->CR2 &= (uint32_t)(~SDADC_CR2_ADON);
  }
}

/**
  * @brief  Enables or disables the initialization mode for specified SDADC peripheral.
  * @note   Initialization mode should be enabled before setting the analog input
  *         configuration, the fast conversion mode, the external trigger...
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the SDADCx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  *         When enabled, the SDADCx is in initialization mode and the SDADCx can
  *         be configured (except: power down mode, standby mode, slow clock and VREF selection).
  *         When disabled, the SDADCx isn't in initialization mode and limited
  *         configurations are allowed (regular channel selection, software trigger)
  * @retval None
  */
void SDADC_InitModeCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Set the INIT bit to enter initialization mode */
    SDADCx->CR1 |= (uint32_t)SDADC_CR1_INIT;
  }
  else
  {
    /* Reset the INIT bit to exit initialization mode */
    SDADCx->CR1 &= (uint32_t)(~SDADC_CR1_INIT);
  }
}

/**
  * @brief  Enables or disables the fast conversion mode for the SDADC.
  * @note   When converting a single channel in continuous mode, having enabled
  *         fast mode causes each conversion (except for the first) to execute
  *         3 times faster (taking 120 SDADC cycles rather than 360).
  *         Fast conversion mode has no meaning for conversions which are not continuous.
  * @note   fast conversion mode can be modified only if the SDADC is disabled
  *         or the INITRDY flag is set. Otherwise the configuration can't be modified.    
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC fast conversion mode
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SDADC_FastConversionCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the fast conversion mode */
    SDADCx->CR2 |= SDADC_CR2_FAST;
  }
  else
  {
    /* Disable the fast conversion mode */
    SDADCx->CR2 &= (uint32_t)(~SDADC_CR2_FAST);
  }
}

/**
  * @brief  Selects the reference voltage.
  * @note   The reference voltage is common to the all SDADCs (SDADC1, SDADC2 and SDADC3).
  *         The reference voltage selection is available only in SDADC1 and therefore
  *         to select the VREF for SDADC2/SDADC3, SDADC1 clock must be already enabled.
  * @note   The reference voltage selection can be performed only when the SDADC
  *         is disabled.
  * @param  SDADC_VREF: Reference voltage selection.
  *          This parameter can be one of the following values:
  *            @arg SDADC_VREF_Ext: The reference voltage is forced externally using VREF pin
  *            @arg SDADC_VREF_VREFINT1: The reference voltage is forced internally to 1.22V VREFINT
  *            @arg SDADC_VREF_VREFINT2: The reference voltage is forced internally to 1.8V VREFINT
  *            @arg SDADC_VREF_VDDA: The reference voltage is forced internally to VDDA
  * @retval None
  */
void SDADC_VREFSelect(uint32_t SDADC_VREF)
{
uint32_t tmpcr1;

  /* Check the parameter */
  assert_param(IS_SDADC_VREF(SDADC_VREF));

  /* Get SDADC1_CR1 register value */
  tmpcr1 = SDADC1->CR1;

  /* Clear the SDADC1_CR1_REFV bits */
  tmpcr1 &= (uint32_t) (~SDADC_CR1_REFV);
  /* Select the reference voltage */
  tmpcr1 |= SDADC_VREF;

  /* Write in SDADC_CR1 */
  SDADC1->CR1 = tmpcr1;
}

/**
  * @brief  Configures the calibration sequence.
  * @note   After calling SDADC_CalibrationSequenceConfig(), use SDADC_StartCalibration()
  *         to start the calibration process.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_CalibrationSequence: Number of calibration sequence to be performed.
  *          This parameter can be one of the following values:
  *            @arg SDADC_CalibrationSequence_1: One calibration sequence will be performed
  *                                      to calculate OFFSET0[11:0] (offset that corresponds to conf0)
  *            @arg SDADC_CalibrationSequence_2: Two calibration sequences will be performed
  *                                      to calculate OFFSET0[11:0] and OFFSET1[11:0]
  *                                      (offsets that correspond to conf0 and conf1)
  *            @arg SDADC_CalibrationSequence_3: Three calibration sequences will be performed
  *                                      to calculate OFFSET0[11:0], OFFSET1[11:0], 
  *                                      and OFFSET2[11:0] (offsets that correspond to conf0, conf1 and conf2)
  * @retval None
  */
void SDADC_CalibrationSequenceConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_CalibrationSequence)
{
  uint32_t tmpcr2;

  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_CALIB_SEQUENCE(SDADC_CalibrationSequence));

  /* Get SDADC_CR2 register value */
  tmpcr2 = SDADCx->CR2;

  /* Clear the SDADC_CR2_CALIBCNT bits */
  tmpcr2 &= (uint32_t) (~SDADC_CR2_CALIBCNT);
  /* Set the calibration sequence */
  tmpcr2 |= SDADC_CalibrationSequence;

  /* Write in SDADC_CR2 */
  SDADCx->CR2 = tmpcr2;
}

/**
  * @brief  Launches a request to start the calibration sequence.
  * @note   use SDADC_CalibrationSequenceConfig() function to configure the 
  *         calibration sequence then call SDADC_StartCalibration() to start the 
  *         calibration process.  
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval None
  */
void SDADC_StartCalibration(SDADC_TypeDef* SDADCx)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));

  /* Request a start of the calibration sequence */
  SDADCx->CR2 |= (uint32_t)SDADC_CR2_STARTCALIB;
}

/**
  * @}
  */

/** @defgroup SDADC_Group2 Regular Channels Configuration functions
 *  @brief   Regular Channels Configuration functions 
 *
@verbatim   
 ===============================================================================
          ##### Regular Channels Configuration functions ##### 
 ===============================================================================  
    [..] This section provides functions allowing to manage the SDADC regular channels,
         it is composed of 3 sub sections: 
        (+) Configuration and management functions for regular channels:
           (++) Select the channel to be used for regular conversion using
                 SDADC_ChannelSelect() (*)
           (++) Activate the continuous Mode using SDADC_ContinuousModeCmd() (*)
           (++) Perform a software start trigger using SDADC_SoftwareStartConv()
           (++) For SDADC2 and SDADC3, Enable synchronization with SDADC1 using
                 SDADC_RegularSynchroSDADC1()
              -@@- Please Note that the following features for regular channels
                   can be configurated using the SDADC_Init() function:
           (++) Channel selection
           (++) Continuous mode activation
        (+) Get the regular conversion data: Use SDADC_GetConversionValue() to 
            read the conversion value for regular conversion
        (+) Get the SDADC2/SDADC3 regular conversion data synchronized with SDADC1
            Use SDADC_GetConversionSDADC12Value()/SDADC_GetConversionSDADC13Value to 
            read the conversion value of SDADC1 regular conversion concatenated to
            SDADC2/SDADC3 regular conversion
@endverbatim
  * @{
  */

/**
  * @brief  Selects the SDADC channel to be used for regular conversion.
  * @note   Just one channel of the 9 available channels can be selected.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_Channel: The SDADC regular channel.
  *          This parameter can be one of the following values:
  *            @arg SDADC_Channel_0: SDADC Channel 0 selected
  *            @arg SDADC_Channel_1: SDADC Channel 1 selected
  *            @arg SDADC_Channel_2: SDADC Channel 2 selected
  *            @arg SDADC_Channel_3: SDADC Channel 3 selected
  *            @arg SDADC_Channel_4: SDADC Channel 4 selected
  *            @arg SDADC_Channel_5: SDADC Channel 5 selected
  *            @arg SDADC_Channel_6: SDADC Channel 6 selected
  *            @arg SDADC_Channel_7: SDADC Channel 7 selected
  *            @arg SDADC_Channel_8: SDADC Channel 8 selected
  * @retval None
  */
void SDADC_ChannelSelect(SDADC_TypeDef* SDADCx, uint32_t SDADC_Channel)
{
uint32_t tmpcr2;

  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_REGULAR_CHANNEL(SDADC_Channel));

  /* Get SDADC_CR2 register value */
  tmpcr2 = SDADCx->CR2;

  /* Clear the RCH[3:0] bits */
  tmpcr2 &= (uint32_t) (~SDADC_CR2_RCH);
  /* Select the regular channel */
  tmpcr2 |= (uint32_t) (SDADC_Channel & 0xFFFF0000);

  /* Write in SDADC_CR2 register */
  SDADCx->CR2 = tmpcr2;
}

/**
  * @brief  Enables or disables the SDADC continuous conversion mode.
  *         When enabled, the regular channel is converted repeatedly after each
  *         conversion request.
  *         When disabled, the regular channel is converted once for each
  *         conversion request.     
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC continuous conversion mode
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SDADC_ContinuousModeCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the selected SDADC continuous conversion mode */
    SDADCx->CR2 |= (uint32_t)SDADC_CR2_RCONT;
  }
  else
  {
    /* Disable the selected SDADC continuous conversion mode */
    SDADCx->CR2 &= (uint32_t)(~SDADC_CR2_RCONT);
  }
}

/**
  * @brief  Enables the selected SDADC software start conversion of the regular channels.
  * @note   If the flag SDADC_FLAG_RCIP is set or INIT bit is set, calling this
  *         function SDADC_SoftwareStartConv() has no effect.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval None
  */
void SDADC_SoftwareStartConv(SDADC_TypeDef* SDADCx)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));

  /* Enable the selected SDADC conversion for regular group */
  SDADCx->CR2 |= (uint32_t)SDADC_CR2_RSWSTART;
}

/**
  * @brief  Returns the last SDADC conversion result data for regular channel.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval The Data conversion value.
  */
int16_t SDADC_GetConversionValue(SDADC_TypeDef* SDADCx)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));

  /* Return the selected SDADC conversion value for regular channel */
  return (int16_t) SDADCx->RDATAR;
}

/**
  * @brief  Launches SDADC2/SDADC3 regular conversion synchronously with SDADC1.
  * @note   This feature is available only on SDADC2 and SDADC3.
  * @param  SDADCx: where x can be 2 or 3 to select the SDADC peripheral.
  *         When enabled, a regular conversion is launched at the same moment
  *         that a regular conversion is launched in SDADC1.
  *         When disabled, do not launch a regular conversion synchronously with SDADC1.
  * @retval None
  */
void SDADC_RegularSynchroSDADC1(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_SLAVE_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable synchronization with SDADC1 on regular conversions */
    SDADCx->CR1 |= SDADC_CR1_RSYNC;
  }
  else
  {
    /* Disable synchronization with SDADC1 on regular conversions */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_RSYNC;
  }
}

/**
  * @brief  Returns the last conversion result data for regular channel of SDADC1 and SDADC2.
  *         RSYNC bit of the SDADC2 should be already set.
  * @param  None
  * @retval The Data conversion value for SDADC1 and SDADC2.
  *         In 16-bit MSB: the regular conversion data for SDADC2.
  *          This data is valid only when the flag SDADC_FLAG_REOC of SDADC2 is set.  
  *         In 16-bit LSB: the regular conversion data for SDADC1.
  *          This data is valid only when the flag SDADC_FLAG_REOC of SDADC1 is set.    
  */
uint32_t SDADC_GetConversionSDADC12Value(void)
{
  /* Return the selected conversion value for regular channel of SDADC1 and SDADC2*/
  return (uint32_t) SDADC1->RDATA12R;
}

/**
  * @brief  Returns the last conversion result data for regular channel of SDADC1 and SDADC3.
  *         RSYNC bit of the SDADC3 should be already set.
  * @param  None
  * @retval The Data conversion value for SDADC1 and SDADC3.
  *         In 16-bit MSB: the regular conversion data for SDADC3.
  *          This data is valid only when the flag SDADC_FLAG_REOC of SDADC3 is set.  
  *         In 16-bit LSB: the regular conversion data for SDADC1.
  *          This data is valid only when the flag SDADC_FLAG_REOC of SDADC1 is set.
  */
uint32_t SDADC_GetConversionSDADC13Value(void)
{
  /* Return the selected conversion value for regular channel of SDADC1 and SDADC3*/
  return (uint32_t) SDADC1->RDATA13R;
}

/**
  * @}
  */

/** @defgroup SDADC_Group3 Injected channels Configuration functions
 *  @brief   Injected channels Configuration functions 
 *
@verbatim   
 ===============================================================================
            ##### Injected channels Configuration functions  #####
 ===============================================================================  

    [..] This section provide functions allowing to configure the SDADC Injected
         channels, it is composed of 2 sub sections:

         (#) Configuration functions for Injected channels: This subsection provides 
             functions allowing to configure the SDADC injected channels:
             (++) Select the configuration  for the SDADC injected channel
             (++) Activate the continuous Mode  
             (++) External/software trigger source   
             (++) External trigger edge (rising, falling, rising & falling)

         (#) Get injected channel conversion data: This subsection provides an
             important function in the SDADC peripheral since it returns the
             converted data of a specific injected channel.

@endverbatim
  * @{
  */

/**
  * @brief  Enables the selected SDADC software start conversion of the injected 
  *         channels.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval None
  */
void SDADC_SoftwareStartInjectedConv(SDADC_TypeDef* SDADCx)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));

  /* Start a conversion of the injected group of channels */
  SDADCx->CR2 |= (uint32_t)SDADC_CR2_JSWSTART;
}
 
/**
  * @brief  Selects the SDADC injected channel(s).
  * @note   When selected, the SDADC channel is part of the injected group
  *         By default, channel 0 is selected
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_Channel: The SDADC injected channel.
  *          This parameter can be one or any combination of the following values:
  *            @arg SDADC_Channel_0: SDADC Channel 0 selected
  *            @arg SDADC_Channel_1: SDADC Channel 1 selected
  *            @arg SDADC_Channel_2: SDADC Channel 2 selected
  *            @arg SDADC_Channel_3: SDADC Channel 3 selected
  *            @arg SDADC_Channel_4: SDADC Channel 4 selected
  *            @arg SDADC_Channel_5: SDADC Channel 5 selected
  *            @arg SDADC_Channel_6: SDADC Channel 6 selected
  *            @arg SDADC_Channel_7: SDADC Channel 7 selected
  *            @arg SDADC_Channel_8: SDADC Channel 8 selected
  * @retval None
  */
void SDADC_InjectedChannelSelect(SDADC_TypeDef* SDADCx, uint32_t SDADC_Channel)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_INJECTED_CHANNEL(SDADC_Channel));

  /* Select the SDADC injected channel */
  SDADCx->JCHGR = (uint32_t) (SDADC_Channel & 0x0000FFFF);
}

/**
  * @brief  Enables or disables delayed start of injected conversions
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC delay start of injected
  *         conversions. This parameter can be: ENABLE or DISABLE.
  *         When disabled, injected conversions begin as soon as possible after
  *         the request.
  *         When enabled, after a request for injected conversion the SDADC waits
  *         a fixed interval before launching the conversion.
  * @retval None
  */
void SDADC_DelayStartInjectedConvCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable delay start of injected conversions */
    SDADCx->CR2 |= (uint32_t) (SDADC_CR2_JDS);
  }
  else
  {
    /* Disable delay start of injected conversions */
    SDADCx->CR2 &= (uint32_t) ~(SDADC_CR2_JDS);
  }
}

/**
  * @brief  Enables or disables the continuous mode for injected channels for
  *         the specified SDADC
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC continuous mode
  *         on injected channels. This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SDADC_InjectedContinuousModeCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the SDADC continuous mode for injected channels */
    SDADCx->CR2 |= (uint32_t)SDADC_CR2_JCONT;
  }
  else
  {
    /* Disable the SDADC continuous mode for injected channels */
    SDADCx->CR2 &= (uint32_t)(~SDADC_CR2_JCONT);
  }
}

/**
  * @brief  Configures the SDADCx external trigger for injected channels conversion.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_ExternalTrigInjecConv: specifies the SDADC trigger to start injected 
  *    conversion. This parameter can be one of the following values:
  *            @arg SDADC_ExternalTrigInjecConv_T13_CC1: Timer13 capture compare1 selected 
  *            @arg SDADC_ExternalTrigInjecConv_T14_CC1: Timer14 TRGO event selected 
  *            @arg SDADC_ExternalTrigInjecConv_T16_CC1: Timer16 TRGO event selected
  *            @arg SDADC_ExternalTrigInjecConv_T17_CC1: Timer17 capture compare1 selected
  *            @arg SDADC_ExternalTrigInjecConv_T12_CC1: Timer12 capture compare1 selected
  *            @arg SDADC_ExternalTrigInjecConv_T12_CC2: Timer12 capture compare2 selected
  *            @arg SDADC_ExternalTrigInjecConv_T15_CC2: Timer15 capture compare2 selected
  *            @arg SDADC_ExternalTrigInjecConv_T2_CC3: Timer2 capture compare3 selected
  *            @arg SDADC_ExternalTrigInjecConv_T2_CC4: Timer2 capture compare4 selected
  *            @arg SDADC_ExternalTrigInjecConv_T3_CC1: Timer3 capture compare1 selected
  *            @arg SDADC_ExternalTrigInjecConv_T3_CC2: Timer3 capture compare2 selected
  *            @arg SDADC_ExternalTrigInjecConv_T3_CC3: Timer3 capture compare3 selected
  *            @arg SDADC_ExternalTrigInjecConv_T4_CC1: Timer4 capture compare1 selected
  *            @arg SDADC_ExternalTrigInjecConv_T4_CC2: Timer4 capture compare2 selected
  *            @arg SDADC_ExternalTrigInjecConv_T4_CC3: Timer4 capture compare3 selected
  *            @arg SDADC_ExternalTrigInjecConv_T19_CC2: Timer19 capture compare2 selected 
  *            @arg SDADC_ExternalTrigInjecConv_T19_CC3: Timer19 capture compare3 selected
  *            @arg SDADC_ExternalTrigInjecConv_T19_CC4: Timer19 capture compare4 selected
  *            @arg SDADC_ExternalTrigInjecConv_T4_CC4: Timer4 capture compare4 selected
  *            @arg SDADC_ExternalTrigInjecConv_Ext_IT11: External interrupt line 11 event selected
  *            @arg SDADC_ExternalTrigInjecConv_Ext_IT15: External interrupt line 15 event selected
  * @retval None
  */
void SDADC_ExternalTrigInjectedConvConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_ExternalTrigInjecConv)
{
  uint32_t tmpreg = 0;

  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_EXT_INJEC_TRIG(SDADC_ExternalTrigInjecConv));

  /* Get the old register value */
  tmpreg = SDADCx->CR2;

  /* Clear the old external trigger selection for injected group */
  tmpreg &= (uint32_t) (~SDADC_CR2_JEXTSEL);
  /* Set the external event selection for injected group */
  tmpreg |= SDADC_ExternalTrigInjecConv;

  /* Store the new register value */
  SDADCx->CR2 = tmpreg;
}

/**
  * @brief  Configures the SDADCx external trigger edge for injected channels conversion.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_ExternalTrigInjecConvEdge: specifies the SDADC external trigger
  *         edge to start injected conversion.
  *          This parameter can be one of the following values:
  *            @arg SDADC_ExternalTrigInjecConvEdge_None: external trigger disabled for
  *          injected conversion
  *            @arg SDADC_ExternalTrigInjecConvEdge_Rising: Each rising edge on the selected
  *          trigger makes a request to launch a injected conversion
  *            @arg SDADC_ExternalTrigInjecConvEdge_Falling: Each falling edge on the selected
  *          trigger makes a request to launch a injected conversion
  *            @arg SDADC_ExternalTrigInjecConvEdge_RisingFalling: Both rising edges and 
  *          falling edges on the selected trigger make requests to launch injected conversions.
  * @retval None
  */
void SDADC_ExternalTrigInjectedConvEdgeConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_ExternalTrigInjecConvEdge)
{
  uint32_t tmpreg = 0;

  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_EXT_INJEC_TRIG_EDGE(SDADC_ExternalTrigInjecConvEdge));

  /* Get the old register value */
  tmpreg = SDADCx->CR2;

  /* Clear the old external trigger edge for injected group */
  tmpreg &= (uint32_t) (~SDADC_CR2_JEXTEN);
  /* Set the new external trigger edge for injected group */
  tmpreg |= SDADC_ExternalTrigInjecConvEdge;

  /* Store the new register value */
  SDADCx->CR2 = tmpreg;
}

/**
  * @brief  Returns the injected channel most recently converted for
  *         the specified SDADC
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @retval The most recently converted SDADC injected channel.
  *          This parameter can be one of the following values:
  *            @arg 0x00000001: SDADC_Channel_0 is recently converted
  *            @arg 0x00010002: SDADC_Channel_1 is recently converted
  *            @arg 0x00020004: SDADC_Channel_2 is recently converted
  *            @arg 0x00030008: SDADC_Channel_3 is recently converted
  *            @arg 0x00040010: SDADC_Channel_4 is recently converted
  *            @arg 0x00050020: SDADC_Channel_5 is recently converted
  *            @arg 0x00060040: SDADC_Channel_6 is recently converted
  *            @arg 0x00070080: SDADC_Channel_7 is recently converted
  *            @arg 0x00080100: SDADC_Channel_8 is recently converted
  */
uint32_t SDADC_GetInjectedChannel(SDADC_TypeDef* SDADCx)
{
  uint32_t temp = 0;
  
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));

  /* Get the injected channel most recently converted */
  temp = (uint32_t)(SDADCx->JDATAR>>16);
  temp = (uint32_t) (((uint32_t)1<<temp) | (temp<<(uint32_t)16));

  /* Returns the injected channel most recently converted */
  return (uint32_t) (temp);
}

/**
  * @brief  Returns the SDADC injected channel conversion result
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_Channel: the most recently converted SDADC injected channel.
  *          This parameter can be one of the following values:
  *            @arg 0x00000001: SDADC_Channel_0 is recently converted 
  *            @arg 0x00010002: SDADC_Channel_1 is recently converted
  *            @arg 0x00020004: SDADC_Channel_2 is recently converted
  *            @arg 0x00030008: SDADC_Channel_3 is recently converted
  *            @arg 0x00040010: SDADC_Channel_4 is recently converted
  *            @arg 0x00050020: SDADC_Channel_5 is recently converted
  *            @arg 0x00060040: SDADC_Channel_6 is recently converted
  *            @arg 0x00070080: SDADC_Channel_7 is recently converted
  *            @arg 0x00080100: SDADC_Channel_8 is recently converted
  * @retval The injected data conversion value.
  */
int16_t SDADC_GetInjectedConversionValue(SDADC_TypeDef* SDADCx, uint32_t* SDADC_Channel)
{
  uint32_t tmp = 0;

  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));

  /* Get SDADC_JDATAR register */
  tmp = (uint32_t)SDADCx->JDATAR;

  /* Get the injected channel most recently converted */
   *(uint32_t*)SDADC_Channel = (uint32_t) ((uint32_t)((tmp>>8) &0xffff0000) | (((uint32_t)1<<(tmp>>24))));

  /* Returns the injected channel conversion data */
  return (int16_t) ((uint32_t)(tmp & 0x0000FFFF));
}

/**
  * @brief  Launches injected conversion synchronously with SDADC1.
  * @note   This feature is available only on SDADC2 and SDADC3.
  * @param  SDADCx: where x can be 2 or 3 to select the SDADC peripheral.
  * @param  NewState: new state of the selected SDADC synchronization with SDADC1
  *          This parameter can be: ENABLE or DISABLE.
  *         When enabled, An injected conversion is launched at the same moment
  *         that an injected conversion is launched in SDADC1.
  *         When disabled, do not launch an injected conversion synchronously with SDADC1.
  * @retval None
  */
void SDADC_InjectedSynchroSDADC1(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_SLAVE_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable synchronization with SDADC1 on injected conversions */
    SDADCx->CR1 |= SDADC_CR1_JSYNC;
  }
  else
  {
    /* Disable synchronization with SDADC1 on injected conversions */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_JSYNC;
  }
}

/**
  * @brief  Returns the last conversion result data for injected channel of SDADC1 and SDADC2.
  *         JSYNC bit of the SDADC2 should be already set.
  * @param  None
  * @retval The Data conversion value for SDADC1 and SDADC2.
  *         In 16-bit MSB: the regular conversion data for SDADC2.
  *          This data is valid only when the flag SDADC_FLAG_JEOC of SDADC2 is set.  
  *         In 16-bit LSB: the regular conversion data for SDADC1.
  *          This data is valid only when the flag SDADC_FLAG_JEOC of SDADC1 is set.    
  */
uint32_t SDADC_GetInjectedConversionSDADC12Value(void)
{
  /* Return the selected conversion value for injected channel of SDADC1 and SDADC2*/
  return (uint32_t) SDADC1->JDATA12R;
}

/**
  * @brief  Returns the last conversion result data for injected channel of SDADC1 and SDADC3.
  *         JSYNC bit of the SDADC3 should be already set.
  * @param  None
  * @retval The Data conversion value for SDADC1 and SDADC3.
  *         In 16-bit MSB: the injected conversion data for SDADC3.
  *          This data is valid only when the flag SDADC_FLAG_JEOC of SDADC3 is set.  
  *         In 16-bit LSB: the injected conversion data for SDADC1.
  *          This data is valid only when the flag SDADC_FLAG_JEOC of SDADC1 is set.
  */
uint32_t SDADC_GetInjectedConversionSDADC13Value(void)
{
  /* Return the selected conversion value for injected channel of SDADC1 and SDADC3*/
  return (uint32_t) SDADC1->JDATA13R;
}

/**
  * @}
  */

/** @defgroup SDADC_Group4 Power saving functions
 *  @brief   Power saving functions 
 *
@verbatim   
 ===============================================================================
          ##### Power saving functions #####
 ===============================================================================  
    [..] This section provides functions allowing to reduce power consumption: 
        (+) Enable the power down mode when idle using SDADC_PowerDownCmd();
        (+) Enable the standby mode when idle using SDADC_StandbyCmd();
        (+) Enable the slow clock mode using SDADC_SlowClockCmd()

@endverbatim
  * @{
  */

/**
  * @brief  Enables or disables the SDADC power down mode when idle.
  * @note   SDADC power down mode when idle is used to cut the consumption when
  *         the SDADC is not converting (when idle).
  * @note   When the SDADC is in power down mode and a conversion is requested, 
  *         the SDADC takes 100us to stabilize before launching the conversion.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.  
  * @param  NewState: new state of the selected SDADC power down mode when idle
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SDADC_PowerDownCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the SDADC power-down when idle */
    SDADCx->CR1 |= SDADC_CR1_PDI;
  }
  else
  {
    /* Disable the SDADCx power-down when idle */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_PDI;
  }
}

/**
  * @brief  Enables or disables the SDADC standby mode when idle.
  * @note   SDADC standby mode when idle is used to cut the consumption when
  *         the SDADC is not converting (when idle).
  * @note   When the SDADC is in standby mode and a conversion is requested, 
  *         the SDADC takes 50us to stabilize before launching the conversion.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.  
  * @param  NewState: new state of the selected SDADC standby mode when idle
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SDADC_StandbyCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the standby mode when idle */
    SDADCx->CR1 |= SDADC_CR1_SBI;
  }
  else
  {
    /* Disable the standby mode when idle */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_SBI;
  }
}

/**
  * @brief  Enables or disables the SDADC in slow clock mode.
  * @note   Slow clock mode (where the SDADC clock frequency should be 1.5MHz)
  *         allowing a lower level of current consumption as well as operation
  *         at a lower minimum voltage.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.  
  * @param  NewState: new state of the selected SDADC slow clock mode
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SDADC_SlowClockCmd(SDADC_TypeDef* SDADCx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the slow clock mode */
    SDADCx->CR1 |= SDADC_CR1_SLOWCK;
  }
  else
  {
    /* Disable the slow clock mode */
    SDADCx->CR1 &= (uint32_t)~SDADC_CR1_SLOWCK;
  }
}

/**
  * @}
  */

/** @defgroup SDADC_Group5 Regular/Injected Channels DMA Configuration function
 *  @brief   Regular/Injected Channels DMA Configuration functions 
 *
@verbatim   
 ===============================================================================
          ##### Regular Channels DMA Configuration functions ##### 
 ===============================================================================  
    [..] This section provides functions allowing to configure the DMA for SDADC regular 
         or injected channels.
    [..] Since converted value is stored into a unique data register, it is useful
         to use DMA for conversion of more than one channel.
         This avoids the loss of the data already stored in the SDADC Data register. 
         When the DMA is enabled for regular/injected channel (using the SDADC_DMAConfig()
         function), after each conversion of a regular/injected channel,
         a DMA request is generated.

@endverbatim
  * @{
  */

/**
  * @brief  Configures the DMA transfer for regular/injected conversions.  
  * @note   DMA requests can't be enabled for both regular and injected conversions.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_DMATransfer: Specifies the SDADC DMA transfer.
  *          This parameter can be one of the following values:
  *          @arg SDADC_DMATransfer_Regular: When enabled, DMA manages reading the 
  *          data for the regular channel.
  *          @arg SDADC_DMATransfer_Injected: When enabled, DMA manages reading the
  *          data for the injected channel.
  * @param  NewState Indicates the new state of the SDADC DMA interface.
  *           This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
void SDADC_DMAConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_DMATransfer, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_DMA_TRANSFER(SDADC_DMATransfer));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the DMA transfer */
    SDADCx->CR1 |= SDADC_DMATransfer;
  }
  else
  {
    /* Disable the DMA transfer */
    SDADCx->CR1 &= (uint32_t)(~SDADC_DMATransfer);
  }
}

/**
  * @}
  */

/** @defgroup SDADC_Group6 Interrupts and flags management functions
 *  @brief   Interrupts and flags management functions
 *
@verbatim   
 ===============================================================================
          ##### Interrupts and flags management functions ##### 
 ===============================================================================  
    [..] This section provides functions allowing to configure the SDADC Interrupts and
         get the status and clear flags and Interrupts pending bits.

    [..] The SDADC provide 5 Interrupts sources and 10 Flags which can be divided into 3 groups:
  
  *** Flags and Interrupts for SDADC regular channels ***
  ======================================================  
    [..]
        (+)Flags :
           (##) SDADC_FLAG_ROVR : Overrun detection when regular converted data are lost.
           (##) SDADC_FLAG_REOC : Regular channel end of conversion.
           (##) SDADC_FLAG_RCIP: Regular conversion in progress.
     
        (+)Interrupts :
           (##) SDADC_IT_REOC : specifies the interrupt source for end of regular conversion event.
           (##) SDADC_IT_ROVRF : specifies the interrupt source for regular conversion overrun event.
  
  
  *** Flags and Interrupts for SDADC Injected channels ***
  ======================================================
    [..]
        (+)Flags :
           (##) SDADC_FLAG_JEOC : Injected channel end of conversion event.
           (##) SDADC_FLAG_JOVR: Injected channel Overrun detection event.
           (##) SDADC_FLAG_JCIP: Injected channel conversion in progress.

        (+)Interrupts :
           (##) SDADC_IT_JEOC: specifies the interrupt source for end of injected
                        conversion event.
           (##) SDADC_IT_JOVR: specifies the interrupt source for injected conersion
                        overrun event.

  *** General Flags and Interrupts for the SDADC ***
  ================================================
    [..]
        (+)Flags :
           (##) SDADC_FLAG_EOCAL: specifies the end of calibration event.
           (##) SDADC_FLAG_CALIBIP: specifies that calibration is in progress.
           (##) SDADC_FLAG_STABIP:  specifies that stabilization is in progress.
           (##) SDADC_FLAG_INITRDY: specifies that initialization mode is ready .

        (+)Interrupts :
           (##) SDADC_IT_EOCAL : specifies the interrupt source for end of calibration event.

    [..] User should identify which mode will be used in his application to manage 
         the SDADC controller events: Polling mode or Interrupt mode.

    [..] In the Polling Mode it is advised to use the following functions:
         (+) SDADC_GetFlagStatus(): to check if flags events occur.
         (+) SDADC_ClearFlag()    : to clear the flags events.

    [..] In the Interrupt Mode it is advised to use the following functions:
         (+) SDADC_ITConfig()         : to enable or disable the interrupt source.
         (+) SDADC_GetITStatus()      : to check if Interrupt occurs.
         (+) SDADC_ClearITPendingBit(): to clear the Interrupt pending Bit 
                                        (corresponding Flag). 
@endverbatim
  * @{
  */ 

/**
  * @brief  Enables or disables the specified SDADC interrupts.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_IT: specifies the SDADC interrupt sources to be enabled or disabled. 
  *          This parameter can be one of the following values:
  *            @arg SDADC_IT_EOCAL: End of calibration interrupt
  *            @arg SDADC_IT_JEOC: End of injected conversion interrupt
  *            @arg SDADC_IT_JOVR: Injected conversion overrun interrupt
  *            @arg SDADC_IT_REOC: End of regular conversion interrupt
  *            @arg SDADC_IT_ROVR: Regular conversion overrun interrupt
  * @param  NewState: new state of the specified SDADC interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SDADC_ITConfig(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT, FunctionalState NewState)  
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  assert_param(IS_SDADC_IT(SDADC_IT)); 

  if (NewState != DISABLE)
  {
    /* Enable the selected SDADC interrupts */
    SDADCx->CR1 |= SDADC_IT;
  }
  else
  {
    /* Disable the selected SDADC interrupts */
    SDADCx->CR1 &= ((uint32_t)~SDADC_IT);
  }
}

/**
  * @brief  Checks whether the specified SDADC flag is set or not.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_FLAG: specifies the flag to check. 
  *          This parameter can be one of the following values:
  *            @arg SDADC_FLAG_EOCAL: End of calibration flag
  *            @arg SDADC_FLAG_JEOC: End of injected conversion flag
  *            @arg SDADC_FLAG_JOVR: Injected conversion overrun flag
  *            @arg SDADC_FLAG_REOC: End of regular conversion flag
  *            @arg SDADC_FLAG_ROVR: Regular conversion overrun flag
  *            @arg SDADC_FLAG_CALIBIP:Calibration in progress status flag
  *            @arg SDADC_FLAG_JCIP: Injected conversion in progress status flag
  *            @arg SDADC_FLAG_RCIP: Regular conversion in progress status flag
  *            @arg SDADC_FLAG_STABIP: Stabilization in progress status flag
  *            @arg SDADC_FLAG_INITRDY: Initialization mode is ready
  * @retval The new state of SDADC_FLAG (SET or RESET).
  */
FlagStatus SDADC_GetFlagStatus(SDADC_TypeDef* SDADCx, uint32_t SDADC_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_GET_FLAG(SDADC_FLAG));

  /* Check the status of the specified SDADC flag */
  if ((SDADCx->ISR & SDADC_FLAG) != (uint32_t)RESET)
  {
    /* SDADC_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* SDADC_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the SDADC_FLAG status */
  return  bitstatus;
}

/**
  * @brief  Clears the SDADCx pending flags.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg SDADC_FLAG_EOCAL: End of calibration flag
  *            @arg SDADC_FLAG_JOVR: Injected conversion overrun flag
  *            @arg SDADC_FLAG_ROVR: Regular conversion overrun flag
  * @retval None
  */
void SDADC_ClearFlag(SDADC_TypeDef* SDADCx, uint32_t SDADC_FLAG)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_CLEAR_FLAG(SDADC_FLAG));

  /* Clear the selected SDADC flags */
  SDADCx->CLRISR |= (uint32_t)SDADC_FLAG;
}

/**
  * @brief  Checks whether the specified SDADC interrupt has occurred or not.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_IT: specifies the SDADC interrupt source to check. 
  *          This parameter can be one of the following values:
  *            @arg SDADC_IT_EOCAL: End of calibration flag 
  *            @arg SDADC_IT_JEOC: End of injected conversion flag
  *            @arg SDADC_IT_JOVR: Injected conversion overrun flag
  *            @arg SDADC_IT_REOC: End of regular conversion flag
  *            @arg SDADC_IT_ROVR: Regular conversion overrun flag
  * @retval The new state of SDADC_IT (SET or RESET).
  */
ITStatus SDADC_GetITStatus(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT)
{
  ITStatus bitstatus = RESET;
  uint32_t itstatus = 0, enablestatus = 0;
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_GET_IT(SDADC_IT));

  /* Get the SDADC interrupt pending status */
  itstatus = (uint32_t) (SDADC_IT & SDADCx->ISR);
  /* Get the SDADC IT enable bit status */
  enablestatus = (SDADCx->CR1 & (uint32_t)SDADC_IT);

  /* Check the status of the specified SDADC interrupt */
  if ((itstatus != (uint32_t)RESET) && (enablestatus != (uint32_t)RESET))
  {
    /* SDADC_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* SDADC_IT is reset */
    bitstatus = RESET;
  }
  /* Return the SDADC_IT status */
  return  bitstatus;
}

/**
  * @brief  Clears the SDADCx interrupt pending bits.
  * @param  SDADCx: where x can be 1, 2 or 3 to select the SDADC peripheral.
  * @param  SDADC_IT: specifies the SDADC interrupt pending bit to clear.
  *          This parameter can be any combination of the following values:
  *            @arg SDADC_IT_EOCAL: End of calibration flag 
  *            @arg SDADC_IT_JOVR: Injected conversion overrun flag
  *            @arg SDADC_IT_ROVR: Regular conversion overrun flag
  * @retval None
  */
void SDADC_ClearITPendingBit(SDADC_TypeDef* SDADCx, uint32_t SDADC_IT)
{
  /* Check the parameters */
  assert_param(IS_SDADC_ALL_PERIPH(SDADCx));
  assert_param(IS_SDADC_CLEAR_IT(SDADC_IT));

  /* Clear the selected SDADC interrupt pending bits */
  SDADCx->CLRISR |= (uint32_t)SDADC_IT;
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
