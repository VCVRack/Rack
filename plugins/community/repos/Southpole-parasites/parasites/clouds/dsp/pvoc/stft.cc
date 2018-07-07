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

#include "clouds/dsp/pvoc/stft.h"

#include <algorithm>

#include "clouds/dsp/pvoc/frame_transformation.h"
#include "stmlib/dsp/dsp.h"

namespace clouds {

using namespace std;
using namespace stmlib;

void STFT::Init(
    FFT* fft,
    size_t fft_size,
    size_t hop_size,
    float* fft_buffer,
    float* ifft_buffer,
    const float* window_lut,
    short* analysis_synthesis_buffer,
    Modifier* modifier) {
  fft_size_ = fft_size;
  hop_size_ = hop_size;
  fft_num_passes_ = 0;
  for (size_t t = fft_size; t > 1; t >>= 1) {
    ++fft_num_passes_;
  }
  buffer_size_ = fft_size_ + hop_size_;
  
  fft_ = fft;
#ifdef USE_ARM_FFT
  arm_rfft_fast_init_f32(fft_, fft_size);
#else
  fft_->Init();
#endif  // USE_ARM_FFT
  
  analysis_ = &analysis_synthesis_buffer[0];
  synthesis_ = &analysis_synthesis_buffer[buffer_size_];

  ifft_in_ = fft_in_ = fft_buffer;
  ifft_out_ = fft_out_ = ifft_buffer;
  
  window_ = window_lut;
  window_stride_ = LUT_SINE_WINDOW_4096_SIZE / fft_size;
  modifier_ = modifier;
  
  parameters_ = NULL;
  
  Reset();
}

void STFT::Reset() {
  buffer_ptr_ = 0;
  process_ptr_ = (2 * hop_size_) % buffer_size_;
  block_size_ = 0;
  fill(&analysis_[0], &analysis_[buffer_size_], 0);
  fill(&synthesis_[0], &synthesis_[buffer_size_], 0);
  ready_ = 0;
  done_ = 0;
}

void STFT::Process(
    const Parameters& parameters,
    const float* input,
    float* output,
    size_t size,
    size_t stride) {
  parameters_ = &parameters;
  while (size) {
    size_t processed = min(size, hop_size_ - block_size_);
    for (size_t i = 0; i < processed; ++i) {
      int32_t sample = *input * 32768.0f;
      analysis_[buffer_ptr_ + i] = Clip16(sample);
      *output = static_cast<float>(synthesis_[buffer_ptr_ + i]) / 16384.0f;
      input += stride;
      output += stride;
    }
    
    block_size_ += processed;
    size -= processed;
    buffer_ptr_ += processed;
    if (buffer_ptr_ >= buffer_size_) {
      buffer_ptr_ -= buffer_size_;
    }
    if (block_size_ >= hop_size_) {
      block_size_ -= hop_size_;
      ++ready_;
    }
  }
}

void STFT::Buffer() {
  if (ready_ == done_) {
    return;
  }
  
  // Copy block to FFT buffer and apply window.
  size_t source_ptr = process_ptr_;
  const float* w = window_;
  for (size_t i = 0; i < fft_size_; ++i) {
    fft_in_[i] = w[0] * analysis_[source_ptr];
    ++source_ptr;
    if (source_ptr >= buffer_size_) {
      source_ptr -= buffer_size_;
    }
    w += window_stride_;
  }
  
  // Compute FFT. fft_in is lost.
#ifdef USE_ARM_FFT
  arm_rfft_fast_f32(fft_, fft_in_, fft_out_, 0);
  copy(&fft_out_[0], &fft_out_[fft_size_], &fft_in_[0]);
  // Re-arrange data.
  for (size_t i = 0; i < fft_size_ / 2; ++i) {
    fft_out_[i] = fft_in_[2 * i];
    fft_out_[i + fft_size_ / 2] = fft_in_[2 * i + 1];
  }
#else
  if (fft_size_ != FFT::max_size) {
    fft_->Direct(fft_in_, fft_out_, fft_num_passes_);
  } else {
    fft_->Direct(fft_in_, fft_out_);
  }
#endif  // USE_ARM_FFT
  // Process in the frequency domain.
  if (modifier_ != NULL && parameters_ != NULL) {
    modifier_->Process(*parameters_, &fft_out_[0], &ifft_in_[0]);
  } else {
    copy(&fft_out_[0], &fft_out_[fft_size_], &ifft_in_[0]);
  }
  
  // Compute IFFT. ifft_in is lost.
#ifdef USE_ARM_FFT
  // Re-arrange data.
  copy(&ifft_in_[0], &ifft_in_[fft_size_], &ifft_out_[0]);
  for (size_t i = 0; i < fft_size_ / 2; ++i) {
    ifft_in_[2 * i] = ifft_out_[i];
    ifft_in_[2 * i + 1] = ifft_out_[i + fft_size_ / 2];
  }
  arm_rfft_fast_f32(fft_, ifft_in_, ifft_out_, 1);
#else
  if (fft_size_ != FFT::max_size) {
    fft_->Inverse(ifft_in_, ifft_out_, fft_num_passes_);
  } else {
    fft_->Inverse(ifft_in_, ifft_out_);
  }
#endif  // USE_ARM_FFT
  
  size_t destination_ptr = process_ptr_;
#ifdef USE_ARM_FFT
  float inverse_window_size = 1.0f / \
      float(fft_size_ / hop_size_ >> 1);
#else
  float inverse_window_size = 1.0f / \
      float(fft_size_ * fft_size_ / hop_size_ >> 1);
#endif  // USE_ARM_FFT
    
  w = window_;
  for (size_t i = 0; i < fft_size_; ++i) {
    float s = ifft_out_[i] * w[0] * inverse_window_size;
    
    int32_t x = static_cast<int32_t>(s);
    if (i < fft_size_ - hop_size_) {
      // Overlap-add.
      x += synthesis_[destination_ptr];
    }
    synthesis_[destination_ptr] = Clip16(x);
    ++destination_ptr;
    if (destination_ptr >= buffer_size_) {
      destination_ptr -= buffer_size_;
    }
    w += window_stride_;
  }

  ++done_;
  process_ptr_ += hop_size_;
  if (process_ptr_ >= buffer_size_) {
    process_ptr_ -= buffer_size_;
  }
}

}  // namespace clouds
