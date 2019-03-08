//
//  OTAFilter.hpp
//
//  Created by Dale Johnson on 13/02/2018.
//  Copyright © 2018 Dale Johnson. All rights reserved.
//

#ifndef OTA_FILTER_HPP
#define OTA_FILTER_HPP

#include <cmath>
#define G_TABLE_SIZE 1100000
#define TANH_TABLE_SIZE 8192
#include "../Utilities.hpp"
#include "NonLinear.hpp"
#include <iostream>

static float kGTable[G_TABLE_SIZE];
static float kTanhTable[TANH_TABLE_SIZE + 1];
void calcGTable(float sampleRate);
void calcTanhTable();
float lookUpTanhf(float x);

class TPTOnePoleStage {
public:
  TPTOnePoleStage();
  inline float process(float in) {
    _v = (tanhDriveSignal(in, 1.f) - _z) * _G;
    _out = tanhDriveSignal(_v + _z, 1.f);
    _z = _out + _v;
    return _out;
  }

  inline void calcG(float g) {
      _G = g / (1.f + g);
  }

  inline void setG(float g) {
      _G = g;
  }

  void setSampleRate(float sampleRate);
  float getSampleRate() const;
  float getZ() const;
  void setNLP(bool nlp);
  float _G;
  float _z;

protected:
  float _s;
  float _sampleRate;

  float _out;
  float _v;
  float _1_tanhf;
  bool _nlp;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class OTAFilter {
public:
  float out;
  OTAFilter();

  inline float process(float in) {
      _G2 = _G * _G;
      _G3 = _G2 * _G;
      _sigma = _G3 * _stage1._z;
      _sigma += _G2 * _stage2._z;
      _sigma += _G * _stage3._z;
      _sigma += _stage4._z;
      _sigma *= _1_h;

      //_u = (in * 0.5f - _k * lookUpTanhf(_sigma) * _1_tanhf) / (1.f + _k * _gamma);
      _u = (in * 0.5f - _k * tanhDriveSignal(_sigma, 1.f) * _1_tanhf) / (1.f + _k * _gamma);
      _lp1 = _stage1.process(_u);
      _lp2 = _stage2.process(_lp1);
      _lp3 = _stage3.process(_lp2);
      _lp4 = _stage4.process(_lp3);
      out = _lp4 * _fourPole + _lp2 * (1.f - _fourPole);
      return out;
  }

  void setSampleRate(float sampleRate);
  void setCutoff(float pitch);
  void setQ(float Q);
  void setNLP(bool nlp);
  inline void set4Pole(float isFourPole) {
      _fourPole = isFourPole;
  }
protected:
  TPTOnePoleStage _stage1;
  TPTOnePoleStage _stage2;
  TPTOnePoleStage _stage3;
  TPTOnePoleStage _stage4;
  float _k;

  float _pitch, _prevPitch, _cutoff, _g , _h, _1_h;
  float _G, _G2, _G3;
  float _sigma, _gamma;
  float _S1, _S2, _S3, _S4;
  float _u;
  float _lp1, _lp2, _lp3, _lp4;

  float _1_tanhf;
  bool _nlp;
  float _fourPole;
};

#endif /* OTAFilter_hpp */
