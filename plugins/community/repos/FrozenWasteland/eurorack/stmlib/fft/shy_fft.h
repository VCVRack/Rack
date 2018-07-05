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
// A modest implementation of real FFT for embedded audio applications,
// largely based on Laurent de Soras' FFTReal.
//
// Improvements:
// * No dynamic allocations.
// * No additional buffering (can use the input buffer as a workspace).
// * No big bitrev lookup table.
// * Keep the fixed size template signature, but also provide method for
//   variable size (up to the fixed size).

#ifndef STMLIB_FFT_SHY_FFT_H_
#define STMLIB_FFT_SHY_FFT_H_

#include "stmlib/stmlib.h"

#include <algorithm>
#include <cmath>

namespace stmlib {

// Compile-time log 2
template<size_t x>
struct Log2 {
  enum {
    value = Log2<x / 2>::value + 1
  };
  typedef char CheckPowerOf2[((x & (x - 1)) == 0 ? 1 : -1)];
};

template<>
struct Log2<1> { enum { value = 0 }; };


// Bit reversal LUT size.
template<size_t> struct BitReversalLut { enum { size = 1 }; };
template<> struct BitReversalLut<3> { enum { size = 2 }; };
template<> struct BitReversalLut<4> { enum { size = 4 }; };
template<> struct BitReversalLut<5> { enum { size = 8 }; };
template<> struct BitReversalLut<6> { enum { size = 16 }; };
template<> struct BitReversalLut<7> { enum { size = 32 }; };
template<> struct BitReversalLut<8> { enum { size = 64 }; };


// Typed math functions and constants.
template<typename T>
struct Math {
  inline T pi() const;
  inline T sqrt_2_div_2() const;
  inline T cos(T x);
  inline T sin(T x);
};

template<>
struct Math<float> {
  inline float pi() const { return 3.141592653589793f; }
  inline float sqrt_2_div_2() const { return 0.7071067811865476f; }
  inline float cos(float x) { return cosf(x); }
  inline float sin(float x) { return sinf(x); }
};

template<>
struct Math<double> {
  inline double pi() const { return 3.141592653589793; }
  inline float sqrt_2_div_2() const { return 0.7071067811865476; }
  inline double cos(double x) { return cos(x); }
  inline double sin(double x) { return sin(x); }
};


// Look-up table for trigonometric data.
template<typename T, size_t num_passes>
class LutPhasor {
 public:
  LutPhasor() { }
  ~LutPhasor() { }
  
  void Init() {
    Math<T> math;
  
    for (size_t pass = 3; pass < num_passes; ++pass) {
      size_t pass_size = 1L << (pass - 1);
      T* pass_ptr = &trig_lut_[(1L << (pass - 1)) - 4];
      T increment = math.pi() / (pass_size << 1);
      T phase = 0.0;
      for (size_t i = 0; i < pass_size; ++i) {
        pass_ptr[i] = math.cos(phase);
        phase += increment;
      }
    }
  }
  
  inline void Start(size_t pass) {
    size_t pass_size = 1 << (pass - 1);
    cos_ptr_ = &trig_lut_[pass_size - 4 + 1];
    sin_ptr_ = &trig_lut_[pass_size + pass_size - 4 - 1];
  }
  
  inline void Rotate() {
    ++cos_ptr_;
    --sin_ptr_;
  }
  
  inline T cos() const { return *cos_ptr_; }
  inline T sin() const { return *sin_ptr_; }
  
 private:
  T trig_lut_[(1 << (num_passes - 1)) - 4];
  T* cos_ptr_;
  T* sin_ptr_;
  
