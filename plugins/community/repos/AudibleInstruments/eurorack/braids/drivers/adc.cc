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
// Driver for ADC scanning.

#include "braids/drivers/adc.h"

#include <string.h>

namespace braids {
  
const uint16_t kPinSS = GPIO_Pin_0;

void Adc::Init(bool double_speed) {
  // Initialize SS pin.
  GPIO_InitTypeDef gpio_init;
  gpio_init.GPIO_Pin = kPinSS;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &gpio_init);
  
  // Initialize MOSI and SCK pins.
  gpio_init.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &gpio_init);
  
  // Initialize MISO pin.
  gpio_init.GPIO_Pin = GPIO_Pin_6;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &gpio_init);
  
  // Initialize SPI
  SPI_InitTypeDef spi_init;
  spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spi_init.SPI_Mode = SPI_Mode_Master;
  spi_init.SPI_DataSize = SPI_DataSize_8b;
  spi_init.SPI_CPOL = SPI_CPOL_Low;
  spi_init.SPI_CPHA = SPI_CPHA_1Edge;
  spi_init.SPI_NSS = SPI_NSS_Soft;
  spi_init.SPI_BaudRatePrescaler = double_speed ? SPI_BaudRatePrescaler_32 : SPI_BaudRatePrescaler_64;
  spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
  spi_init.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &spi_init);
  GPIO_SetBits(GPIOB, kPinSS);
  SPI_Cmd(SPI1, ENABLE);
  
  rx_word_ = 0;
  active_channel_ = 0;
  acquisition_stage_ = 0;
  memset(channels_, 0, sizeof(channels_));
}

bool Adc::PipelinedRead(uint8_t channel) {
  switch (acquisition_stage_) {
    case 0:
      rx_word_ |= SPI_I2S_ReceiveData(SPI1);
      channels_[active_channel_] = rx_word_;
      GPIO_SetBits(GPIOB, kPinSS);
      GPIO_ResetBits(GPIOB, kPinSS);
      SPI_I2S_SendData(SPI1, 0x04 | 0x02);
      active_channel_ = channel;
      acquisition_stage_ = 1;
      break;
      
    case 1:
      SPI_I2S_ReceiveData(SPI1);
      SPI_I2S_SendData(SPI1, active_channel_ << 6);
      acquisition_stage_ = 2;
      break;
      
    case 2:
      rx_word_ = (SPI_I2S_ReceiveData(SPI1) & 0xf) << 8;
      SPI_I2S_SendData(SPI1, 0x00);  // Dummy trailing data.
      acquisition_stage_ = 0;
      break;
  }
  return acquisition_stage_ == 1;
}

uint16_t Adc::Read(uint8_t channel) {
  uint16_t value = 0;
  
  // Send header
  GPIO_ResetBits(GPIOB, kPinSS);
  SPI_I2S_SendData(SPI1, 0x04 | 0x02);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  SPI_I2S_ReceiveData(SPI1);
  
  // Send channel
  SPI_I2S_SendData(SPI1, channel << 6);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  value = (SPI_I2S_ReceiveData(SPI1) & 0xf) << 8;

  // Send trailing word
  SPI_I2S_SendData(SPI1, 0x00);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  value |= SPI_I2S_ReceiveData(SPI1);
  
  GPIO_SetBits(GPIOB, kPinSS);
  return value;
}

}  // namespace braids
