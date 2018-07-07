/**
  ******************************************************************************
  * @file    stm32f30x_fmc.h
  * @author  MCD Application Team
  * @version V1.2.3
  * @date    10-July-2015
  * @brief   This file contains all the functions prototypes for the FMC firmware 
  *          library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
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
#ifndef __STM32F30x_FMC_H
#define __STM32F30x_FMC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"

/** @addtogroup STM32F30x_StdPeriph_Driver
  * @{
  */

/** @addtogroup FMC
  * @{
  */

/* Exported types ------------------------------------------------------------*/  
   
/** 
  * @brief  Timing parameters For NOR/SRAM Banks  
  */
typedef struct
{
  uint32_t FMC_AddressSetupTime;       /*!< Defines the number of HCLK cycles to configure
                                             the duration of the address setup time. 
                                             This parameter can be a value between 0 and 15.
                                             @note This parameter is not used with synchronous NOR Flash memories. */

  uint32_t FMC_AddressHoldTime;        /*!< Defines the number of HCLK cycles to configure
                                             the duration of the address hold time.
                                             This parameter can be a value between 1 and 15. 
                                             @note This parameter is not used with synchronous NOR Flash memories.*/

  uint32_t FMC_DataSetupTime;          /*!< Defines the number of HCLK cycles to configure
                                             the duration of the data setup time.
                                             This parameter can be a value between 1 and 255.
                                             @note This parameter is used for SRAMs, ROMs and asynchronous multiplexed NOR Flash memories. */

  uint32_t FMC_BusTurnAroundDuration;  /*!< Defines the number of HCLK cycles to configure
                                             the duration of the bus turnaround.
                                             This parameter can be a value between 0 and 15.
                                             @note This parameter is only used for multiplexed NOR Flash memories. */

  uint32_t FMC_CLKDivision;            /*!< Defines the period of CLK clock output signal, expressed in number of HCLK cycles.
                                             This parameter can be a value between 2 and 16.
                                             @note This parameter is not used for asynchronous NOR Flash, SRAM or ROM accesses. */

  uint32_t FMC_DataLatency;            /*!< Defines the number of memory clock cycles to issue
                                             to the memory before getting the first data.
                                             The parameter value depends on the memory type as shown below:
                                              - It must be set to 0 in case of a CRAM
                                              - It is don't care in asynchronous NOR, SRAM or ROM accesses
                                              - It may assume a value between 2 and 17 in NOR Flash memories
                                                with synchronous burst mode enable */

  uint32_t FMC_AccessMode;             /*!< Specifies the asynchronous access mode. 
                                             This parameter can be a value of @ref FMC_Access_Mode */
}FMC_NORSRAMTimingInitTypeDef;

/** 
  * @brief  FMC NOR/SRAM Init structure definition
  */
