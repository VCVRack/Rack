// Copyright 2017 Olivier Gillet.
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
// CV reader.

#include "stages/factory_test.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/units.h"

#include "stages/drivers/gate_inputs.h"
#include "stages/cv_reader.h"
#include "stages/resources.h"
#include "stages/settings.h"
#include "stages/ui.h"

namespace stages {
  
using namespace std;
using namespace stmlib;

/* static */
FactoryTest* FactoryTest::instance_;

void FactoryTest::Start(
    Settings* settings,
    CvReader* cv_reader,
    GateInputs* gate_inputs,
    Ui* ui) {
  instance_ = this;

  settings_ = settings;
  cv_reader_ = cv_reader;
  gate_inputs_ = gate_inputs;
  ui_ = ui;
  
  calibration_data_ = 0;
  calibration_first_adc_value_ = 0.0f;
  
  link_.Init(
      SERIAL_LINK_DIRECTION_LEFT,
      9600,
      rx_buffer_,
      1);
}

/* static */
void FactoryTest::ProcessFn(IOBuffer::Block* block, size_t size) {
  FactoryTest::GetInstance()->Process(block, size);
}

void FactoryTest::Process(IOBuffer::Block* block, size_t size) {
  static float phase;
  for (size_t i = 0; i < size; ++i) {
    phase += 100.0f / static_cast<float>(kSampleRate);
    if (phase >= 1.0f) {
      phase -= 1.0f;
    }

    block->output[0][i] = settings_->calibration_data(0).dac_code(
        phase < 0.2f ? 0.5f : -0.5f);

    block->output[1][i] = settings_->calibration_data(1).dac_code(
        phase < 0.5f ? 0.5f : -0.5f);

    block->output[2][i] = settings_->calibration_data(2).dac_code(
        phase < 0.8f ? 0.5f : -0.5f);

    block->output[3][i] = settings_->calibration_data(3).dac_code(
        0.5f * stmlib::Interpolate(lut_sine, phase, 1024.0f));

    block->output[4][i] = settings_->calibration_data(4).dac_code(
        -1.0f * phase + 0.5f);

    block->output[5][i] = settings_->calibration_data(5).dac_code(
        2.0f * (phase < 0.5f ? phase : 1.0f - phase) - 0.5f);

    for (size_t j = 0; j < kNumChannels; ++j) {
      uint16_t dac_code = forced_dac_code_[j];
      if (dac_code) {
        block->output[j][i] = dac_code;
      }
    }
  }
}

uint8_t FactoryTest::HandleRequest(uint8_t command) {
  uint8_t argument = command & 0x1f;
  command = command >> 5;
  uint8_t reply = 0;
  switch (command) {
    case FACTORY_TEST_READ_POT:
      reply = argument < kNumChannels
          ? cv_reader_->raw_pot(argument)
          : cv_reader_->raw_slider(argument - kNumChannels);
      break;

    case FACTORY_TEST_READ_CV:
      reply = cv_reader_->raw_cv(argument);
      break;
    
    case FACTORY_TEST_READ_NORMALIZATION:
      reply = gate_inputs_->is_normalized(argument) ? 255 : 0;
      break;      
    
    case FACTORY_TEST_READ_GATE:
      reply = argument < kNumChannels
          ? gate_inputs_->value(argument)
          : ui_->switches().pressed(argument - kNumChannels);
      break;
      
    case FACTORY_TEST_GENERATE_TEST_SIGNALS:
      fill(
          &forced_dac_code_[0],
          &forced_dac_code_[kNumChannels],
          0);
      break;
      
    case FACTORY_TEST_CALIBRATE:
      {
        int channel = argument >> 2;
        int step = argument & 0x3;
        if (step == 0) {
          for (size_t i = 0; i < kNumChannels; ++i) {
            forced_dac_code_[i] = settings_->dac_code(i, -0.125f);
          }
        } else if (step == 1) {
          calibration_first_adc_value_ = cv_reader_->lp_cv(channel);
          for (size_t i = 0; i < kNumChannels; ++i) {
            forced_dac_code_[i] = settings_->dac_code(i, +0.25f);
          }
        } else {
          float calibration_vm2 = calibration_first_adc_value_;
          float calibration_vp4 = cv_reader_->lp_cv(channel);
          float scale = 0.375f / (calibration_vp4 - calibration_vm2);
          float offset = 0.25f - calibration_vp4 * scale;
          
          if (scale > -1.2f && scale < -0.8f) {
            ChannelCalibrationData* cal = settings_->mutable_calibration_data(
                channel);
            cal->adc_scale = scale;
            cal->adc_offset = offset;
            settings_->SavePersistentData();
            ui_->set_slider_led(channel, true, 2);
          }
        }
      }
      break;
      
    case FACTORY_TEST_FORCE_DAC_CODE:
      {
        int channel = argument >> 2;
        int step = argument & 0x3;
        if (step == 0) {
          forced_dac_code_[channel] = 0x9ff1;
        } else if (step == 1) {
          forced_dac_code_[channel] = 0x416b;
        }
        else {
          ChannelCalibrationData* cal = settings_->mutable_calibration_data(
              channel);
          cal->dac_offset = static_cast<float>(calibration_data_ & 0xffff);
          cal->dac_scale = -static_cast<float>(calibration_data_ >> 16);
          forced_dac_code_[channel] = settings_->dac_code(
              channel, 0.125f);
          settings_->SavePersistentData();
          ui_->set_slider_led(channel, true, 1);
        }
      }
      break;
      
    case FACTORY_TEST_WRITE_CALIBRATION_DATA_NIBBLE:
      calibration_data_ <<= 4;
      calibration_data_ |= argument & 0xf;
      break;
  }
  return reply;
}

}  // namespace stages
