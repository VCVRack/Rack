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

#include "tides/plotter.h"

#include "stmlib/utils/dsp.h"

#include "tides/resources.h"

namespace tides {

using namespace stmlib;

void Plotter::Run() {
  const PlotInstruction& instruction = program_[pc_];
  if (instruction.command == PLOT_MOVE_TO) {
    x_ = instruction.x;
    y_ = instruction.y;
    NextInstruction();
  } else {
    x_ = start_x_ + ((instruction.x - start_x_) * counter_) / instruction.steps;
    y_ = start_y_ + ((instruction.y - start_y_) * counter_) / instruction.steps;
    ++counter_;
    if (counter_ > instruction.steps) {
      x_ = instruction.x;
      y_ = instruction.y;
      NextInstruction();
    }
  }
}

void Plotter::NextInstruction() {
  counter_ = 0;
  start_x_ = x_;
  start_y_ = y_;
  pc_++;
  if (pc_ >= program_size_) {
    pc_ = 0;
  }
}

}  // namespace tides