typedef struct
{
  uint32_t FMC_Bank;                /*!< Specifies the NOR/SRAM memory bank that will be used.
                                          This parameter can be a value of @ref FMC_NORSRAM_Bank */

  uint32_t FMC_DataAddressMux;      /*!< Specifies whether the address and data values are
                                          multiplexed on the databus or not. 
                                          This parameter can be a value of @ref FMC_Data_Address_Bus_Multiplexing */

  uint32_t FMC_MemoryType;          /*!< Specifies the type of external memory attached to
                                          the corresponding memory bank.
                                          This parameter can be a value of @ref FMC_Memory_Type */

  uint32_t FMC_MemoryDataWidth;     /*!< Specifies the external memory device width.
                                          This parameter can be a value of @ref FMC_NORSRAM_Data_Width */

  uint32_t FMC_BurstAccessMode;     /*!< Enables or disables the burst access mode for Flash memory,
                                          valid only with synchronous burst Flash memories.
                                          This parameter can be a value of @ref FMC_Burst_Access_Mode */                                        

  uint32_t FMC_WaitSignalPolarity;  /*!< Specifies the wait signal polarity, valid only when accessing
                                          the Flash memory in burst mode.
                                          This parameter can be a value of @ref FMC_Wait_Signal_Polarity */

  uint32_t FMC_WrapMode;            /*!< Enables or disables the Wrapped burst access mode for Flash
                                          memory, valid only when accessing Flash memories in burst mode.
                                          This parameter can be a value of @ref FMC_Wrap_Mode */

  uint32_t FMC_WaitSignalActive;    /*!< Specifies if the wait signal is asserted by the memory one
                                          clock cycle before the wait state or during the wait state,
                                          valid only when accessing memories in burst mode. 
                                          This parameter can be a value of @ref FMC_Wait_Timing */

  uint32_t FMC_WriteOperation;      /*!< Enables or disables the write operation in the selected bank by the FMC. 
                                          This parameter can be a value of @ref FMC_Write_Operation */

  uint32_t FMC_WaitSignal;          /*!< Enables or disables the wait state insertion via wait
                                          signal, valid for Flash memory access in burst mode. 
                                          This parameter can be a value of @ref FMC_Wait_Signal */

  uint32_t FMC_ExtendedMode;        /*!< Enables or disables the extended mode.
                                          This parameter can be a value of @ref FMC_Extended_Mode */
  
  uint32_t FMC_AsynchronousWait;     /*!< Enables or disables wait signal during asynchronous transfers,
                                          valid only with asynchronous Flash memories.
                                          This parameter can be a value of @ref FMC_AsynchronousWait */  

  uint32_t FMC_WriteBurst;          /*!< Enables or disables the write burst operation.
                                          This parameter can be a value of @ref FMC_Write_Burst */

  
  FMC_NORSRAMTimingInitTypeDef* FMC_ReadWriteTimingStruct; /*!< Timing Parameters for write and read access if the  Extended Mode is not used*/  

  FMC_NORSRAMTimingInitTypeDef* FMC_WriteTimingStruct;     /*!< Timing Parameters for write access if the  Extended Mode is used*/      
}FMC_NORSRAMInitTypeDef;

/** 
  * @brief  Timing parameters For FMC NAND and PCCARD Banks
  */
typedef struct
{
  uint32_t FMC_SetupTime;      /*!< Defines the number of HCLK cycles to setup address before
                                     the command assertion for NAND-Flash read or write access
                                     to common/Attribute or I/O memory space (depending on
                                     the memory space timing to be configured).
                                     This parameter can be a value between 0 and 255.*/

  uint32_t FMC_WaitSetupTime;  /*!< Defines the minimum number of HCLK cycles to assert the
                                     command for NAND-Flash read or write access to
                                     common/Attribute or I/O memory space (depending on the
                                     memory space timing to be configured). 
                                     This parameter can be a number between 0 and 255 */

  uint32_t FMC_HoldSetupTime;  /*!< Defines the number of HCLK clock cycles to hold address
                                     (and data for write access) after the command de-assertion
                                     for NAND-Flash read or write access to common/Attribute
                                     or I/O memory space (depending on the memory space timing
                                     to be configured).
                                     This parameter can be a number between 0 and 255 */

  uint32_t FMC_HiZSetupTime;   /*!< Defines the number of HCLK clock cycles during which the
                                     databus is kept in HiZ after the start of a NAND-Flash
                                     write access to common/Attribute or I/O memory space (depending
                                     on the memory space timing to be configured).
                                     This parameter can be a number between 0 and 255 */
}FMC_NAND_PCCARDTimingInitTypeDef;

/** 
  * @brief  FMC NAND Init structure definition
  */