  DISALLOW_COPY_AND_ASSIGN(LutPhasor);
};

template<typename T> struct LutPhasor<T, 0> { void Init() { }; };
template<typename T> struct LutPhasor<T, 1> { void Init() { }; };
template<typename T> struct LutPhasor<T, 2> { void Init() { }; };

template<typename T>
struct LutPhasor<T, 3> {
  void Init() { };
  void Start(size_t) { };
  void Rotate() { };
  inline T cos() const { return 1.0; }
  inline T sin() const { return 0.0; }
};


// Another way of generating roots of unity.
template<typename T, size_t num_passes>
class RotationPhasor {
 public:
  RotationPhasor() { }
  ~RotationPhasor() { }
  
  void Init() {
    Math<T> math;
    for (size_t pass = 3; pass < num_passes; ++pass) {
      size_t index = (pass - 3) << 1;
      T angle = math.pi() / (1 << pass);
      sin_cos_lut_[index] = math.cos(angle);
      sin_cos_lut_[index + 1] = math.sin(angle);
    }
  }
  
  inline void Start(size_t pass) {
    size_t index = (pass - 3) << 1;
    cos_ = real_ = sin_cos_lut_[index];
    sin_ = imag_ = sin_cos_lut_[index + 1];
  }
  
  inline void Rotate() {
    T temp = cos_ * real_ - sin_ * imag_;
    sin_ = cos_ * imag_ + sin_ * real_;
    cos_ = temp;
  }
  
  inline T cos() const { return cos_; }
  inline T sin() const { return sin_; }
  
 private:
  T sin_cos_lut_[(num_passes - 3) << 1];
  T cos_;
  T sin_;
  T real_;
  T imag_;
  
  DISALLOW_COPY_AND_ASSIGN(RotationPhasor);
};

template<typename T> struct RotationPhasor<T, 0> { void Init() { }; };
template<typename T> struct RotationPhasor<T, 1> { void Init() { }; };
template<typename T> struct RotationPhasor<T, 2> { void Init() { }; };

template<typename T>
struct RotationPhasor<T, 3> {
  void Init() { };
  void Start(size_t) { };
  void Rotate() { };
  inline T cos() const { return 1.0; }
  inline T sin() const { return 0.0; }
};

// Direct transform
template<typename T, size_t num_passes, typename Phasor>
struct DirectTransform {
 private:
  enum {
    size = 1 << num_passes
  };
  
 public:
  void operator()(
      T* input,
      T* output,
      const uint8_t* bit_rev,
      Phasor* phasor) {
    T* s;
    T* d;
    Math<T> math;
    
    // First and second pass.
    d = output;
    for (size_t i = 0; i < size; i += 4) {
      const T* s = input;
      size_t r0 = num_passes <= 8
          ? bit_rev[i >> 2]
          : ((bit_rev[i & 0xff] << 8) | bit_rev[i >> 8]) >> (16 - num_passes);
      size_t r1 = r0 + 2 * (size >> 2);
      size_t r2 = r0 + 1 * (size >> 2);
      size_t r3 = r0 + 3 * (size >> 2);
      
      d[1] = s[r0] - s[r1];
      d[3] = s[r2] - s[r3];
      T a = s[r0] + s[r1];
      T b = s[r2] + s[r3];
      d[0] = a + b;
      d[2] = a - b;
      d += 4;
    }
    
    // Third pass.
    s = output;
    d = input;
    for (size_t i = 0; i < size; i += 8) {
      T v;

      d[i] = s[i] + s[i + 4];
      d[i + 4] = s[i] - s[i + 4];
      d[i + 2] = s[i + 2];
      d[i + 6] = s[i + 6];

      v = (s[i + 5] - s[i + 7]) * math.sqrt_2_div_2();
      d[i + 1] = s[i + 1] + v;
      d[i + 3] = s[i + 1] - v;

      v = (s[i + 5] + s[i + 7]) * math.sqrt_2_div_2();
      d[i + 5] = v + s[i + 3];
      d[i + 7] = v - s[i + 3];
    }
    
    // Remaining passes.
    for (size_t pass = 3; pass < num_passes; ++pass) {
      // Flip source and destination pointers
      {
        T* tmp = s;
        s = d;
        d = tmp;
      }
      
      size_t n = 1 << pass;
      size_t n_2 = n >> 1;

      for (size_t i = 0; i < size; i += (n << 1)) {
        T* s1r = s + i;
        T* s2r = s1r + n;
        T* dr = d + i;
        T* di = dr + n;

        dr[0] = s1r[0] + s2r[0];
        di[0] = s1r[0] - s2r[0];
        dr[n_2] = s1r[n_2];
        di[n_2] = s2r[n_2];
        T* s1i = s1r + n_2;
        T* s2i = s1i + n;
        phasor->Start(pass);
        for (size_t j = 1; j < n_2; ++j) {
          T c = phasor->cos();
          T s = phasor->sin();
          T v;

          v = s2r[j] * c - s2i[j] * s;
          dr[j] = s1r[j] + v;
          di[-j] = s1r[j] - v;

          v = s2r[j] * s + s2i[j] * c;
          di[j] = v + s1i[j];
          di[n - j] = v - s1i[j];
          phasor->Rotate();
        }
      }
    }
    
    // Annoying additional data copy step.
    if (d != output) {
      std::copy(&d[0], &d[size], &output[0]);
    }
  }
  
