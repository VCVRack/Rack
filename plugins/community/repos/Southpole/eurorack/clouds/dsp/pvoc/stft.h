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
// STFT with overlap-add.

#ifndef CLOUDS_DSP_PVOC_STFT_H_
#define CLOUDS_DSP_PVOC_STFT_H_

#include "stmlib/stmlib.h"

// #define USE_ARM_FFT

#ifdef USE_ARM_FFT
  #include <arm_math.h>
#else
  #include "stmlib/fft/shy_fft.h"
#endif  // USE_ARM_FFT

namespace clouds {

struct Parameters;

const size_t kMaxFftSize = 4096;
#ifdef USE_ARM_FFT
  typedef arm_rfft_fast_instance_f32 FFT;
#else
  typedef stmlib::ShyFFT<float, kMaxFftSize, stmlib::RotationPhasor> FFT;
#endif  // USE_ARM_FFT

typedef class FrameTransformation Modifier;

class STFT {
 public:
  STFT() { }
  ~STFT() { }
  
  struct Frame { short l; short r; };
  
  void Init(
      FFT* fft,
      size_t fft_size,
      size_t hop_size,
      float* fft_buffer,
      float* ifft_buffer,
      const float* window_lut,
      short* stft_frame_processor_buffer,
      Modifier* modifier);

  void Reset();

  void Process(
      const Parameters& parameters,
      const float* input,
      float* output,
      size_t size,
      size_t stride);

  void Buffer();
  
 private:
  FFT* fft_;
  size_t fft_size_;
  size_t fft_num_passes_;
  size_t hop_size_;
  size_t buffer_size_;
  float* fft_in_;
  float* fft_out_;
  float* ifft_out_;
  float* ifft_in_;
  
  const float* window_;
  size_t window_stride_;

  short* analysis_;
  short* synthesis_;
  
  size_t buffer_ptr_;
  size_t process_ptr_;
  size_t block_size_;
  
  size_t ready_;
  size_t done_;
  
  const Parameters* parameters_;
  
  Modifier* modifier_;
  
  DISALLOW_COPY_AND_ASSIGN(STFT);
};

}  // namespace clouds

#endif  // CLOUDS_DSP_PVOC_STFT_H_
