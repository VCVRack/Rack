//
//  VecOTAFilter.hpp
//
//  Created by Dale Johnson on 13/02/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
//

#ifndef VEC_OTA_FILTER_HPP
#define VEC_OTA_FILTER_HPP

#include <cmath>
#define G_TABLE_SIZE 1100000
#define TANH_TABLE_SIZE 8192
#include "./SIMDUtilities.hpp"
#include "./VecNonLinear.hpp"
#include "../Utilities.hpp"
#include "../DSP/OTAFilter.hpp"
#include <iostream>

class VecTPTOnePoleStage {
public:
  VecTPTOnePoleStage();
  inline __m128 process(const __m128& in) {
    _v = _mm_mul_ps(_mm_sub_ps(vecDriveSignal(in, _ones), _z), _G);
    _out = vecDriveSignal(_mm_add_ps(_v, _z), _ones);
    _z = _mm_add_ps(_out, _v);
    return _out;
  }

  inline void calcG(const __m128& g) {
      _G = _mm_div_ps(g, _mm_add_ps(_ones, g));
  }

  inline void setG(const __m128& g) {
      _G = g;
  }

  void setSampleRate(float sampleRate);
  float getSampleRate() const;
  float getZ() const;
  __m128 _G;
  __m128 _z;

protected:
  float _sampleRate;

  __m128 _ones, _zeros;
  __m128 _out;
  __m128 _v;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class VecOTAFilter {
public:
  VecOTAFilter();
  ~VecOTAFilter();

  inline __m128 process(const __m128& in) {
      __sigma = _mm_mul_ps(__G3, _stage1._z);
      __sigma = _mm_add_ps(__sigma, _mm_mul_ps(__G2, _stage2._z));
      __sigma = _mm_add_ps(__sigma, _mm_mul_ps(__G, _stage3._z));
      __sigma = _mm_mul_ps(_mm_add_ps(__sigma, _stage4._z), __1_h);

      __u = _mm_mul_ps(in, _mm_set1_ps(0.5f));
      __u = _mm_sub_ps(__u, _mm_mul_ps(_mm_mul_ps(__k, vecDriveSignal(__sigma, __ones)), _mm_set1_ps(_1_tanhf)));
      __u = _mm_div_ps(__u, _mm_add_ps(__ones, _mm_mul_ps(__k, __gamma)));
      __lp1 = _stage1.process(__u);
      __lp2 = _stage2.process(__lp1);
      __lp3 = _stage3.process(__lp2);
      __lp4 = _stage4.process(__lp3);
      out = _mm_mul_ps(__lp1, __1p);
      out = _mm_add_ps(out, _mm_mul_ps(__lp2, __2p));
      out = _mm_add_ps(out, _mm_mul_ps(__lp3, __3p));
      out = _mm_add_ps(out, _mm_mul_ps(__lp4, __4p));
      return out;
  }

  void setSampleRate(float sampleRate);
  void setCutoff(const __m128& pitch);
  void setQ(const __m128& Q);

  inline void setNumPoles(int poles) {
      if(poles == _poles) {
          return;
      }
      _poles = poles;
      __1p = __zeros;
      __2p = __zeros;
      __3p = __zeros;
      __4p = __zeros;
      switch(_poles) {
          case 1 : __1p = __ones; break;
          case 2 : __2p = __ones; break;
          case 3 : __3p = __ones; break;
          case 4 : __4p = __ones; break;
      }
  }

  __m128 out;

protected:
  VecTPTOnePoleStage _stage1;
  VecTPTOnePoleStage _stage2;
  VecTPTOnePoleStage _stage3;
  VecTPTOnePoleStage _stage4;
  __m128 __k;

  __m128 __ones, __zeros;
  __m128 __pitch, __cutoff, __g , __h, __1_h;
  __m128 __G, __G2, __G3;
  __m128 __sigma, __gamma;
  __m128 __u, __lp1, __lp2, __lp3, __lp4;

  int _poles;
  __m128 __1p, __2p, __3p, __4p;

  __m128 __frac;
  __m128i __cutoffI;
  int32_t* _pos;
  float* _lowG;
  float* _highG;
  __m128 __lowG, __highG;

  float _1_tanhf;
  float _fourPole;

  float _kGTable[G_TABLE_SIZE];
  float _sampleRate;

  void calcInternalGTable();
};

#endif /* OTAFilter_hpp */
