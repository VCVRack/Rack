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
// Drivers for the column of LEDs.

#include "plaits/drivers/leds.h"

#include <algorithm>

#include <stm32f37x_conf.h>

namespace plaits {

using namespace std;

const uint16_t kPinEnable = GPIO_Pin_7;

void Leds::Init() {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  
  GPIO_InitTypeDef gpio_init;
  
  // SS
  gpio_init.GPIO_Mode = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Pin = kPinEnable;
  GPIO_Init(GPIOF, &gpio_init);
  
  // MOSI
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOF, &gpio_init);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource6, GPIO_AF_5);

  // SCK
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpio_init.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOA, &gpio_init);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_6);
  
  // Initialize SPI
  SPI_InitTypeDef spi_init;
  spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spi_init.SPI_Mode = SPI_Mode_Master;
  spi_init.SPI_DataSize = SPI_DataSize_16b;
  spi_init.SPI_CPOL = SPI_CPOL_Low;
  spi_init.SPI_CPHA = SPI_CPHA_1Edge;
  spi_init.SPI_NSS = SPI_NSS_Soft;
  spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
  spi_init.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &spi_init);
  SPI_Cmd(SPI1, ENABLE);
  Clear();
}

void Leds::Clear() {
  fill(&colors_[0], &colors_[kNumLEDs], LED_COLOR_OFF);
}

/* static */
const int Leds::led_map_[8] = {
  0, 1, 2, 3, 7, 6, 5, 4
};

void Leds::Write() {
  uint16_t leds_data = 0;
  for (int i = 0; i < kNumLEDs; ++i) {
    int j = led_map_[i];
    leds_data <<= 2;
    leds_data |= (colors_[j] & LED_COLOR_RED) ? 1 : 0;
    leds_data |= (colors_[j] & LED_COLOR_GREEN) ? 2 : 0;
  }
  GPIOF->BSRR = kPinEnable;
  __asm__("nop");
  GPIOF->BRR = kPinEnable;
  __asm__("nop");
  SPI1->DR = leds_data;
}

}  // namespace plaits
