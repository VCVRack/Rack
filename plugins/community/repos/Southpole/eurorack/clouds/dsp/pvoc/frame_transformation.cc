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

#include "clouds/dsp/pvoc/frame_transformation.h"

#include <algorithm>

#include "stmlib/dsp/atan.h"
#include "stmlib/dsp/units.h"
#include "stmlib/utils/random.h"

#include "clouds/dsp/frame.h"
#include "clouds/dsp/parameters.h"

namespace clouds {

using namespace std;
using namespace stmlib;

void FrameTransformation::Init(
    float* buffer,
    int32_t fft_size,
    int32_t num_textures) {
  fft_size_ = fft_size;
  size_ = (fft_size >> 1) - kHighFrequencyTruncation;
  
  for (int32_t i = 0; i < num_textures; ++i) {
    textures_[i] = &buffer[i * size_];
  }
  phases_ = static_cast<uint16_t*>((void*)(textures_[num_textures - 1]));
  num_textures_ = num_textures - 1;  // Last texture is used for storing phases.
  phases_delta_ = phases_ + size_;

  glitch_algorithm_ = 0;
  Reset();
}

void FrameTransformation::Reset() {
  for (int32_t i = 0; i < num_textures_; ++i) {
    fill(&textures_[i][0], &textures_[i][size_], 0.0f);
  }
}

void FrameTransformation::Process(
    const Parameters& parameters,
    float* fft_out,
    float* ifft_in) {
  fft_out[0] = 0.0f;
  fft_out[fft_size_ >> 1] = 0.0f;

  bool freeze = parameters.freeze;
  bool glitch = parameters.gate;
  float pitch_ratio = SemitonesToRatio(parameters.pitch);
  
  if (!freeze) {
    RectangularToPolar(fft_out);
    StoreMagnitudes(
        fft_out,
        parameters.position,
        parameters.spectral.refresh_rate);
  }
  float* temp = &fft_out[0];
  ReplayMagnitudes(ifft_in, parameters.position);
  WarpMagnitudes(ifft_in, temp, parameters.spectral.warp);
  ShiftMagnitudes(temp, ifft_in, pitch_ratio);
  if (glitch) {
    AddGlitch(ifft_in);
  }
  QuantizeMagnitudes(ifft_in, parameters.spectral.quantization);
  SetPhases(ifft_in, parameters.spectral.phase_randomization, pitch_ratio);
  PolarToRectangular(ifft_in);

  if (!glitch) {
    // Decide on which glitch algorithm will be used next time... if glitch
    // is enabled on the next frame!
    glitch_algorithm_ = stmlib::Random::GetSample() & 3;
  }

  ifft_in[0] = 0.0f;
  ifft_in[fft_size_ >> 1] = 0.0f;
}

void FrameTransformation::RectangularToPolar(float* fft_data) {
  float* real = &fft_data[0];
  float* imag = &fft_data[fft_size_ >> 1];
  float* magnitude = &fft_data[0];
  for (int32_t i = 1; i < size_; ++i) {
    uint16_t angle = fast_atan2r(imag[i], real[i], &magnitude[i]);
    phases_delta_[i] = angle - phases_[i];
    phases_[i] = angle;
  }
}

void FrameTransformation::SetPhases(
    float* destination,
    float phase_randomization,
    float pitch_ratio) {
  uint32_t* synthesis_phase = (uint32_t*) &destination[fft_size_ >> 1];
  for (int32_t i = 0; i < size_; ++i) {
    synthesis_phase[i] = phases_[i];
    phases_[i] += static_cast<uint16_t>(
        static_cast<float>(phases_delta_[i]) * pitch_ratio);
  }
  float r = phase_randomization;
  r = (r - 0.05f) * 1.06f;
  CONSTRAIN(r, 0.0f, 1.0f);
  r *= r;
  int32_t amount = static_cast<int32_t>(r * 32768.0f);
  for (int32_t i = 0; i < size_; ++i) {
    synthesis_phase[i] += \
        static_cast<int32_t>(stmlib::Random::GetSample()) * amount >> 14;
  }
}

void FrameTransformation::PolarToRectangular(float* fft_data) {
  float* real = &fft_data[0];
  float* imag = &fft_data[fft_size_ >> 1];
  float* magnitude = &fft_data[0];
  uint32_t* angle = (uint32_t*) &fft_data[fft_size_ >> 1];
  for (int32_t i = 1; i < size_; ++i) {
    fast_p2r(magnitude[i], angle[i], &real[i], &imag[i]);
  }
  for (int32_t i = size_; i < fft_size_ >> 1; ++i) {
    real[i] = imag[i] = 0.0f;
  }
}

void FrameTransformation::AddGlitch(float* xf_polar) {
  float* x = xf_polar;
  switch (glitch_algorithm_) {
    case 0:
      // Spectral hold and blow.
      {
        // Create trails
        float held = 0.0;
        for (int32_t i = 0; i < size_; ++i) {
          if ((stmlib::Random::GetSample() & 15) == 0) {
            held = x[i];
          }
          x[i] = held;
          held = held * 1.01f;
        }
      }
      break;
      
    case 1:
      // Spectral shift up with aliasing.
      {
        float factor = 1.0f + (stmlib::Random::GetSample() & 7) / 4.0f;
        float source = 0.0f;
        for (int32_t i = 0; i < size_; ++i) {
          source += factor;
          if (source >= size_) {
            source = 0.0f;
          }
          x[i] = x[static_cast<int32_t>(source)];
        }
      }
      break;
      
    case 2:
      // Kill largest harmonic and boost second largest.
      *std::max_element(&x[0], &x[size_]) = 0.0f;
      *std::max_element(&x[0], &x[size_]) *= 8.0f;
      break;
      
    case 3:
      {
        // Nasty high-pass
        for (int32_t i = 0; i < size_; ++i) {
          uint32_t random = stmlib::Random::GetSample() & 15;
          if (random == 0) {
            x[i] *= static_cast<float>(i) / 16.0f;
          }
        }
      }
      break;
      
    default:
      break;
  }
}

void FrameTransformation::QuantizeMagnitudes(float* xf_polar, float amount) {
  if (amount <= 0.48f) {
    amount = amount * 2.0f;
    float scale_down = 0.5f * SemitonesToRatio(
        -108.0f * (1.0f - amount * amount)) / float(fft_size_);
    float scale_up = 1.0f / scale_down;
    for (int32_t i = 0.0f; i < size_; ++i) {
      xf_polar[i] = scale_up * static_cast<float>(
          static_cast<int32_t>(scale_down * xf_polar[i]));
    }
  } else if (amount >= 0.52f) {
    amount = (amount - 0.52f) * 2.0f;
    float norm = *std::max_element(&xf_polar[0], &xf_polar[size_]);
    float inv_norm = 1.0f / (norm + 0.0001f);
    for (int32_t i = 1.0f; i < size_; ++i) {
      float x = xf_polar[i] * inv_norm;
      float warped = 4.0f * x * (1.0f - x) * (1.0f - x) * (1.0f - x);
      xf_polar[i] = (x + (warped - x) * amount) * norm;
    }
  }
}

const float kWarpPolynomials[6][4] = {
  { 10.5882f, -14.8824f, 5.29412f, 0.0f },
  { -7.3333f, +9.0, -1.79167f, 0.125f },
  { 0.0f, 0.0f, 1.0f, 0.0f },
  { 0.0f, 0.5f, 0.5f, 0.0f },
  { -7.3333f, +9.5f, -2.416667f, 0.25f },
  { -7.3333f, +9.5f, -2.416667f, 0.25f },
};

void FrameTransformation::WarpMagnitudes(
    float* source,
    float* xf_polar,
    float amount) {
  float bin_width = 1.0f / static_cast<float>(size_);
  float f = 0.0;
  
  float coefficients[4];
  amount *= 4.0f;
  MAKE_INTEGRAL_FRACTIONAL(amount);
  for (int32_t i = 0; i < 4; ++i) {
    coefficients[i] = Crossfade(
        kWarpPolynomials[amount_integral][i],
        kWarpPolynomials[amount_integral + 1][i],
        amount_fractional);
  }
  
  float a = coefficients[0];
  float b = coefficients[1];
  float c = coefficients[2];
  float d = coefficients[3];
  
  for (int32_t i = 1.0f; i < size_; ++i) {
    f += bin_width;
    float wf = (d + f * (c + f * (b + a * f))) * size_;
    xf_polar[i] = Interpolate(source, wf, 1.0f);
  }
}

void FrameTransformation::ShiftMagnitudes(
    float* source,
    float* xf_polar,
    float pitch_ratio) {
  float* destination = &xf_polar[0];
  float* temp = &xf_polar[size_];
  if (pitch_ratio == 1.0f) {
    copy(&source[0], &source[size_], &temp[0]);
  } else if (pitch_ratio > 1.0f) {
    float index = 1.0f;
    float increment = 1.0f / pitch_ratio;
    for (int32_t i = 1; i < size_; ++i) {
      temp[i] = Interpolate(source, index, 1.0f);
      index += increment;
    }
  } else {
    fill(&temp[0], &temp[size_], 0.0f);
    float index = 1.0f;
    float increment = pitch_ratio;
    for (int32_t i = 1; i < size_; ++i) {
      MAKE_INTEGRAL_FRACTIONAL(index)
      temp[index_integral] += (1.0f - index_fractional) * source[i];
      temp[index_integral + 1] += index_fractional * source[i];
      index += increment;
    }
  }
  copy(&temp[0], &temp[size_], &destination[0]);
}

void FrameTransformation::StoreMagnitudes(
    float* xf_polar,
    float position,
    float feedback) {
  // Write into magnitude buffers.
  float index_float = position * float(num_textures_ - 1);
  int32_t index_int = static_cast<int32_t>(index_float);
  float index_fractional = index_float - index_int;
  float gain_a = 1.0f - index_fractional;
  float gain_b = index_fractional;
  
  float* a = textures_[index_int];
  float* b = textures_[index_int + (position == 1.0f ? 0 : 1)];
  
  if (feedback >= 0.5f) {
    feedback = 2.0f * (feedback - 0.5f);
    if (feedback < 0.5f) {
      gain_a *= 1.0f - feedback;
      gain_b *= 1.0f - feedback;
      for (int32_t i = 0; i < size_; ++i) {
        float x = *xf_polar++;
        a[i] = Crossfade(a[i], x, gain_a);
        b[i] = Crossfade(b[i], x, gain_b);
      }
    } else {
      float t = (feedback - 0.5f) * 0.7f + 0.5f;
      float gain_new = t - 0.5f;
      gain_new = gain_new * gain_new * 2.0f + 0.5f;
      float gain_new_a = gain_a * gain_new;
      float gain_new_b = gain_b * gain_new;
      float gain_old_a = 1.0f - gain_a * (1.0f - t);
      float gain_old_b = 1.0f - gain_b * (1.0f - t);
      for (int32_t i = 0; i < size_; ++i) {
        float x = *xf_polar++;
        a[i] = a[i] * gain_old_a + x * gain_new_a;
        b[i] = b[i] * gain_old_b + x * gain_new_b;
      }
    }
  } else {
    feedback *= 2.0f;
    feedback *= feedback;
    uint16_t threshold = feedback * 65535.0f;
    for (int32_t i = 0; i < size_; ++i) {
      float x = *xf_polar++;
      float gain = static_cast<uint16_t>(Random::GetSample()) <= threshold
          ? 1.0f : 0.0f;
      a[i] = Crossfade(a[i], x, gain_a * gain);
      b[i] = Crossfade(b[i], x, gain_b * gain);
    }
  }
}

void FrameTransformation::ReplayMagnitudes(float* xf_polar, float position) {
  float index_float = position * float(num_textures_ - 1);
  int32_t index_int = static_cast<int32_t>(index_float);
  float index_fractional = index_float - static_cast<float>(index_int);
  float* a = textures_[index_int];
  float* b = textures_[index_int + (position == 1.0f ? 0 : 1)];
  for (int32_t i = 0; i < size_; ++i) {
    xf_polar[i] = Crossfade(a[i], b[i], index_fractional);
  }
}

}  // namespace clouds
