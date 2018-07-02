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
// WAV-file writer.

#ifndef STMLIB_TEST_WAV_WRITER_H_
#define STMLIB_TEST_WAV_WRITER_H_

#include <cstdio>
#include <cstdlib>

namespace stmlib {

class WavWriter {
 public:
  WavWriter(size_t num_channels,
      size_t sample_rate,
      size_t duration) :
      num_channels_(num_channels),
      sample_rate_(sample_rate), duration_(duration),
      remaining_frames_(duration * sample_rate),
      fp_(NULL) { }

  ~WavWriter() {
    if (fp_) {
      fclose(fp_);
    }
  }
  
  bool Open(const char* file_name) {
    fp_ = fopen(file_name, "wb");
    if (!fp_) {
      return false;
    }
    
    uint32_t l;
    uint16_t s;
  
    fwrite("RIFF", 4, 1, fp_);
    l = 36 + duration_ * sample_rate_ * 2 * num_channels_;
    fwrite(&l, 4, 1, fp_);
    fwrite("WAVE", 4, 1, fp_);
  
    fwrite("fmt ", 4, 1, fp_);
    l = 16;
    fwrite(&l, 4, 1, fp_);
    s = 1;
    fwrite(&s, 2, 1, fp_);
    s = num_channels_;
    fwrite(&s, 2, 1, fp_);
    l = sample_rate_;
    fwrite(&l, 4, 1, fp_);
    l = static_cast<uint32_t>(sample_rate_) * 2 * num_channels_;
    fwrite(&l, 4, 1, fp_);
    s = 2 * num_channels_;
    fwrite(&s, 2, 1, fp_);
    s = 16;
    fwrite(&s, 2, 1, fp_);
  
    fwrite("data", 4, 1, fp_);
    l = duration_ * sample_rate_ * 2 * num_channels_;
    fwrite(&l, 4, 1, fp_);
    return true;
  }
  
  void WriteFrames(short* data, size_t num_frames) {
    fwrite(data, sizeof(short) * num_channels_, num_frames, fp_);
    remaining_frames_ -= num_frames;
  }
  
  void Write(float* out, size_t size, float gain=32767.0f) {
    int16_t* short_buffer = (int16_t*)(calloc(size, sizeof(int16_t)));
    for (size_t i = 0; i < size; ++i) {
      float x = out[i] * gain;
      if (x >= 32767.0f) x = 32767.0f;
      if (x <= -32767.0f) x = -32767.0f;
      short_buffer[i] = static_cast<int16_t>(x);
    }
    fwrite(short_buffer, sizeof(int16_t), size, fp_);
    remaining_frames_ -= size / num_channels_;
  }
  
  void Write(float* l, float* r, size_t size, float gain=32767.0f) {
    int16_t* short_buffer = (int16_t*)(calloc(size * 2, sizeof(int16_t)));
    for (size_t i = 0; i < size; ++i) {
      {
        float x = l[i] * gain;
        if (x >= 32767.0f) x = 32767.0f;
        if (x <= -32767.0f) x = -32767.0f;
        short_buffer[i * 2] = static_cast<int16_t>(x);
      }
      {
        float x = r[i] * gain;
        if (x >= 32767.0f) x = 32767.0f;
        if (x <= -32767.0f) x = -32767.0f;
        short_buffer[i * 2 + 1] = static_cast<int16_t>(x);
      }
    }
    fwrite(short_buffer, sizeof(int16_t), size * 2, fp_);
    remaining_frames_ -= size;
  }
  
  size_t remaining_frames() { return remaining_frames_; }
  float progress() {
    return float(remaining_frames_) / float(sample_rate_ * duration_);
  }
  bool done() { return remaining_frames_ == 0; }
  
  float triangle(size_t division = 8) {
    uint16_t tri = (remaining_frames() / division);
    tri = tri > 32767 ? 65535 - tri : tri;
    return tri / 32768.0f;
  }

 private:
  size_t num_channels_;
  size_t sample_rate_;
  size_t duration_;
  size_t remaining_frames_;
  
  FILE* fp_;
};

}  // namespace stmlib

#endif  // STMLIB_TEST_WAV_WRITER_H_
