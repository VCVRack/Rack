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
// Driver for rotary encoder.

#include "yarns/drivers/encoder.h"

namespace yarns {

void Encoder::Init() {
  GPIO_InitTypeDef gpio_init;

  gpio_init.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOC, &gpio_init);
  
  switch_state_ = 0xff;
  quadrature_decoding_state_[0] = quadrature_decoding_state_[1] = 0xff;
}

void Encoder::Debounce() {
  switch_state_ = (switch_state_ << 1) | \
      GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15);
  quadrature_decoding_state_[0] = (quadrature_decoding_state_[0] << 1) | \
      GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
  quadrature_decoding_state_[1] = (quadrature_decoding_state_[1] << 1) | \
      GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14);
}

}  // namespace yarns
