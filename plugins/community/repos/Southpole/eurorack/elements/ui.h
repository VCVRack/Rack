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
// User interface

#ifndef ELEMENTS_UI_H_
#define ELEMENTS_UI_H_

#include "stmlib/stmlib.h"

#include "stmlib/ui/event_queue.h"

#include "elements/drivers/leds.h"
#include "elements/drivers/switch.h"

namespace elements {

class CvScaler;
class Part;

enum UiMode {
  UI_MODE_NORMAL,
  UI_MODE_CALIBRATION_1,
  UI_MODE_CALIBRATION_2,
  UI_MODE_DISPLAY_MODEL,
  UI_MODE_PANIC,
};

enum FactoryTestingCommand {
  FACTORY_TESTING_READ_POT,
  FACTORY_TESTING_READ_CV,
  FACTORY_TESTING_READ_GATE,
  FACTORY_TESTING_SET_BYPASS,
  FACTORY_TESTING_CALIBRATE
};

class Ui {
 public:
  Ui() { }
  ~Ui() { }
  
  void Init(Part* part, CvScaler* cv_scaler);
  void Poll();
  void DoEvents();
  void FlushEvents();
  void Panic() {
    mode_ = UI_MODE_PANIC;
  }
  
  uint8_t HandleFactoryTestingRequest(uint8_t command);
  
  inline bool gate() const { return gate_; }
  
 private:
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);
  void CalibrateC1();
  void CalibrateC3();

  stmlib::EventQueue<16> queue_;
  
  Leds leds_;
  Switch switch_;
  uint32_t press_time_;
  UiMode mode_;
  
  bool gate_;
  
  CvScaler* cv_scaler_;
  Part* part_;
  
  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace elements

#endif  // ELEMENTS_UI_H_
