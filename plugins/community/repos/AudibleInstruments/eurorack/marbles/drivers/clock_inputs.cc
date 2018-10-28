// Copyright 2015 Olivier Gillet.
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
// Driver for the two clock inputs and their normalization probe.

#include "marbles/drivers/clock_inputs.h"

#include <stm32f4xx_conf.h>

namespace marbles {

using namespace std;
using namespace stmlib;

struct ClockInputDefinition {
  GPIO_TypeDef* gpio;
  uint16_t pin;
};

const ClockInputDefinition clock_input_definition[] = {
  { GPIOC, GPIO_Pin_9 },  // CLOCK_INPUT_T,
  { GPIOA, GPIO_Pin_8 },  // CLOCK_INPUT_X,
};

void ClockInputs::Init() {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  // Initialize probe.
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Pin = GPIO_Pin_8;
  GPIO_Init(GPIOC, &gpio_init);
  
  // Initialize inputs.
  gpio_init.GPIO_Mode = GPIO_Mode_IN;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

  for (int i = 0; i < CLOCK_INPUT_LAST; ++i) {
    gpio_init.GPIO_Pin = clock_input_definition[i].pin;
    GPIO_Init(clock_input_definition[i].gpio, &gpio_init);
    previous_flags_[i] = 0;
    normalization_mismatch_count_[i] = 0;
    normalized_[i] = false;
  }

  normalization_probe_state_ = 0;
  normalization_decision_count_ = 0;
}

void ClockInputs::ReadNormalization(IOBuffer::Block* block) {
  ++normalization_decision_count_;
  if (normalization_decision_count_ >= kProbeSequenceDuration) {
    normalization_decision_count_ = 0;
    for (int i = 0; i < CLOCK_INPUT_LAST; ++i) {
      normalized_[i] = \
          normalization_mismatch_count_[i] < kProbeSequenceDuration / 8;
      normalization_mismatch_count_[i] = 0;
    }
  }
  
  int expected_value = normalization_probe_state_ >> 31;
  for (int i = 0; i < CLOCK_INPUT_LAST; ++i) {
    int read_value = previous_flags_[i] & GATE_FLAG_HIGH;
    normalization_mismatch_count_[i] += read_value ^ expected_value;
    block->input_patched[i] = !normalized_[i];
  }

  normalization_probe_state_ = 1103515245 * normalization_probe_state_ + 12345;
  if (normalization_probe_state_ >> 31) {
    GPIOC->BSRRL = GPIO_Pin_8;
  } else {
    GPIOC->BSRRH = GPIO_Pin_8;
  }
}

void ClockInputs::Read(const IOBuffer::Slice& slice, size_t size) {
  for (int i = 0; i < CLOCK_INPUT_LAST; ++i) {
    previous_flags_[i] = ExtractGateFlags(
        previous_flags_[i],
        !(clock_input_definition[i].gpio->IDR & clock_input_definition[i].pin));
    slice.block->input[i][slice.frame_index] = previous_flags_[i];
  }
  
  // Extend gate input data to the next samples.
  for (size_t j = 1; j < size; ++j) {
    for (int i = 0; i < CLOCK_INPUT_LAST; ++i) {
      slice.block->input[i][slice.frame_index + j] = \
          previous_flags_[i] & GATE_FLAG_HIGH;
    }
  }
}

}  // namespace marbles
