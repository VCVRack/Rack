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
// Record a note CV distribution - to be used for the quantizer.

#ifndef MARBLES_SCALE_RECORDER_H_
#define MARBLES_SCALE_RECORDER_H_

#include "stmlib/stmlib.h"
#include "marbles/random/quantizer.h"

namespace marbles {

class ScaleRecorder {
 public:
  ScaleRecorder() { }
  ~ScaleRecorder() { }

  struct Degree {
    float average_voltage;
    float total_voltage;
    float count;
    
    bool operator< (const Degree& rhs) const {
      return average_voltage < rhs.average_voltage;
    }
  };
  
  void Init() {
    Clear();
  }
  
  void Clear() {
    num_degrees_ = 0;
    current_voltage_ = 0.0f;
    total_count_ = 0.0f;
  }
  
  void NewNote(float v) {
    current_voltage_ = v;
  }
  
  void UpdateVoltage(float v) {
    ONE_POLE(current_voltage_, v, 0.01f);
  }
  
  void AcceptNote() {
    const float base_interval = 1.0f;
    float v = current_voltage_;
    while (v < 0.0f) {
      v += base_interval;
    }
    float octave = static_cast<float>(
        static_cast<int>(v / base_interval)) * base_interval;
    v -= octave;
    
    int nearest_degree = -1;
    for (int i = 0; i < num_degrees_; ++i) {
      float av = degrees_[i].average_voltage;
      const float tolerance = 1.0f / 36.0f;
      if (fabsf(v - av) < tolerance) {
        nearest_degree = i;
        break;
      }
      if (fabsf((v - base_interval) - av) < tolerance) {
        v -= base_interval;
        nearest_degree = i;
        break;
      }
    }
    if (nearest_degree == -1 && num_degrees_ != kMaxDegrees) {
      nearest_degree = num_degrees_;
      Degree* d = &degrees_[nearest_degree];
      d->total_voltage = 0.0f;
      d->average_voltage = 0.0f;
      d->count = 0.0f;
      ++num_degrees_;
    }
    
    if (nearest_degree != -1) {
      Degree* d = &degrees_[nearest_degree];
      d->total_voltage += v;
      d->count += 1.0f;
      d->average_voltage = d->total_voltage / d->count;
      total_count_ += 1.0f;
    }
  }
  
  bool ExtractScale(Scale* scale) {
    if (num_degrees_ < 2) {
      return false;
    }
    std::sort(&degrees_[0], &degrees_[num_degrees_]);

    float max_count = 0.0f;
    for (int i = 0; i < num_degrees_; ++i) {
      max_count = std::max(degrees_[i].count, max_count);
    }
    
    scale->base_interval = 1.0f;
    scale->num_degrees = num_degrees_;
    for (int i = 0; i < num_degrees_; ++i) {
      Degree* d = &degrees_[i];
      scale->degree[i].voltage = d->average_voltage;
      scale->degree[i].weight = static_cast<uint8_t>(
          255.0f * d->count / max_count);
      if (scale->degree[i].weight == 0) {
        ++scale->degree[i].weight;
      }
    }
    
    return true;
  }
  
 private:
  int num_degrees_;
  float current_voltage_;
  float total_count_;
  Degree degrees_[kMaxDegrees];
  
  DISALLOW_COPY_AND_ASSIGN(ScaleRecorder);
};

}  // namespace marbles

#endif  // MARBLES_SCALE_RECORDER_H_
