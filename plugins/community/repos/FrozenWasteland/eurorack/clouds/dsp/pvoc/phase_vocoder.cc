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
// Naive phase vocoder.

#include "clouds/dsp/pvoc/phase_vocoder.h"

#include <algorithm>

#include "stmlib/utils/buffer_allocator.h"

namespace clouds {

using namespace std;
using namespace stmlib;

void PhaseVocoder::Init(
    void** buffer,
    size_t* buffer_size,
    const float* large_window_lut,
    size_t largest_fft_size,
    int32_t num_channels,
    int32_t resolution,
    float sample_rate) {
  num_channels_ = num_channels;

  size_t fft_size = largest_fft_size;
  size_t hop_ratio = 4;
  
  BufferAllocator allocator_0(buffer[0], buffer_size[0]);
  BufferAllocator allocator_1(buffer[1], buffer_size[1]);
  BufferAllocator* allocator[2] = { &allocator_0, &allocator_1 };
  float* fft_buffer = allocator[0]->Allocate<float>(fft_size);
  float* ifft_buffer = allocator[num_channels_ - 1]->Allocate<float>(fft_size);
  
  size_t num_textures = kMaxNumTextures;
  size_t texture_size = (fft_size >> 1) - kHighFrequencyTruncation;
  for (int32_t i = 0; i < num_channels_; ++i) {
    short* ana_syn_buffer = allocator[i]->Allocate<short>(
        (fft_size + (fft_size >> 1)) * 2);
    
    num_textures = min(
        allocator[i]->free() / (sizeof(float) * texture_size),
        num_textures);
    stft_[i].Init(
        &fft_,
        fft_size,
        fft_size / hop_ratio,
        fft_buffer,
        ifft_buffer,
        large_window_lut,
        ana_syn_buffer,
        &frame_transformation_[i]);
  }
  for (int32_t i = 0; i < num_channels_; ++i) {
    float* texture_buffer = allocator[i]->Allocate<float>(
        num_textures * texture_size);
    frame_transformation_[i].Init(texture_buffer, fft_size, num_textures);
  }
}

void PhaseVocoder::Process(
    const Parameters& parameters,
    const FloatFrame* input,
    FloatFrame* output, size_t size) {
  const float* input_samples = &input[0].l;
  float* output_samples = &output[0].l;
  for (int32_t i = 0; i < num_channels_; ++i) {
    stft_[i].Process(
        parameters,
        input_samples + i,
        output_samples + i,
        size,
        2);
  }
}

void PhaseVocoder::Buffer() {
  for (int32_t i = 0; i < num_channels_; ++i) {
    stft_[i].Buffer();
  }
}

}  // namespace clouds