  // The exact same thing but with "num_passes" as a run-time argument.
  void operator()(
      T* input,
      T* output,
      const uint8_t* bit_rev,
      Phasor* phasor,
      size_t rt_num_passes) {
    T* s;
    T* d;
    Math<T> math;
    size_t rt_size = 1 << rt_num_passes;
    // First and second pass.
    d = output;
    for (size_t i = 0; i < rt_size; i += 4) {
      const T* s = input;
      size_t r0 = \
          ((bit_rev[i & 0xff] << 8) | bit_rev[i >> 8]) >> (16 - rt_num_passes);
      size_t r1 = r0 + 2 * (rt_size >> 2);
      size_t r2 = r0 + 1 * (rt_size >> 2);
      size_t r3 = r0 + 3 * (rt_size >> 2);
      
      d[1] = s[r0] - s[r1];
      d[3] = s[r2] - s[r3];
      T a = s[r0] + s[r1];
      T b = s[r2] + s[r3];
      d[0] = a + b;
      d[2] = a - b;
      d += 4;
    }
    
    // Third pass.
    s = output;
    d = input;
    for (size_t i = 0; i < rt_size; i += 8) {
      T v;

      d[i] = s[i] + s[i + 4];
      d[i + 4] = s[i] - s[i + 4];
      d[i + 2] = s[i + 2];
      d[i + 6] = s[i + 6];

      v = (s[i + 5] - s[i + 7]) * math.sqrt_2_div_2();
      d[i + 1] = s[i + 1] + v;
      d[i + 3] = s[i + 1] - v;

      v = (s[i + 5] + s[i + 7]) * math.sqrt_2_div_2();
      d[i + 5] = v + s[i + 3];
      d[i + 7] = v - s[i + 3];
    }
    
    // Remaining passes.
    for (size_t pass = 3; pass < rt_num_passes; ++pass) {
      // Flip source and destination pointers
      {
        T* tmp = s;
        s = d;
        d = tmp;
      }
      
      size_t n = 1 << pass;
      size_t n_2 = n >> 1;

      for (size_t i = 0; i < rt_size; i += (n << 1)) {
        T* s1r = s + i;
        T* s2r = s1r + n;
        T* dr = d + i;
        T* di = dr + n;

        dr[0] = s1r[0] + s2r[0];
        di[0] = s1r[0] - s2r[0];
        dr[n_2] = s1r[n_2];
        di[n_2] = s2r[n_2];
        T* s1i = s1r + n_2;
        T* s2i = s1i + n;
        phasor->Start(pass);
        for (size_t j = 1; j < n_2; ++j) {
          T c = phasor->cos();
          T s = phasor->sin();
          T v;

          v = s2r[j] * c - s2i[j] * s;
          dr[j] = s1r[j] + v;
          di[-j] = s1r[j] - v;

          v = s2r[j] * s + s2i[j] * c;
          di[j] = v + s1i[j];
          di[n - j] = v - s1i[j];
          phasor->Rotate();
        }
      }
    }
    
    // Annoying additional data copy step.
    if (d != output) {
      std::copy(&d[0], &d[rt_size], &output[0]);
    }
  }
};

template<typename T, typename Phasor>
struct DirectTransform<T, 0, Phasor> {
  void operator()(const T* i, T* o, T*, const uint8_t*, Phasor*) {
    o[0] = i[0];
  }
};

template<typename T, typename Phasor>
struct DirectTransform<T, 1, Phasor> {
  void operator()(const T* i, T* o, T*, const uint8_t*, Phasor*) {
    o[0] = i[0] + i[1];
    o[1] = i[0] - i[1];
  }
};

template<typename T, typename Phasor>
struct DirectTransform<T, 2, Phasor> {
  void operator()(const T* i, T* o, T*, const uint8_t*, Phasor*) {
    o[1] = i[0] - i[2];
    o[3] = i[1] - i[3];
    T a = i[0] + i[2];
    T b = i[1] + i[3];
    o[0] = a + b;
    o[2] = a - b;
  }
};


// Inverse transform
template<typename T, size_t num_passes, typename Phasor>
struct InverseTransform {
 private:
  enum {
    size = 1 << num_passes
  };
  
