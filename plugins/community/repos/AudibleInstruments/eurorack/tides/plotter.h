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
// Plotter.

#ifndef TIDES_PLOTTER_H_
#define TIDES_PLOTTER_H_

#include "stmlib/stmlib.h"

namespace tides {

enum PlotCommand {
  PLOT_MOVE_TO,
  PLOT_LINE_TO
};

struct PlotInstruction {
  PlotCommand command;
  uint8_t steps;
  uint16_t x;
  uint16_t y;
};

class Plotter {
 public:
  Plotter() { }
  ~Plotter() { }
  
  void Init(const PlotInstruction* program, size_t program_size) {
    program_ = program;
    program_size_ = program_size;
    pc_ = 0;
    counter_ = 0;
  }
  
  void Run();
  
  inline int32_t x() const { return x_; }
  inline int32_t y() const { return y_; }

 private:
  void NextInstruction();
  void Move();
   
  const PlotInstruction* program_;
  size_t program_size_;
  size_t pc_;

  int32_t start_x_;
  int32_t start_y_;
  int32_t x_;
  int32_t y_;
  int32_t counter_;
  
  DISALLOW_COPY_AND_ASSIGN(Plotter);
};

}  // namespace tides

#endif  // TIDES_PLOTTER_H_