typedef struct
{
  uint32_t FMC_Bank;              /*!< Specifies the NAND memory bank that will be used.
                                      This parameter can be a value of @ref FMC_NAND_Bank */

  uint32_t FMC_Waitfeature;      /*!< Enables or disables the Wait feature for the NAND Memory Bank.
                                       This parameter can be any value of @ref FMC_Wait_feature */

  uint32_t FMC_MemoryDataWidth;  /*!< Specifies the external memory device width.
                                       This parameter can be any value of @ref FMC_NAND_Data_Width */

  uint32_t FMC_ECC;              /*!< Enables or disables the ECC computation.
                                       This parameter can be any value of @ref FMC_ECC */

  uint32_t FMC_ECCPageSize;      /*!< Defines the page size for the extended ECC.
                                       This parameter can be any value of @ref FMC_ECC_Page_Size */

  uint32_t FMC_TCLRSetupTime;    /*!< Defines the number of HCLK cycles to configure the
                                       delay between CLE low and RE low.
                                       This parameter can be a value between 0 and 255. */

  uint32_t FMC_TARSetupTime;     /*!< Defines the number of HCLK cycles to configure the
                                       delay between ALE low and RE low.
                                       This parameter can be a number between 0 and 255 */ 

  FMC_NAND_PCCARDTimingInitTypeDef*  FMC_CommonSpaceTimingStruct;   /*!< FMC Common Space Timing */ 

  FMC_NAND_PCCARDTimingInitTypeDef*  FMC_AttributeSpaceTimingStruct; /*!< FMC Attribute Space Timing */
}FMC_NANDInitTypeDef;

/** 
  * @brief  FMC PCCARD Init structure definition
  */

typedef struct
{
  uint32_t FMC_Waitfeature;    /*!< Enables or disables the Wait feature for the Memory Bank.
                                    This parameter can be any value of @ref FMC_Wait_feature */

  uint32_t FMC_TCLRSetupTime;  /*!< Defines the number of HCLK cycles to configure the
                                     delay between CLE low and RE low.
                                     This parameter can be a value between 0 and 255. */

  uint32_t FMC_TARSetupTime;   /*!< Defines the number of HCLK cycles to configure the
                                     delay between ALE low and RE low.
                                     This parameter can be a number between 0 and 255 */ 

  
  FMC_NAND_PCCARDTimingInitTypeDef*  FMC_CommonSpaceTimingStruct; /*!< FMC Common Space Timing */

  FMC_NAND_PCCARDTimingInitTypeDef*  FMC_AttributeSpaceTimingStruct;  /*!< FMC Attribute Space Timing */ 
  
  FMC_NAND_PCCARDTimingInitTypeDef*  FMC_IOSpaceTimingStruct; /*!< FMC IO Space Timing */  
}FMC_PCCARDInitTypeDef;


/* Exported constants --------------------------------------------------------*/

/** @defgroup FMC_Exported_Constants
  * @{
  */ 

/** @defgroup FMC_NORSRAM_Bank 
  * @{
  */
#define FMC_Bank1_NORSRAM1                      ((uint32_t)0x00000000)
#define FMC_Bank1_NORSRAM2                      ((uint32_t)0x00000002)
#define FMC_Bank1_NORSRAM3                      ((uint32_t)0x00000004)
#define FMC_Bank1_NORSRAM4                      ((uint32_t)0x00000006)

#define IS_FMC_NORSRAM_BANK(BANK) (((BANK) == FMC_Bank1_NORSRAM1) || \
                                   ((BANK) == FMC_Bank1_NORSRAM2) || \
                                   ((BANK) == FMC_Bank1_NORSRAM3) || \
                                   ((BANK) == FMC_Bank1_NORSRAM4))
/**
  * @}
  */

/** @defgroup FMC_NAND_Bank 
  * @{
  */  
#define FMC_Bank2_NAND                          ((uint32_t)0x00000010)
#define FMC_Bank3_NAND                          ((uint32_t)0x00000100)

#define IS_FMC_NAND_BANK(BANK) (((BANK) == FMC_Bank2_NAND) || \
                                ((BANK) == FMC_Bank3_NAND))
/**
  * @}
  */

/** @defgroup FMC_PCCARD_Bank 
  * @{
  */    
#define FMC_Bank4_PCCARD                        ((uint32_t)0x00001000)
/**
  * @}                                                         
  */                              

                              
/** @defgroup FMC_NOR_SRAM_Controller 
  * @{
  */

/** @defgroup FMC_Data_Address_Bus_Multiplexing 
  * @{
  */

#define FMC_DataAddressMux_Disable                ((uint32_t)0x00000000)
#define FMC_DataAddressMux_Enable                 ((uint32_t)0x00000002)

#define IS_FMC_MUX(MUX) (((MUX) == FMC_DataAddressMux_Disable) || \
                         ((MUX) == FMC_DataAddressMux_Enable))