 public:
  void operator()(
      T* input,
      T* output,
      const uint8_t* bit_rev,
      Phasor* phasor) {
    T* s = (T*)(input);
    T* d = output;
    Math<T> math;
    
    // Remaining passes.
    for (size_t pass = num_passes - 1; pass >= 3; --pass) {
      size_t n = 1 << pass;
      size_t n_2 = n >> 1;
      
      for (size_t i = 0; i < size; i += (n << 1)) {
        T* sr = s + i;
        T* si = sr + n;
        T* d1r = d + i;
        T* d2r = d1r + n;
        
        d1r[0] = sr[0] + si[0];
        d2r[0] = sr[0] - si[0];
        d1r[n_2] = sr[n_2] * T(2);
        d2r[n_2] = si[n_2] * T(2);
      
        T* d1i = d1r + n_2;
        T* d2i = d1i + n;
        phasor->Start(pass);
        for (size_t j = 1; j < n_2; ++j) {
          d1r[j] = sr[j] + si[-j];
          d1i[j] = si[j] - si[n - j];
          
          T c = phasor->cos();
          T s = phasor->sin();
          T vr = sr[j] - si[-j];
          T vi = si[j] + si[n - j];
          
          d2r[j] = vr * c + vi * s;
          d2i[j] = vi * c - vr * s;
          phasor->Rotate();
        }
      }

      // Flip source and destination pointers for the next pass.
      if (d == output) {
        s = output;
        d = input;
      } else {
        s = input;
        d = output;
      }
    }
    
    // Copy data if necessary.
    if (d == output) {
      std::copy(&s[0], &s[size], &output[0]);
    }
    
    s = output;
    d = input;
    for (size_t i = 0; i < size; i += 8) {
      T vr, vi;
      d[i] = s[i] + s[i + 4];
      d[i + 4] = s[i] - s[i + 4];
      d[i + 2] = s[i + 2] * T(2);
      d[i + 6] = s[i + 6] * T(2);
      d[i + 1] = s[i + 1] + s[i + 3];
      d[i + 3] = s[i + 5] - s[i + 7];
      vr = s[i + 1] - s[i + 3];
      vi = s[i + 5] + s[i + 7];
      d[i + 5] = (vr + vi) * math.sqrt_2_div_2();
      d[i + 7] = (vi - vr) * math.sqrt_2_div_2();
    }
    
    // First and second pass.
    s = input;
    d = output;
    for (size_t i = 0; i < size; i += 4) {
      size_t r0 = num_passes <= 8
          ? bit_rev[i >> 2]
          : ((bit_rev[i & 0xff] << 8) | bit_rev[i >> 8]) >> (16 - num_passes);
      size_t r1 = r0 + 2 * (size >> 2);
      size_t r2 = r0 + 1 * (size >> 2);
      size_t r3 = r0 + 3 * (size >> 2);
      
      T b_0 = s[0] + s[2];
      T b_2 = s[0] - s[2];
      T b_1 = s[1] * T(2);
      T b_3 = s[3] * T(2);
      
      d[r0] = b_0 + b_1;
      d[r1] = b_0 - b_1;
      d[r2] = b_2 + b_3;
      d[r3] = b_2 - b_3;
      s += 4;
    }
  }
  
