// Copyright 2014 Olivier Gillet.
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
// WM8371 Codec support.

#include "rings/drivers/codec.h"

#define CODEC_I2C                      I2C2
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C2
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C2
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_10
#define CODEC_I2C_SDA_PIN              GPIO_Pin_11
#define CODEC_I2C_SCL_PINSRC           GPIO_PinSource10
#define CODEC_I2C_SDA_PINSRC           GPIO_PinSource11
#define CODEC_TIMEOUT                  ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_TIMEOUT))
#define CODEC_I2C_SPEED                100000

#define CODEC_I2S                      SPI2
#define CODEC_I2S_EXT                  I2S2ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI2
#define CODEC_I2S_ADDRESS              0x4000380C
#define CODEC_I2S_EXT_ADDRESS          0x4000340C
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI2
#define CODEC_I2S_IRQ                  SPI2_IRQn
#define CODEC_I2S_EXT_IRQ              SPI2_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB)
#define CODEC_I2S_WS_PIN               GPIO_Pin_12
#define CODEC_I2S_SCK_PIN              GPIO_Pin_13
#define CODEC_I2S_SDI_PIN              GPIO_Pin_14
#define CODEC_I2S_SDO_PIN              GPIO_Pin_15
#define CODEC_I2S_MCK_PIN              GPIO_Pin_6
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource12
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource13
#define CODEC_I2S_SDI_PINSRC           GPIO_PinSource14
#define CODEC_I2S_SDO_PINSRC           GPIO_PinSource15
#define CODEC_I2S_MCK_PINSRC           GPIO_PinSource6
#define CODEC_I2S_GPIO                 GPIOB
#define CODEC_I2S_MCK_GPIO             GPIOC
#define AUDIO_I2S_IRQHandler           SPI2_IRQHandler

#define AUDIO_DMA_PERIPH_DATA_SIZE     DMA_PeripheralDataSize_HalfWord
#define AUDIO_DMA_MEM_DATA_SIZE        DMA_MemoryDataSize_HalfWord
#define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S_DMA_STREAM           DMA1_Stream4
#define AUDIO_I2S_DMA_DREG             CODEC_I2S_ADDRESS
#define AUDIO_I2S_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S_DMA_IRQ              DMA1_Stream4_IRQn
#define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF4
#define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF4
#define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF4
#define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF4
#define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF4
#define AUDIO_I2S_EXT_DMA_STREAM       DMA1_Stream3
#define AUDIO_I2S_EXT_DMA_DREG         CODEC_I2S_EXT_ADDRESS
#define AUDIO_I2S_EXT_DMA_CHANNEL      DMA_Channel_3
#define AUDIO_I2S_EXT_DMA_IRQ          DMA1_Stream3_IRQn
#define AUDIO_I2S_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF3
#define AUDIO_I2S_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF3
#define AUDIO_I2S_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF3
#define AUDIO_I2S_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF3
#define AUDIO_I2S_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF3
#define AUDIO_I2S_EXT_DMA_REG          DMA1
#define AUDIO_I2S_EXT_DMA_ISR          LISR
#define AUDIO_I2S_EXT_DMA_IFCR         LIFCR

#define W8731_ADDR_0 0x1A
#define W8731_ADDR_1 0x1B
#define W8731_NUM_REGS 10
#define CODEC_ADDRESS           (W8731_ADDR_0 << 1)

#define WAIT_LONG(x) { \
  uint32_t timeout = CODEC_LONG_TIMEOUT; \
  while (x) { if ((timeout--) == 0) return false; } \
}

#define WAIT(x) { \
  uint32_t timeout = CODEC_TIMEOUT; \
  while (x) { if ((timeout--) == 0) return false; } \
}

