// Copyright 2016 Olivier Gillet.
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
// UART driver for conversing with the factory testing program.

#ifndef PLAITS_DRIVERS_DEBUG_PORT_H_
#define PLAITS_DRIVERS_DEBUG_PORT_H_

#include "stmlib/stmlib.h"

#include <stm32f37x_conf.h>

namespace plaits {

class DebugPort {
 public:
  DebugPort() { }
  ~DebugPort() { }
  
  void Init();
  
  bool writable() {
    return USART3->ISR & USART_FLAG_TXE;
  }
  
  bool readable() {
    return USART3->ISR & USART_FLAG_RXNE;
  }
  
  void Write(uint8_t byte) {
    USART3->TDR = byte;
  }
  
  uint8_t Read() {
    return USART3->RDR;
  }
  
 private:
  DISALLOW_COPY_AND_ASSIGN(DebugPort);
};

}  // namespace plaits

#endif  // PLAITS_DRIVERS_DEBUG_PORT_H_