  void operator()(
      T* input,
      T* output,
      const uint8_t* bit_rev,
      Phasor* phasor,
      size_t rt_num_passes) {
    T* s = (T*)(input);
    T* d = output;
    Math<T> math;
    
    size_t rt_size = 1 << rt_num_passes;
    
    // Remaining passes.
    for (size_t pass = rt_num_passes - 1; pass >= 3; --pass) {
      size_t n = 1 << pass;
      size_t n_2 = n >> 1;
      
      for (size_t i = 0; i < rt_size; i += (n << 1)) {
        T* sr = s + i;
        T* si = sr + n;
        T* d1r = d + i;
        T* d2r = d1r + n;
        
        d1r[0] = sr[0] + si[0];
        d2r[0] = sr[0] - si[0];
        d1r[n_2] = sr[n_2] * T(2);
        d2r[n_2] = si[n_2] * T(2);
      
        T* d1i = d1r + n_2;
        T* d2i = d1i + n;
        phasor->Start(pass);
        for (size_t j = 1; j < n_2; ++j) {
          d1r[j] = sr[j] + si[-j];
          d1i[j] = si[j] - si[n - j];
          
          T c = phasor->cos();
          T s = phasor->sin();
          T vr = sr[j] - si[-j];
          T vi = si[j] + si[n - j];
          
          d2r[j] = vr * c + vi * s;
          d2i[j] = vi * c - vr * s;
          phasor->Rotate();
        }
      }

      // Flip source and destination pointers for the next pass.
      if (d == output) {
        s = output;
        d = input;
      } else {
        s = input;
        d = output;
      }
    }
    
    // Copy data if necessary.
    if (d == output) {
      std::copy(&s[0], &s[rt_size], &output[0]);
    }
    
    s = output;
    d = input;
    for (size_t i = 0; i < rt_size; i += 8) {
      T vr, vi;
      d[i] = s[i] + s[i + 4];
      d[i + 4] = s[i] - s[i + 4];
      d[i + 2] = s[i + 2] * T(2);
      d[i + 6] = s[i + 6] * T(2);
      d[i + 1] = s[i + 1] + s[i + 3];
      d[i + 3] = s[i + 5] - s[i + 7];
      vr = s[i + 1] - s[i + 3];
      vi = s[i + 5] + s[i + 7];
      d[i + 5] = (vr + vi) * math.sqrt_2_div_2();
      d[i + 7] = (vi - vr) * math.sqrt_2_div_2();
    }
    
    // First and second pass.
    s = input;
    d = output;
    for (size_t i = 0; i < rt_size; i += 4) {
      size_t r0 = \
            ((bit_rev[i & 0xff] << 8) | bit_rev[i >> 8]) >> (16 - rt_num_passes);
      size_t r1 = r0 + 2 * (rt_size >> 2);
      size_t r2 = r0 + 1 * (rt_size >> 2);
      size_t r3 = r0 + 3 * (rt_size >> 2);
      
      T b_0 = s[0] + s[2];
      T b_2 = s[0] - s[2];
      T b_1 = s[1] * T(2);
      T b_3 = s[3] * T(2);
      
      d[r0] = b_0 + b_1;
      d[r1] = b_0 - b_1;
      d[r2] = b_2 + b_3;
      d[r3] = b_2 - b_3;
      s += 4;
    }
  }
};

template<typename T, typename Phasor>
struct InverseTransform<T, 0, Phasor> {
  void operator()(const T* i, T* o, T*, const uint8_t*, Phasor*) {
    o[0] = i[0];
  }
};

template<typename T, typename Phasor>
struct InverseTransform<T, 1, Phasor> {
  void operator()(const T* i, T* o, T*, const uint8_t*, Phasor*) {
    o[0] = i[0] + i[1];
    o[1] = i[0] - i[1];
  }
};

template<typename T, typename Phasor>
struct InverseTransform<T, 2, Phasor> {
  void operator()(const T* i, T* o, T*, const uint8_t*, Phasor*) {
    T a = i[0] + i[2];
    T b = i[0] - i[2];
    
    o[0] = a + i[1] * T(2);
    o[2] = a - i[1] * T(2);
    o[1] = b + i[3] * T(2);
    o[3] = b - i[3] * T(2);
  }
};


template<
    typename T=float,
    size_t size=16,
    template <typename, size_t> class Phasor=LutPhasor>
class ShyFFT {
 public:
  enum {
    num_passes = Log2<size>::value,
    max_size = size
  };

