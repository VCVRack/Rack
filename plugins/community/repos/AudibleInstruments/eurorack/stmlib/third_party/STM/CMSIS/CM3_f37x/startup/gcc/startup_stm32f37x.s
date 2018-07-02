/**
  ******************************************************************************
  * @file      startup_stm32f37x.s
  * @author    MCD Application Team
  * @version   V1.0.0
  * @date      20-September-2012
  * @brief     STM32F37x Devices vector table for RIDE7 toolchain. 
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Configure the clock system
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M4 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
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
    
  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.global  g_pfnVectors
.global  Default_Handler

/* start address for the initialization values of the .data section. 
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */  
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss
/* stack used for SystemInit_ExtMemCtl; always internal RAM used */

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/

    .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:  

/* Copy the data segment initializers from flash to SRAM */  
  movs  r1, #0
  b  LoopCopyDataInit

CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
    
LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit
  ldr  r2, =_sbss
  b  LoopFillZerobss
/* Zero fill the bss segment. */  
FillZerobss:
  movs  r3, #0
  str  r3, [r2], #4
    
LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss

/* Call the clock system intitialization function.*/
  bl  SystemInit   
/* Call the application's entry point.*/
  bl  main
  bx  lr    
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None     
 * @retval None       
*/
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex-M4. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors
    
    
g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler
  
  /* External Interrupts */
  .word     WWDG_IRQHandler                   /* Window WatchDog              */                                        
  .word     PVD_IRQHandler                    /* PVD through EXTI Line detection */                        
  .word     TAMPER_STAMP_IRQHandler           /* Tamper and TimeStamps through the EXTI line */            
  .word     RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line */                      
  .word     FLASH_IRQHandler                  /* FLASH                        */                                          
  .word     RCC_IRQHandler                    /* RCC                          */                                            
  .word     EXTI0_IRQHandler                  /* EXTI Line0                   */                        
  .word     EXTI1_IRQHandler                  /* EXTI Line1                   */                          
  .word     EXTI2_TS_IRQHandler               /* EXTI Line2                   */                          
  .word     EXTI3_IRQHandler                  /* EXTI Line3                   */                          
  .word     EXTI4_IRQHandler                  /* EXTI Line4                   */                          
  .word     DMA1_Channel1_IRQHandler          /* DMA1 Channel 1               */                  
  .word     DMA1_Channel2_IRQHandler          /* DMA1 Channel 2               */                  
  .word     DMA1_Channel3_IRQHandler          /* DMA1 Channel 3               */                 
  .word     DMA1_Channel4_IRQHandler          /* DMA1 Channel 4               */                  
  .word     DMA1_Channel5_IRQHandler          /* DMA1 Channel 5               */                  
  .word     DMA1_Channel6_IRQHandler          /* DMA1 Channel 6               */                  
  .word     DMA1_Channel7_IRQHandler          /* DMA1 Channel 7               */                 
  .word     ADC1_IRQHandler                    /* ADC1                         */                   
  .word     CAN1_TX_IRQHandler                /* CAN1 TX                      */                         
  .word     CAN1_RX0_IRQHandler               /* CAN1 RX0                     */                          
  .word     CAN1_RX1_IRQHandler               /* CAN1 RX1                     */                          
  .word     CAN1_SCE_IRQHandler               /* CAN1 SCE                     */                          
  .word     EXTI9_5_IRQHandler                /* External Line[9:5]s          */                          
  .word     TIM15_IRQHandler                  /* TIM15                        */         
  .word     TIM16_IRQHandler                  /* TIM16                        */         
  .word     TIM17_IRQHandler                  /* TIM17                        */ 
  .word     TIM18_DAC2_IRQHandler             /* TIM18 and DAC2               */                          
  .word     TIM2_IRQHandler                   /* TIM2                         */                       
  .word     TIM3_IRQHandler                   /* TIM3                         */                   
  .word     TIM4_IRQHandler                   /* TIM4                         */                   
  .word     I2C1_EV_IRQHandler                /* I2C1 Event                   */                          
  .word     I2C1_ER_IRQHandler                /* I2C1 Error                   */                          
  .word     I2C2_EV_IRQHandler                /* I2C2 Event                   */                          
  .word     I2C2_ER_IRQHandler                /* I2C2 Error                   */                            
  .word     SPI1_IRQHandler                   /* SPI1                         */                   
  .word     SPI2_IRQHandler                   /* SPI2                         */                   
  .word     USART1_IRQHandler                 /* USART1                       */                   
  .word     USART2_IRQHandler                 /* USART2                       */                   
  .word     USART3_IRQHandler                 /* USART3                       */                   
  .word     EXTI15_10_IRQHandler              /* External Line[15:10]s        */                          
  .word     RTC_Alarm_IRQHandler              /* RTC_Alarm_IRQHandler         */
  .word     CEC_IRQHandler                    /* CEC                          */
  .word     TIM12_IRQHandler                  /* TIM12                        */
  .word     TIM13_IRQHandler                  /* TIM13                        */
  .word     TIM14_IRQHandler                  /* TIM14                        */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     TIM5_IRQHandler                   /* TIM5                         */      
  .word     SPI3_IRQHandler                   /* SPI3                         */           
  .word     0                                 /* Reserved                      */
  .word     0                                 /* Reserved                      */                 
  .word     TIM6_DAC1_IRQHandler             /* TIM6 and DAC1 Channel1 & channel2 */                   
  .word     TIM7_IRQHandler                   /* TIM7                         */
  .word     DMA2_Channel1_IRQHandler          /* DMA2 Channel 1               */                   
  .word     DMA2_Channel2_IRQHandler          /* DMA2 Channel 2               */                  
  .word     DMA2_Channel3_IRQHandler          /* DMA2 Channel 3               */                   
  .word     DMA2_Channel4_IRQHandler          /* DMA2 Channel 4               */                   
  .word     DMA2_Channel5_IRQHandler          /* DMA2 Channel 5               */                   
  .word     SDADC1_IRQHandler                 /* SDADC1                       */                   
  .word     SDADC2_IRQHandler                 /* SDADC2                       */                     
  .word     SDADC3_IRQHandler                 /* SDADC3                       */                          
  .word     COMP_IRQHandler                   /* COMP                         */                          
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */                               
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */                         
  .word     0                                 /* Reserved                     */                        
  .word     USB_HP_IRQHandler                 /* USB High Priority            */                   
  .word     USB_LP_IRQHandler                 /* USB Low Priority             */             
  .word     USBWakeUp_IRQHandler              /* USB Wakeup                   */                         
  .word     0                                 /* Resrved                      */                      
  .word     TIM19_IRQHandler                  /*TIM19                         */                   
  .word     0                                 /* Resrved                      */                 
  .word     FPU_IRQHandler                    /* FPU                          */
                         
                         