namespace rings {

/* static */
Codec* Codec::instance_;

enum CodecRegister {
  CODEC_REG_LEFT_LINE_IN = 0x00,
  CODEC_REG_RIGHT_LINE_IN = 0x01,
  CODEC_REG_LEFT_HEADPHONES_OUT = 0x02,
  CODEC_REG_RIGHT_HEADPHONES_OUT = 0x03,
  CODEC_REG_ANALOGUE_ROUTING = 0x04,
  CODEC_REG_DIGITAL_ROUTING = 0x05,
  CODEC_REG_POWER_MANAGEMENT = 0x06,
  CODEC_REG_DIGITAL_FORMAT = 0x07,
  CODEC_REG_SAMPLE_RATE = 0x08,
  CODEC_REG_ACTIVE = 0x09,
  CODEC_REG_RESET = 0x0f,
};

enum CodecSettings {
  CODEC_INPUT_0_DB = 0x17,
  CODEC_INPUT_UPDATE_BOTH = 0x40,
  CODEC_HEADPHONES_MUTE = 0x00,
  CODEC_MIC_BOOST = 0x1,
  CODEC_MIC_MUTE = 0x2,
  CODEC_ADC_MIC = 0x4,
  CODEC_ADC_LINE = 0x0,
  CODEC_OUTPUT_DAC_ENABLE = 0x10,
  CODEC_OUTPUT_MONITOR = 0x20,
  CODEC_DEEMPHASIS_NONE = 0x00,
  CODEC_DEEMPHASIS_32K = 0x01,
  CODEC_DEEMPHASIS_44K = 0x02,
  CODEC_DEEMPHASIS_48K = 0x03,
  CODEC_SOFT_MUTE = 0x01,
  CODEC_ADC_HPF = 0x00,
  
  CODEC_POWER_DOWN_LINE_IN = 0x01,
  CODEC_POWER_DOWN_MIC = 0x02,
  CODEC_POWER_DOWN_ADC = 0x04,
  CODEC_POWER_DOWN_DAC = 0x08,
  CODEC_POWER_DOWN_LINE_OUT = 0x10,
  CODEC_POWER_DOWN_OSCILLATOR = 0x20,
  CODEC_POWER_DOWN_CLOCK_OUTPUT = 0x40,
  CODEC_POWER_DOWN_EVERYTHING = 0x80,
  
  CODEC_PROTOCOL_MASK_MSB_FIRST = 0x00,
  CODEC_PROTOCOL_MASK_LSB_FIRST = 0x01,
  CODEC_PROTOCOL_MASK_PHILIPS = 0x02,
  CODEC_PROTOCOL_MASK_DSP = 0x03,
  
  CODEC_FORMAT_MASK_16_BIT = 0x00 << 2,
  CODEC_FORMAT_MASK_20_BIT = 0x01 << 2,
  CODEC_FORMAT_MASK_24_BIT = 0x02 << 2,
  CODEC_FORMAT_MASK_32_BIT = 0x03 << 2,
  
  CODEC_FORMAT_LR_SWAP = 0x20,
  CODEC_FORMAT_MASTER = 0x40,
  CODEC_FORMAT_SLAVE = 0x00,
  CODEC_FORMAT_INVERT_CLOCK = 0x80,
  
  CODEC_RATE_48K_48K = 0x00 << 2,
  CODEC_RATE_8K_8K = 0x03 << 2,
  CODEC_RATE_96K_96K = 0x07 << 2,
  CODEC_RATE_32K_32K = 0x06 << 2,
  CODEC_RATE_44K_44K = 0x08 << 2,
};

bool Codec::InitializeGPIO() {
  GPIO_InitTypeDef gpio_init;

  // Start GPIO peripheral clocks.
  RCC_AHB1PeriphClockCmd(CODEC_I2C_GPIO_CLOCK | CODEC_I2S_GPIO_CLOCK, ENABLE);

  // Initialize I2C pins
  gpio_init.GPIO_Pin = CODEC_I2C_SCL_PIN | CODEC_I2C_SDA_PIN; 
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_OType = GPIO_OType_OD;
  gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2C_GPIO, &gpio_init);

  // Connect pins to I2C peripheral
  GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2C_SCL_PINSRC, CODEC_I2C_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2C_SDA_PINSRC, CODEC_I2C_GPIO_AF);
  
  // Initialize I2S pins
  gpio_init.GPIO_Pin = CODEC_I2S_SCK_PIN | CODEC_I2S_SDO_PIN | \
      CODEC_I2S_SDI_PIN | CODEC_I2S_WS_PIN; 
  gpio_init.GPIO_Mode = GPIO_Mode_AF;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2S_GPIO, &gpio_init);
  
  gpio_init.GPIO_Pin = CODEC_I2S_MCK_PIN; 
  GPIO_Init(CODEC_I2S_MCK_GPIO, &gpio_init);
  
  // Connect pins to I2S peripheral.
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_WS_PINSRC, CODEC_I2S_GPIO_AF);  
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SCK_PINSRC, CODEC_I2S_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SDO_PINSRC, CODEC_I2S_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SDI_PINSRC, CODEC_I2S_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_MCK_GPIO, CODEC_I2S_MCK_PINSRC, CODEC_I2S_GPIO_AF); 
  return true;
}