 private:
  typedef Phasor<T, num_passes> PhasorType;

 public:
  ShyFFT() { }
  ~ShyFFT() { }
  
  void Init() {
    bit_rev_[0] = 0;
    for (size_t i = 1; i < sizeof(bit_rev_); ++i) {
      uint8_t byte = 0;
      uint8_t source = i << 2;
      uint8_t destination = static_cast<uint8_t>(size >> 1);
      while (source) {
        if (source & 1) {
          byte |= destination;
        }
        destination >>= 1;
        source >>= 1;
      }
      bit_rev_[i] = byte;
    }
    phasor_.Init();
  }
  
  void Direct(T* input, T* output) {
    DirectTransform<T, num_passes, Phasor<T, num_passes> > d;
    d(
        input,
        output,
        num_passes <= 8 ? &bit_rev_[0] : bit_rev_256_lut_,
        &phasor_);
  }
  
  void Inverse(T* input, T* output) {
    InverseTransform<T, num_passes, Phasor<T, num_passes> > i;
    i(
        input,
        output,
        num_passes <= 8 ? &bit_rev_[0] : bit_rev_256_lut_,
        &phasor_);
  }
  
  void Direct(T* input, T* output, size_t n) {
    DirectTransform<T, num_passes, Phasor<T, num_passes> > d;
    d(
        input,
        output,
        bit_rev_256_lut_,
        &phasor_,
        n);
  }
  
  void Inverse(T* input, T* output, size_t n) {
    InverseTransform<T, num_passes, Phasor<T, num_passes> > i;
    i(
        input,
        output,
        bit_rev_256_lut_,
        &phasor_,
        n);
  }
  

 private:
  PhasorType phasor_;
  uint8_t bit_rev_[BitReversalLut<num_passes>::size];
  static const uint8_t bit_rev_256_lut_[256];

  DISALLOW_COPY_AND_ASSIGN(ShyFFT);
};

template<typename T, size_t size, template <typename, size_t> class Phasor>
const uint8_t ShyFFT<T, size, Phasor>::bit_rev_256_lut_[256] = {
#define R2(n) n, n + 2*64, n + 1*64, n + 3*64
#define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
R6(0), R6(2), R6(1), R6(3)
};

}  // namespace stmlib

#endif  // STMLIB_FFT_SHY_FFT_H_