/**
  * @}
  */

/** @defgroup FMC_Memory_Type 
  * @{
  */

#define FMC_MemoryType_SRAM                     ((uint32_t)0x00000000)
#define FMC_MemoryType_PSRAM                    ((uint32_t)0x00000004)
#define FMC_MemoryType_NOR                      ((uint32_t)0x00000008)

#define IS_FMC_MEMORY(MEMORY) (((MEMORY) == FMC_MemoryType_SRAM) || \
                               ((MEMORY) == FMC_MemoryType_PSRAM)|| \
                               ((MEMORY) == FMC_MemoryType_NOR))
/**
  * @}
  */

/** @defgroup FMC_NORSRAM_Data_Width 
  * @{
  */

#define FMC_NORSRAM_MemoryDataWidth_8b                  ((uint32_t)0x00000000)
#define FMC_NORSRAM_MemoryDataWidth_16b                 ((uint32_t)0x00000010)

#define IS_FMC_NORSRAM_MEMORY_WIDTH(WIDTH) (((WIDTH) == FMC_NORSRAM_MemoryDataWidth_8b)  || \
                                            ((WIDTH) == FMC_NORSRAM_MemoryDataWidth_16b))
/**
  * @}
  */

/** @defgroup FMC_Burst_Access_Mode 
  * @{
  */

#define FMC_BurstAccessMode_Disable             ((uint32_t)0x00000000) 
#define FMC_BurstAccessMode_Enable              ((uint32_t)0x00000100)

#define IS_FMC_BURSTMODE(STATE) (((STATE) == FMC_BurstAccessMode_Disable) || \
                                  ((STATE) == FMC_BurstAccessMode_Enable))
/**
  * @}
  */
    
/** @defgroup FMC_AsynchronousWait 
  * @{
  */
#define FMC_AsynchronousWait_Disable            ((uint32_t)0x00000000)
#define FMC_AsynchronousWait_Enable             ((uint32_t)0x00008000)

#define IS_FMC_ASYNWAIT(STATE) (((STATE) == FMC_AsynchronousWait_Disable) || \
                                 ((STATE) == FMC_AsynchronousWait_Enable))
/**
  * @}
  */

/** @defgroup FMC_Wait_Signal_Polarity 
  * @{
  */
#define FMC_WaitSignalPolarity_Low              ((uint32_t)0x00000000)
#define FMC_WaitSignalPolarity_High             ((uint32_t)0x00000200)

#define IS_FMC_WAIT_POLARITY(POLARITY) (((POLARITY) == FMC_WaitSignalPolarity_Low) || \
                                         ((POLARITY) == FMC_WaitSignalPolarity_High))
/**
  * @}
  */

/** @defgroup FMC_Wrap_Mode 
  * @{
  */
#define FMC_WrapMode_Disable                    ((uint32_t)0x00000000)
#define FMC_WrapMode_Enable                     ((uint32_t)0x00000400) 

#define IS_FMC_WRAP_MODE(MODE) (((MODE) == FMC_WrapMode_Disable) || \
                                 ((MODE) == FMC_WrapMode_Enable))
/**
  * @}
  */

/** @defgroup FMC_Wait_Timing 
  * @{
  */
#define FMC_WaitSignalActive_BeforeWaitState    ((uint32_t)0x00000000)
#define FMC_WaitSignalActive_DuringWaitState    ((uint32_t)0x00000800) 

#define IS_FMC_WAIT_SIGNAL_ACTIVE(ACTIVE) (((ACTIVE) == FMC_WaitSignalActive_BeforeWaitState) || \
                                            ((ACTIVE) == FMC_WaitSignalActive_DuringWaitState))
/**
  * @}
  */

/** @defgroup FMC_Write_Operation 
  * @{
  */
#define FMC_WriteOperation_Disable                     ((uint32_t)0x00000000)
#define FMC_WriteOperation_Enable                      ((uint32_t)0x00001000)

#define IS_FMC_WRITE_OPERATION(OPERATION) (((OPERATION) == FMC_WriteOperation_Disable) || \
                                            ((OPERATION) == FMC_WriteOperation_Enable))                         
