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

#ifndef STAGES_FACTORY_TEST_H_
#define STAGES_FACTORY_TEST_H_

#include "stmlib/stmlib.h"

#include "stages/drivers/serial_link.h"
#include "stages/io_buffer.h"

namespace stages {

enum FactoryTestCommand {
  FACTORY_TEST_READ_POT,
  FACTORY_TEST_READ_CV,
  FACTORY_TEST_READ_GATE,
  FACTORY_TEST_GENERATE_TEST_SIGNALS,
  FACTORY_TEST_CALIBRATE,
  FACTORY_TEST_READ_NORMALIZATION,
  FACTORY_TEST_FORCE_DAC_CODE,
  FACTORY_TEST_WRITE_CALIBRATION_DATA_NIBBLE,
};

class Settings;
class CvReader;
class GateInputs;
class Ui;

class FactoryTest {
 public:
  FactoryTest() { instance_ = NULL; }
  ~FactoryTest() { }
  
  void Start(
      Settings* settings,
      CvReader* cv_reader,
      GateInputs* gate_inputs,
      Ui* ui);
  
  inline void Poll() {
    if (running()) {
      const uint8_t* command = link_.available_rx_buffer();
      if (command) {
        tx_buffer_[0] = HandleRequest(*command);
        link_.Transmit(tx_buffer_);
      }
    }
  }
  
  static void ProcessFn(IOBuffer::Block* block, size_t size);
  void Process(IOBuffer::Block* block, size_t size);
  
  inline bool running() const { return instance_ != NULL; }
  static FactoryTest* GetInstance() { return instance_; }

 private:
  uint8_t HandleRequest(uint8_t command);
  
  static FactoryTest* instance_;

  SerialLink link_;
  uint8_t tx_buffer_[1];
  uint8_t rx_buffer_[2];
  
  Settings* settings_;
  CvReader* cv_reader_;
  GateInputs* gate_inputs_;
  Ui* ui_;
  
  uint16_t forced_dac_code_[kNumChannels];
  uint32_t calibration_data_;
  float calibration_first_adc_value_;
  
  DISALLOW_COPY_AND_ASSIGN(FactoryTest);
};

}  // namespace stages

#endif  // STAGES_FACTORY_TEST_H_