/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler. 
* As they are weak aliases, any function with the same name will override 
* this definition.
* 
*******************************************************************************/
   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler
  
   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler
  
   .weak      MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler
  
   .weak      BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler

   .weak      UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler              
  
   .weak      WWDG_IRQHandler                   
   .thumb_set WWDG_IRQHandler,Default_Handler      
                  
   .weak      PVD_IRQHandler      
   .thumb_set PVD_IRQHandler,Default_Handler
               
   .weak      TAMPER_STAMP_IRQHandler            
   .thumb_set TAMPER_STAMP_IRQHandler,Default_Handler
            
   .weak      RTC_WKUP_IRQHandler                  
   .thumb_set RTC_WKUP_IRQHandler,Default_Handler
            
   .weak      FLASH_IRQHandler         
   .thumb_set FLASH_IRQHandler,Default_Handler
                  
   .weak      RCC_IRQHandler      
   .thumb_set RCC_IRQHandler,Default_Handler
                  
   .weak      EXTI0_IRQHandler         
   .thumb_set EXTI0_IRQHandler,Default_Handler
                  
   .weak      EXTI1_IRQHandler         
   .thumb_set EXTI1_IRQHandler,Default_Handler
                     
   .weak      EXTI2_TS_IRQHandler        
   .thumb_set EXTI2_TS_IRQHandler,Default_Handler 
                 
   .weak      EXTI3_IRQHandler         
   .thumb_set EXTI3_IRQHandler,Default_Handler
                        
   .weak      EXTI4_IRQHandler         
   .thumb_set EXTI4_IRQHandler,Default_Handler
                  
   .weak      DMA1_Channel1_IRQHandler               
   .thumb_set DMA1_Channel1_IRQHandler,Default_Handler
         
   .weak      DMA1_Channel2_IRQHandler            
   .thumb_set DMA1_Channel2_IRQHandler,Default_Handler
                  
   .weak      DMA1_Channel3_IRQHandler               
   .thumb_set DMA1_Channel3_IRQHandler,Default_Handler
                  
   .weak      DMA1_Channel4_IRQHandler              
   .thumb_set DMA1_Channel4_IRQHandler,Default_Handler 
                 
   .weak      DMA1_Channel5_IRQHandler              
   .thumb_set DMA1_Channel5_IRQHandler,Default_Handler
                  
   .weak      DMA1_Channel6_IRQHandler               
   .thumb_set DMA1_Channel6_IRQHandler,Default_Handler
                  
   .weak      DMA1_Channel7_IRQHandler              
   .thumb_set DMA1_Channel7_IRQHandler,Default_Handler
                  
   .weak      ADC1_IRQHandler      
   .thumb_set ADC1_IRQHandler,Default_Handler
               
   .weak      CAN1_TX_IRQHandler   
   .thumb_set CAN1_TX_IRQHandler,Default_Handler
            
   .weak      CAN1_RX0_IRQHandler                  
   .thumb_set CAN1_RX0_IRQHandler,Default_Handler
                           
   .weak      CAN1_RX1_IRQHandler                  
   .thumb_set CAN1_RX1_IRQHandler,Default_Handler
            
   .weak      CAN1_SCE_IRQHandler                  
   .thumb_set CAN1_SCE_IRQHandler,Default_Handler
            
   .weak      EXTI9_5_IRQHandler   
   .thumb_set EXTI9_5_IRQHandler,Default_Handler
            
   .weak      TIM15_IRQHandler            
   .thumb_set TIM15_IRQHandler,Default_Handler
            
   .weak      TIM16_IRQHandler           
   .thumb_set TIM16_IRQHandler,Default_Handler
      
   .weak      TIM17_IRQHandler      
   .thumb_set TIM17_IRQHandler,Default_Handler
      
   .weak      TIM18_DAC2_IRQHandler   
   .thumb_set TIM18_DAC2_IRQHandler,Default_Handler
                  
   .weak      TIM2_IRQHandler            
   .thumb_set TIM2_IRQHandler,Default_Handler
                  
   .weak      TIM3_IRQHandler            
   .thumb_set TIM3_IRQHandler,Default_Handler
                  
   .weak      TIM4_IRQHandler            
   .thumb_set TIM4_IRQHandler,Default_Handler
                  
   .weak      I2C1_EV_IRQHandler   
   .thumb_set I2C1_EV_IRQHandler,Default_Handler
                     
   .weak      I2C1_ER_IRQHandler   
   .thumb_set I2C1_ER_IRQHandler,Default_Handler
                     
   .weak      I2C2_EV_IRQHandler   
   .thumb_set I2C2_EV_IRQHandler,Default_Handler
                  
   .weak      I2C2_ER_IRQHandler   
   .thumb_set I2C2_ER_IRQHandler,Default_Handler
                           
   .weak      SPI1_IRQHandler            
   .thumb_set SPI1_IRQHandler,Default_Handler
                        
   .weak      SPI2_IRQHandler            
   .thumb_set SPI2_IRQHandler,Default_Handler
                  
   .weak      USART1_IRQHandler      
   .thumb_set USART1_IRQHandler,Default_Handler
                     
   .weak      USART2_IRQHandler      
   .thumb_set USART2_IRQHandler,Default_Handler
                     
   .weak      USART3_IRQHandler      
   .thumb_set USART3_IRQHandler,Default_Handler
                  
   .weak      EXTI15_10_IRQHandler               
   .thumb_set EXTI15_10_IRQHandler,Default_Handler
               
   .weak      RTC_Alarm_IRQHandler               
   .thumb_set RTC_Alarm_IRQHandler,Default_Handler
            
   .weak      CEC_IRQHandler       
   .thumb_set CEC_IRQHandler,Default_Handler
            
   .weak      TIM12_IRQHandler         
   .thumb_set TIM12_IRQHandler,Default_Handler
         
   .weak      TIM13_IRQHandler            
   .thumb_set TIM13_IRQHandler,Default_Handler
         
   .weak      TIM14_IRQHandler      
   .thumb_set TIM14_IRQHandler,Default_Handler
      
   .weak      TIM5_IRQHandler   
   .thumb_set TIM5_IRQHandler,Default_Handler
                  
   .weak      SPI3_IRQHandler               
   .thumb_set SPI3_IRQHandler,Default_Handler
                     
   .weak      TIM6_DAC1_IRQHandler           
   .thumb_set TIM6_DAC1_IRQHandler,Default_Handler
                     
   .weak      TIM7_IRQHandler           
   .thumb_set TIM7_IRQHandler,Default_Handler
                     
   .weak      DMA2_Channel1_IRQHandler            
   .thumb_set DMA2_Channel1_IRQHandler,Default_Handler
                     
   .weak      DMA2_Channel2_IRQHandler            
   .thumb_set DMA2_Channel2_IRQHandler,Default_Handler
                     
   .weak      DMA2_Channel3_IRQHandler         
   .thumb_set DMA2_Channel3_IRQHandler,Default_Handler
                  
   .weak      DMA2_Channel4_IRQHandler         
   .thumb_set DMA2_Channel4_IRQHandler,Default_Handler
                  
   .weak      DMA2_Channel5_IRQHandler                  
   .thumb_set DMA2_Channel5_IRQHandler,Default_Handler
               
   .weak      SDADC1_IRQHandler            
   .thumb_set SDADC1_IRQHandler,Default_Handler
         
   .weak      SDADC2_IRQHandler               
   .thumb_set SDADC2_IRQHandler,Default_Handler
               
   .weak      SDADC3_IRQHandler              
   .thumb_set SDADC3_IRQHandler,Default_Handler
                  
   .weak      COMP_IRQHandler               
   .thumb_set COMP_IRQHandler,Default_Handler
            
   .weak      USB_HP_IRQHandler               
   .thumb_set USB_HP_IRQHandler,Default_Handler
            
   .weak      USB_LP_IRQHandler               
   .thumb_set USB_LP_IRQHandler,Default_Handler
            
   .weak      USBWakeUp_IRQHandler      
   .thumb_set USBWakeUp_IRQHandler,Default_Handler
                  
   .weak      TIM19_IRQHandler                  
   .thumb_set TIM19_IRQHandler,Default_Handler
            
   .weak      FPU_IRQHandler   
   .thumb_set FPU_IRQHandler,Default_Handler
                           
   
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