bool Codec::InitializeControlInterface() {
  I2C_InitTypeDef i2c_init;

  // Initialize I2C
  RCC_APB1PeriphClockCmd(CODEC_I2C_CLK, ENABLE);

  I2C_DeInit(CODEC_I2C);
  i2c_init.I2C_Mode = I2C_Mode_I2C;
  i2c_init.I2C_DutyCycle = I2C_DutyCycle_2;
  i2c_init.I2C_OwnAddress1 = 0x33;
  i2c_init.I2C_Ack = I2C_Ack_Enable;
  i2c_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  i2c_init.I2C_ClockSpeed = CODEC_I2C_SPEED;
  
  I2C_Init(CODEC_I2C, &i2c_init);
  I2C_Cmd(CODEC_I2C, ENABLE);  
  
  return true;
}

bool Codec::InitializeAudioInterface(
    bool mcu_is_master,
    int32_t sample_rate) {
  // Configure PLL and I2S master clock.
  RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
  
  // The following values have been computed for a 8Mhz external crystal!
  RCC_PLLI2SCmd(DISABLE);
  if (sample_rate == 48000) {
    // 47.992kHz
    RCC_PLLI2SConfig(258, 3);
  } else if (sample_rate == 44100) {
    // 44.11kHz
    RCC_PLLI2SConfig(271, 6);
  } else if (sample_rate == 32000) {
    // 32.003kHz
    RCC_PLLI2SConfig(426, 4);
  } else if (sample_rate == 96000) {
    // 95.95 kHz
    RCC_PLLI2SConfig(393, 4);
  } else {
    // Unsupported sample rate!
    return false;
  }
  RCC_PLLI2SCmd(ENABLE);
  WAIT(RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY) == RESET);

  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

  // Initialize I2S
  I2S_InitTypeDef i2s_init;
  
  SPI_I2S_DeInit(CODEC_I2S);
  i2s_init.I2S_AudioFreq = sample_rate;
  i2s_init.I2S_Standard = I2S_Standard_Phillips;
  i2s_init.I2S_DataFormat = I2S_DataFormat_16b;
  i2s_init.I2S_CPOL = I2S_CPOL_Low;
  i2s_init.I2S_Mode = mcu_is_master ? I2S_Mode_MasterTx : I2S_Mode_SlaveTx;
  i2s_init.I2S_MCLKOutput = mcu_is_master
      ? I2S_MCLKOutput_Enable
      : I2S_MCLKOutput_Disable;

  // Initialize the I2S main channel for TX
  I2S_Init(CODEC_I2S, &i2s_init);
  
  // Initialize the I2S extended channel for RX
  I2S_FullDuplexConfig(CODEC_I2S_EXT, &i2s_init);
  
  return true;
}

bool Codec::WriteControlRegister(uint8_t address, uint16_t data) {
  uint8_t byte_1 = ((address << 1) & 0xfe) | ((data >> 8) & 0x01);
  uint8_t byte_2 = data & 0xff;
  
  WAIT_LONG(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BUSY));
  
  I2C_GenerateSTART(CODEC_I2C, ENABLE);
  WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT));

  I2C_Send7bitAddress(CODEC_I2C, CODEC_ADDRESS, I2C_Direction_Transmitter);
  WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

  I2C_SendData(CODEC_I2C, byte_1);
  WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING));

  I2C_SendData(CODEC_I2C, byte_2);
  WAIT(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING));

  WAIT_LONG(!I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BTF));

  I2C_GenerateSTOP(CODEC_I2C, ENABLE);  

  return true;  
}