/**
  * @}
  */

/** @defgroup FMC_Wait_Signal 
  * @{
  */
#define FMC_WaitSignal_Disable                  ((uint32_t)0x00000000)
#define FMC_WaitSignal_Enable                   ((uint32_t)0x00002000) 

#define IS_FMC_WAITE_SIGNAL(SIGNAL) (((SIGNAL) == FMC_WaitSignal_Disable) || \
                                      ((SIGNAL) == FMC_WaitSignal_Enable))
/**
  * @}
  */

/** @defgroup FMC_Extended_Mode 
  * @{
  */
#define FMC_ExtendedMode_Disable                ((uint32_t)0x00000000)
#define FMC_ExtendedMode_Enable                 ((uint32_t)0x00004000)

#define IS_FMC_EXTENDED_MODE(MODE) (((MODE) == FMC_ExtendedMode_Disable) || \
                                     ((MODE) == FMC_ExtendedMode_Enable)) 
/**
  * @}
  */

/** @defgroup FMC_Write_Burst 
  * @{
  */

#define FMC_WriteBurst_Disable                  ((uint32_t)0x00000000)
#define FMC_WriteBurst_Enable                   ((uint32_t)0x00080000) 

#define IS_FMC_WRITE_BURST(BURST) (((BURST) == FMC_WriteBurst_Disable) || \
                                    ((BURST) == FMC_WriteBurst_Enable))
/**
  * @}
  */
  
/** @defgroup FMC_Continous_Clock 
  * @{
  */

#define FMC_CClock_SyncOnly                     ((uint32_t)0x00000000)
#define FMC_CClock_SyncAsync                    ((uint32_t)0x00100000) 

#define IS_FMC_CONTINOUS_CLOCK(CCLOCK) (((CCLOCK) == FMC_CClock_SyncOnly) || \
                                        ((CCLOCK) == FMC_CClock_SyncAsync))
/**
  * @}
  */  

/** @defgroup FMC_Address_Setup_Time 
  * @{
  */
#define IS_FMC_ADDRESS_SETUP_TIME(TIME) ((TIME) <= 15)
/**
  * @}
  */

/** @defgroup FMC_Address_Hold_Time 
  * @{
  */
#define IS_FMC_ADDRESS_HOLD_TIME(TIME) (((TIME) > 0) && ((TIME) <= 15))
/**
  * @}
  */

/** @defgroup FMC_Data_Setup_Time 
  * @{
  */
#define IS_FMC_DATASETUP_TIME(TIME) (((TIME) > 0) && ((TIME) <= 255))
/**
  * @}
  */

/** @defgroup FMC_Bus_Turn_around_Duration 
  * @{
  */
#define IS_FMC_TURNAROUND_TIME(TIME) ((TIME) <= 15)
/**
  * @}
  */

/** @defgroup FMC_CLK_Division 
  * @{
  */
#define IS_FMC_CLK_DIV(DIV) (((DIV) > 1) && ((DIV) <= 16))
/**
  * @}
  */

/** @defgroup FMC_Data_Latency 
  * @{
  */
#define IS_FMC_DATA_LATENCY(LATENCY) (((LATENCY) > 1) && ((LATENCY) <= 17))
/**
  * @}
  */

/** @defgroup FMC_Access_Mode 
  * @{
  */
#define FMC_AccessMode_A                        ((uint32_t)0x00000000)
#define FMC_AccessMode_B                        ((uint32_t)0x10000000) 
#define FMC_AccessMode_C                        ((uint32_t)0x20000000)
#define FMC_AccessMode_D                        ((uint32_t)0x30000000)

#define IS_FMC_ACCESS_MODE(MODE) (((MODE) == FMC_AccessMode_A)  || \
                                   ((MODE) == FMC_AccessMode_B) || \
                                   ((MODE) == FMC_AccessMode_C) || \
                                   ((MODE) == FMC_AccessMode_D))
/**
  * @}
  */

/**
  * @}
  */
  
/** @defgroup FMC_NAND_PCCARD_Controller 
  * @{
  */

