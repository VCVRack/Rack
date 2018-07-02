// Copyright 2013 Olivier Gillet.
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
// Driver for the Gate/Trigger outputs.

#include "yarns/drivers/gate_output.h"

#include <stm32f10x_conf.h>

namespace yarns {

void GateOutput::Init() {
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &gpio_init);
}

void GateOutput::Write(const bool* gate) {
  GPIO_WriteBit(GPIOB, GPIO_Pin_10, static_cast<BitAction>(gate[0]));
  GPIO_WriteBit(GPIOB, GPIO_Pin_11, static_cast<BitAction>(gate[1]));
  GPIO_WriteBit(GPIOB, GPIO_Pin_0, static_cast<BitAction>(gate[2]));
  GPIO_WriteBit(GPIOB, GPIO_Pin_1, static_cast<BitAction>(gate[3]));
}

}  // namespace yarns
