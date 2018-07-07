// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Function for jumping from bootloader code to application code.

#include "stmlib/system/bootloader_utils.h"

#if defined STM32F4XX
  #include <stm32f4xx_conf.h>
#elif defined STM32F37X
  #include <stm32f37x_conf.h>
#elif defined STM32F30X
  #include <stm32f30x_conf.h>
#elif defined STM32F10X
  #include <stm32f10x_conf.h>
#endif  // STM32F37X

namespace stmlib {

#define u32 uint32_t
#define vu32 volatile uint32_t

#define SET_REG(addr,val) do { *(vu32*)(addr)=val; } while(0)
#define GET_REG(addr)     (*(vu32*)(addr))

#define RCC_CR      RCC
#define RCC_CFGR    (RCC + 0x04)
#define RCC_CIR     (RCC + 0x08)
#define RCC_AHBENR  (RCC + 0x14)
#define RCC_APB2ENR (RCC + 0x18)
#define RCC_APB1ENR (RCC + 0x1C)

#define SCS      0xE000E000
#define STK      (SCS+0x10)
#define STK_CTRL (STK+0x00)
#define RCC_CR      RCC

typedef struct {
    vu32 ISER[2];
    u32  RESERVED0[30];
    vu32 ICER[2];
    u32  RSERVED1[30];
    vu32 ISPR[2];
    u32  RESERVED2[30];
    vu32 ICPR[2];
    u32  RESERVED3[30];
    vu32 IABR[2];
    u32  RESERVED4[62];
    vu32 IPR[15];
} NVIC_TypeDef;

void Uninitialize() {
  // Stop NVIC.
  NVIC_TypeDef *rNVIC = (NVIC_TypeDef *) NVIC_BASE;
  rNVIC->ICER[0] = 0xFFFFFFFF;
  rNVIC->ICER[1] = 0xFFFFFFFF;
  rNVIC->ICPR[0] = 0xFFFFFFFF;
  rNVIC->ICPR[1] = 0xFFFFFFFF;
  SET_REG(STK_CTRL, 0x04);

  // System reset.
  SET_REG(RCC_CR, GET_REG(RCC_CR)     | 0x00000001);
  SET_REG(RCC_CFGR, GET_REG(RCC_CFGR) & 0xF8FF0000);
  SET_REG(RCC_CR, GET_REG(RCC_CR)     & 0xFEF6FFFF);
  SET_REG(RCC_CR, GET_REG(RCC_CR)     & 0xFFFBFFFF);
  SET_REG(RCC_CFGR, GET_REG(RCC_CFGR) & 0xFF80FFFF);
  SET_REG(RCC_CIR, 0x00000000);
}

typedef void (*EntryPoint)(void);

void JumpTo(uint32_t address) {
  uint32_t application_address = *(__IO uint32_t*)(address + 4);
  EntryPoint application = (EntryPoint)(application_address);
  __set_MSP(*(__IO uint32_t*)address);
  application();
}

}  // namespace stmlib