/** @defgroup FMC_Wait_feature 
  * @{
  */
#define FMC_Waitfeature_Disable                 ((uint32_t)0x00000000)
#define FMC_Waitfeature_Enable                  ((uint32_t)0x00000002)

#define IS_FMC_WAIT_FEATURE(FEATURE) (((FEATURE) == FMC_Waitfeature_Disable) || \
                                       ((FEATURE) == FMC_Waitfeature_Enable))
/**
  * @}
  */

/** @defgroup FMC_NAND_Data_Width 
  * @{
  */
#define FMC_NAND_MemoryDataWidth_8b             ((uint32_t)0x00000000)
#define FMC_NAND_MemoryDataWidth_16b            ((uint32_t)0x00000010)

#define IS_FMC_NAND_MEMORY_WIDTH(WIDTH) (((WIDTH) == FMC_NAND_MemoryDataWidth_8b) || \
                                         ((WIDTH) == FMC_NAND_MemoryDataWidth_16b))
/**
  * @}
  */

/** @defgroup FMC_ECC 
  * @{
  */
#define FMC_ECC_Disable                         ((uint32_t)0x00000000)
#define FMC_ECC_Enable                          ((uint32_t)0x00000040)

#define IS_FMC_ECC_STATE(STATE) (((STATE) == FMC_ECC_Disable) || \
                                  ((STATE) == FMC_ECC_Enable))
/**
  * @}
  */

/** @defgroup FMC_ECC_Page_Size 
  * @{
  */
#define FMC_ECCPageSize_256Bytes                ((uint32_t)0x00000000)
#define FMC_ECCPageSize_512Bytes                ((uint32_t)0x00020000)
#define FMC_ECCPageSize_1024Bytes               ((uint32_t)0x00040000)
#define FMC_ECCPageSize_2048Bytes               ((uint32_t)0x00060000)
#define FMC_ECCPageSize_4096Bytes               ((uint32_t)0x00080000)
#define FMC_ECCPageSize_8192Bytes               ((uint32_t)0x000A0000)

#define IS_FMC_ECCPAGE_SIZE(SIZE) (((SIZE) == FMC_ECCPageSize_256Bytes)   || \
                                    ((SIZE) == FMC_ECCPageSize_512Bytes)  || \
                                    ((SIZE) == FMC_ECCPageSize_1024Bytes) || \
                                    ((SIZE) == FMC_ECCPageSize_2048Bytes) || \
                                    ((SIZE) == FMC_ECCPageSize_4096Bytes) || \
                                    ((SIZE) == FMC_ECCPageSize_8192Bytes))
/**
  * @}
  */

/** @defgroup FMC_TCLR_Setup_Time 
  * @{
  */
#define IS_FMC_TCLR_TIME(TIME) ((TIME) <= 255)
/**
  * @}
  */

/** @defgroup FMC_TAR_Setup_Time 
  * @{
  */
#define IS_FMC_TAR_TIME(TIME) ((TIME) <= 255)
/**
  * @}
  */

/** @defgroup FMC_Setup_Time 
  * @{
  */
#define IS_FMC_SETUP_TIME(TIME) ((TIME) <= 255)
/**
  * @}
  */

/** @defgroup FMC_Wait_Setup_Time 
  * @{
  */
#define IS_FMC_WAIT_TIME(TIME) ((TIME) <= 255)
/**
  * @}
  */

/** @defgroup FMC_Hold_Setup_Time 
  * @{
  */
#define IS_FMC_HOLD_TIME(TIME) ((TIME) <= 255)
/**
  * @}
  */

/** @defgroup FMC_HiZ_Setup_Time 
  * @{
  */
#define IS_FMC_HIZ_TIME(TIME) ((TIME) <= 255)
/**
  * @}
  */ 

/**
  * @}
  */  

/** @defgroup FMC_Interrupt_sources 
  * @{
  */
#define FMC_IT_RisingEdge                       ((uint32_t)0x00000008)
#define FMC_IT_Level                            ((uint32_t)0x00000010)
#define FMC_IT_FallingEdge                      ((uint32_t)0x00000020)