bool Codec::InitializeCodec(
    bool mcu_is_master,
    int32_t sample_rate) {
  bool s = true;  // success;
  s = s && WriteControlRegister(CODEC_REG_RESET, 0);
  // Configure L&R inputs
  s = s && WriteControlRegister(CODEC_REG_LEFT_LINE_IN, CODEC_INPUT_0_DB);
  s = s && WriteControlRegister(CODEC_REG_RIGHT_LINE_IN, CODEC_INPUT_0_DB);
  
  // Configure L&R headphone outputs
  s = s && WriteControlRegister(CODEC_REG_LEFT_HEADPHONES_OUT, CODEC_HEADPHONES_MUTE);
  s = s && WriteControlRegister(CODEC_REG_RIGHT_HEADPHONES_OUT, CODEC_HEADPHONES_MUTE);

  // Configure analog routing
  s = s && WriteControlRegister(
      CODEC_REG_ANALOGUE_ROUTING,
      CODEC_MIC_MUTE | CODEC_ADC_LINE | CODEC_OUTPUT_DAC_ENABLE);

  // Configure digital routing
  s = s && WriteControlRegister(CODEC_REG_DIGITAL_ROUTING, CODEC_DEEMPHASIS_NONE);

  // Configure power management
  uint8_t power_down_reg = CODEC_POWER_DOWN_MIC | CODEC_POWER_DOWN_CLOCK_OUTPUT;
  if (mcu_is_master) {
    power_down_reg |= CODEC_POWER_DOWN_OSCILLATOR;
  }
    
  s = s && WriteControlRegister(CODEC_REG_POWER_MANAGEMENT, power_down_reg);
  
  uint8_t format_byte = CODEC_PROTOCOL_MASK_PHILIPS | CODEC_FORMAT_MASK_16_BIT;
  format_byte |= mcu_is_master ? CODEC_FORMAT_SLAVE : CODEC_FORMAT_MASTER;

  s = s && WriteControlRegister(CODEC_REG_DIGITAL_FORMAT, format_byte);
  
  uint8_t rate_byte = 0;
  if (mcu_is_master) {
    // According to the WM8731 datasheet, the 32kHz and 96kHz modes require the
    // master clock to be at 12.288 MHz (384 fs / 128 fs). The STM32F4 I2S clock
    // is always at 256 fs. So the 32kHz and 96kHz modes are achieved by
    // pretending that we are doing 48kHz, but with a slower or faster master
    // clock.
    rate_byte = sample_rate == 44100 ? CODEC_RATE_44K_44K : CODEC_RATE_48K_48K;
  } else {
    switch (sample_rate) {
      case 8000:
        rate_byte = CODEC_RATE_8K_8K;
        break;
      case 32000:
        rate_byte = CODEC_RATE_32K_32K;
        break;
      case 44100:
        rate_byte = CODEC_RATE_44K_44K;
        break;
      case 96000:
        rate_byte = CODEC_RATE_96K_96K;
        break;
      case 48000:
      default:
        rate_byte = CODEC_RATE_48K_48K;
        break;
    }
  }
  s = s && WriteControlRegister(CODEC_REG_SAMPLE_RATE, rate_byte);

  // For now codec is not active.
  s = s && WriteControlRegister(CODEC_REG_ACTIVE, 0x00);
  
  return s;
}

bool Codec::InitializeDMA() {
  RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK, ENABLE);

  // DMA setup for TX.
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_I2S_DMA_STREAM);

  dma_init_tx_.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;
  dma_init_tx_.DMA_PeripheralBaseAddr = AUDIO_I2S_DMA_DREG;
  dma_init_tx_.DMA_Memory0BaseAddr = (uint32_t)0;
  dma_init_tx_.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  dma_init_tx_.DMA_BufferSize = (uint32_t)0xFFFE;
  dma_init_tx_.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init_tx_.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init_tx_.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init_tx_.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma_init_tx_.DMA_Mode = DMA_Mode_Circular;
  dma_init_tx_.DMA_Priority = DMA_Priority_High;
  dma_init_tx_.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dma_init_tx_.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dma_init_tx_.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dma_init_tx_.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(AUDIO_I2S_DMA_STREAM, &dma_init_tx_);

  // DMA setup for RX.
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
  DMA_DeInit(AUDIO_I2S_EXT_DMA_STREAM);

  dma_init_rx_.DMA_Channel = AUDIO_I2S_EXT_DMA_CHANNEL;  
  dma_init_rx_.DMA_PeripheralBaseAddr = AUDIO_I2S_EXT_DMA_DREG;
  dma_init_rx_.DMA_Memory0BaseAddr = (uint32_t)0;
  dma_init_rx_.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dma_init_rx_.DMA_BufferSize = (uint32_t)0xFFFE;
  dma_init_rx_.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma_init_rx_.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma_init_rx_.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init_rx_.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; 
  dma_init_rx_.DMA_Mode = DMA_Mode_Circular;
  dma_init_rx_.DMA_Priority = DMA_Priority_High;
  dma_init_rx_.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  dma_init_rx_.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  dma_init_rx_.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dma_init_rx_.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
  DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &dma_init_rx_);  

  // Enable the interrupts.
  DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);
    
  // Enable the IRQ.
  NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);

  // Start DMA from/to codec.
  SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);
  SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);
  
  return true;
}

