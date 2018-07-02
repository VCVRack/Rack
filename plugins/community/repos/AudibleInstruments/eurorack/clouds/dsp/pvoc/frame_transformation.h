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
// Transformations applied to a single STFT slice.

#ifndef CLOUDS_DSP_PVOC_FRAME_TRANSFORMATION_H_
#define CLOUDS_DSP_PVOC_FRAME_TRANSFORMATION_H_

#include "stmlib/stmlib.h"

#include "clouds/dsp/pvoc/stft.h"

#include "clouds/resources.h"

namespace clouds {

const int32_t kMaxNumTextures = 7;
const int32_t kHighFrequencyTruncation = 16;

struct Parameters;

class FrameTransformation {
 public:
  FrameTransformation() { }
  ~FrameTransformation() { }
  
  void Init(float* buffer, int32_t fft_size, int32_t num_textures);
  void Reset();
  
  void Process(
      const Parameters& parameters,
      float* fft_out,
      float* ifft_in);
  
 private:
  void RectangularToPolar(float* fft_data);
  void PolarToRectangular(float* fft_data);
  void AddGlitch(float* xf_polar);
  void ShiftMagnitudes(
      float* source,
      float* xf_polar,
      float pitch_ratio);
  void WarpMagnitudes(
      float* source,
      float* xf_polar,
      float amount);
  void QuantizeMagnitudes(float* xf_polar, float amount);
  void StoreMagnitudes(float* xf_polar, float position, float feedback);
  void SetPhases(float* destination, float diffusion, float pitch_ratio);
  void ReplayMagnitudes(float* xf_polar, float position);
  void DiffuseMagnitudes(float* xf_polar, float diffusion);
  
  inline void fast_p2r(float magnitude, uint16_t angle, float* re, float* im) {
    angle >>= 6;
    *re = magnitude * lut_sin[angle + 256];
    *im = magnitude * lut_sin[angle];
  }
  
  int32_t fft_size_;
  int32_t num_textures_;
  int32_t size_;
  
  // Magnitude buffers.
  float* textures_[kMaxNumTextures];
  
  // Original phase and phase unrolling buffers.
  uint16_t* phases_;
  uint16_t* phases_delta_;

  int8_t glitch_algorithm_;
  
  DISALLOW_COPY_AND_ASSIGN(FrameTransformation);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_PVOC_FRAME_TRANSFORMATION_H_