#define IS_FMC_IT(IT) ((((IT) & (uint32_t)0xFFFFBFC7) == 0x00000000) && ((IT) != 0x00000000))
#define IS_FMC_GET_IT(IT) (((IT) == FMC_IT_RisingEdge)  || \
                           ((IT) == FMC_IT_Level)       || \
                           ((IT) == FMC_IT_FallingEdge)) 
                           
#define IS_FMC_IT_BANK(BANK) (((BANK) == FMC_Bank2_NAND)   || \
                              ((BANK) == FMC_Bank3_NAND)   || \
                              ((BANK) == FMC_Bank4_PCCARD))                           
/**
  * @}
  */

/** @defgroup FMC_Flags 
  * @{
  */
#define FMC_FLAG_RisingEdge                     ((uint32_t)0x00000001)
#define FMC_FLAG_Level                          ((uint32_t)0x00000002)
#define FMC_FLAG_FallingEdge                    ((uint32_t)0x00000004)
#define FMC_FLAG_FEMPT                          ((uint32_t)0x00000040)

#define IS_FMC_GET_FLAG(FLAG) (((FLAG) == FMC_FLAG_RisingEdge)       || \
                               ((FLAG) == FMC_FLAG_Level)            || \
                               ((FLAG) == FMC_FLAG_FallingEdge)      || \
                               ((FLAG) == FMC_FLAG_FEMPT))

#define IS_FMC_GETFLAG_BANK(BANK) (((BANK) == FMC_Bank2_NAND)    || \
                                   ((BANK) == FMC_Bank3_NAND)    || \
                                   ((BANK) == FMC_Bank4_PCCARD))
                                   
#define IS_FMC_CLEAR_FLAG(FLAG) ((((FLAG) & (uint32_t)0xFFFFFFF8) == 0x00000000) && ((FLAG) != 0x00000000))


/**
  * @}
  */

/**
  * @}
  */


/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/ 

/* NOR/SRAM Controller functions **********************************************/
void FMC_NORSRAMDeInit(uint32_t FMC_Bank);
void FMC_NORSRAMInit(FMC_NORSRAMInitTypeDef* FMC_NORSRAMInitStruct);
void FMC_NORSRAMStructInit(FMC_NORSRAMInitTypeDef* FMC_NORSRAMInitStruct);
void FMC_NORSRAMCmd(uint32_t FMC_Bank, FunctionalState NewState);

/* NAND Controller functions **************************************************/
void     FMC_NANDDeInit(uint32_t FMC_Bank);
void     FMC_NANDInit(FMC_NANDInitTypeDef* FMC_NANDInitStruct);
void     FMC_NANDStructInit(FMC_NANDInitTypeDef* FMC_NANDInitStruct);
void     FMC_NANDCmd(uint32_t FMC_Bank, FunctionalState NewState);
void     FMC_NANDECCCmd(uint32_t FMC_Bank, FunctionalState NewState);
uint32_t FMC_GetECC(uint32_t FMC_Bank);

/* PCCARD Controller functions ************************************************/
void FMC_PCCARDDeInit(void);
void FMC_PCCARDInit(FMC_PCCARDInitTypeDef* FMC_PCCARDInitStruct);
void FMC_PCCARDStructInit(FMC_PCCARDInitTypeDef* FMC_PCCARDInitStruct);
void FMC_PCCARDCmd(FunctionalState NewState);

/* Interrupts and flags management functions **********************************/
void       FMC_ITConfig(uint32_t FMC_Bank, uint32_t FMC_IT, FunctionalState NewState);
FlagStatus FMC_GetFlagStatus(uint32_t FMC_Bank, uint32_t FMC_FLAG);
void       FMC_ClearFlag(uint32_t FMC_Bank, uint32_t FMC_FLAG);
ITStatus   FMC_GetITStatus(uint32_t FMC_Bank, uint32_t FMC_IT);
void       FMC_ClearITPendingBit(uint32_t FMC_Bank, uint32_t FMC_IT);

#ifdef __cplusplus
}
#endif

#endif /*__STM32F30x_FMC_H */
/**
  * @}
  */

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