bool Codec::Init(
    bool mcu_is_master,
    int32_t sample_rate) {
  instance_ = this;
  callback_ = NULL;
  
  sample_rate_ = sample_rate;
  mcu_is_master_ = mcu_is_master;

  return InitializeGPIO() && \
      InitializeControlInterface() && \
      InitializeAudioInterface(mcu_is_master, sample_rate) && \
      InitializeCodec(mcu_is_master, sample_rate) && \
      InitializeDMA();
}

bool Codec::Start(size_t block_size, FillBufferCallback callback) {
  // Start the codec.
  if (!WriteControlRegister(CODEC_REG_ACTIVE, 0x01)) {
    return false;
  }
  if (block_size > kMaxCodecBlockSize) {
    return false;
  }
  
  if (!mcu_is_master_) {
    while(GPIO_ReadInputDataBit(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN));
    while(!GPIO_ReadInputDataBit(CODEC_I2S_GPIO, CODEC_I2S_WS_PIN));
  }
  
  callback_ = callback;
  
  // Enable the I2S TX and RX peripherals.
  if ((CODEC_I2S->I2SCFGR & 0x0400) == 0){
    I2S_Cmd(CODEC_I2S, ENABLE);
  }
  if ((CODEC_I2S_EXT->I2SCFGR & 0x0400) == 0){
    I2S_Cmd(CODEC_I2S_EXT, ENABLE);
  }
  
  dma_init_tx_.DMA_Memory0BaseAddr = (uint32_t)(tx_dma_buffer_);
  dma_init_rx_.DMA_Memory0BaseAddr = (uint32_t)(rx_dma_buffer_);

  size_t stride = 1;
  if (!mcu_is_master_) {
    // When the WM8731 is the master, the data is sent with padding.
    switch (sample_rate_) {
      case 32000:
        stride = 3;
        break;
      case 48000:
        stride = 2;
        break;
      case 96000:
        stride = 4;
        break;
    }
  }

  block_size_ = block_size;
  stride_ = stride;

  dma_init_tx_.DMA_BufferSize = 2 * stride * block_size * 2;
  dma_init_rx_.DMA_BufferSize = 2 * stride * block_size * 2;
  
  DMA_Init(AUDIO_I2S_DMA_STREAM, &dma_init_tx_);
  DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &dma_init_rx_);
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, ENABLE);
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, ENABLE);
  
  return true;
}

void Codec::Stop() {
  DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
  DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
}

bool Codec::set_line_input_gain(int32_t channel, int32_t gain) {
  return WriteControlRegister(CODEC_REG_LEFT_LINE_IN + channel, gain);
}

bool Codec::set_line_input_gain(int32_t gain) {
  return WriteControlRegister(0, gain) && WriteControlRegister(1, gain);
}

void Codec::Fill(size_t offset) {
  if (callback_) {
    offset *= block_size_ * stride_ * 2;
    short* in = &rx_dma_buffer_[offset];
    short* out = &tx_dma_buffer_[offset];
    if (stride_) {
      // Undo the padding from the WM8731.
      for (size_t i = 1; i < block_size_ * 2; ++i) {
        in[i] = in[i * stride_];
      }
    }
    (*callback_)((Frame*)(in), (Frame*)(out), block_size_);
    if (stride_) {
      // Pad for the WM8731.
      for (size_t i = block_size_ * 2 - 1; i > 0; --i) {
        out[i * stride_] = out[i];
      }
    }
  }
}

}  // namespace rings

extern "C" {
// Do not call into the firmware library to save on calls/jumps.
// if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC) != RESET) {
//  DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC);  

void DMA1_Stream3_IRQHandler(void) {
  if (AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_ISR & AUDIO_I2S_EXT_DMA_FLAG_TC) {
    AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_IFCR = AUDIO_I2S_EXT_DMA_FLAG_TC;
    rings::Codec::GetInstance()->Fill(1);
  }
  if (AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_ISR & AUDIO_I2S_EXT_DMA_FLAG_HT) {
    AUDIO_I2S_EXT_DMA_REG->AUDIO_I2S_EXT_DMA_IFCR = AUDIO_I2S_EXT_DMA_FLAG_HT;
    rings::Codec::GetInstance()->Fill(0);
  }
}
  
}
