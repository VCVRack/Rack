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
// Helper functions for flash programming.

#ifndef STMLIB_SYSTEM_FLASH_PROGRAMMING_H_
#define STMLIB_SYSTEM_FLASH_PROGRAMMING_H_

#if defined STM32F37X

  #define PAGE_SIZE (uint16_t)0x800

#else

  #if defined (STM32F10X_LD) || defined (STM32F10X_MD)
    #define PAGE_SIZE  (uint16_t)0x400  /* Page size = 1KByte */
  #elif defined (STM32F10X_HD) || defined (STM32F10X_CL)
    #define PAGE_SIZE  (uint16_t)0x800  /* Page size = 2KByte */
  #endif

#endif

#endif  // STMLIB_SYSTEM_FLASH_PROGRAMMING_H_
