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
// Dynamics processor

#include "streams/processor.h"

#include <algorithm>

namespace streams {

using namespace stmlib;
using namespace std;

#define REGISTER_PROCESSOR(ClassName) \
  { &Processor::ClassName ## Init, \
    &Processor::ClassName ## Process, \
    &Processor::ClassName ## Configure },

/* static */
const Processor::ProcessorCallbacks 
Processor::callbacks_table_[PROCESSOR_FUNCTION_LAST] = {
  REGISTER_PROCESSOR(Envelope)
  REGISTER_PROCESSOR(Vactrol)
  REGISTER_PROCESSOR(Follower)
  REGISTER_PROCESSOR(Compressor)
  REGISTER_PROCESSOR(FilterController)
  REGISTER_PROCESSOR(LorenzGenerator)
};

void Processor::Init(uint8_t index) {
  for (uint8_t i = 0; i < PROCESSOR_FUNCTION_LAST; ++i) {
    (this->*callbacks_table_[i].init)();
  }
  dirty_ = true;
  alternate_ = false;
  linked_ = false;
  
  fill(&parameters_[0], &parameters_[2], 32768);
  fill(&globals_[0], &globals_[4], 32768);
  
  set_function(PROCESSOR_FUNCTION_ENVELOPE);
  
  lorenz_generator_.set_index(index);
}

}  // namespace streams
