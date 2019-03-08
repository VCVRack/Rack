//
// This plate reverb is based upon Jon Dattorro's 1997 reverb algorithm.
//

#pragma once
#include "../Common/DSP/AllpassFilter.hpp"
#include "../Common/DSP/OnePoleFilters.hpp"
#include "../Common/DSP/LFO.hpp"
#include <iostream>

class Dattorro {
public:
    Dattorro();
    void process(double leftInput, double rightInput);
    void clear();
    void setTimeScale(double timeScale);
    void setPreDelay(double time);
    void setModShape(double shape);
    void setSampleRate(double sampleRate);
    void freeze();
    void unFreeze();

    double rightOut = 0.0;
    double leftOut = 0.0;
    double inputLowCut = 0.0;
    double inputHighCut = 10000.0;
    double reverbHighCut = 10000.0;
    double reverbLowCut = 0.0;
    double modDepth = 1.0;
    double inputDiffusion1 = 0.75;
    double inputDiffusion2 = 0.625;
    double plateDiffusion1 = 0.7;
    double plateDiffusion2 = 0.5;
    double decay = 0.9999;
    double modSpeed = 1.0;
    double diffuseInput = 0.0;
private:
    double _timeScale = 1.0;
    double _preDelayTime = 0.0;
    const long _kInApf1Time = 141;
    const long _kInApf2Time = 107;
    const long _kInApf3Time = 379;
    const long _kInApf4Time = 277;

    const long _kLeftApf1Time = 672;
    const long _kLeftDelay1Time = 4453;
    const long _kLeftApf2Time = 1800;
    const long _kLeftDelay2Time = 3720;

    const long _kRightApf1Time = 908;
    const long _kRightDelay1Time = 4217;
    const long _kRightApf2Time = 2656;
    const long _kRightDelay2Time = 3163;

    const long _kLeftTaps[7] = {266, 2974, 1913, 1996, 1990, 187, 1066};
    const long _kRightTaps[7] = {266, 2974, 1913, 1996, 1990, 187, 1066};
    long _scaledLeftTaps[7] = {0,0,0,0,0,0,0};
    long _scaledRightTaps[7] = {0,0,0,0,0,0,0};

    double _leftApf1Time = 0.0;
    double _leftApf2Time = 0.0;
    double _rightApf1Time = 0.0;
    double _rightApf2Time = 0.0;

    const double _kLfoExcursion = 16.0;
    double _lfoDepth = 0.0;
    double _lfo1Freq = 0.10;
    double _lfo2Freq = 0.150;
    double _lfo3Freq = 0.120;
    double _lfo4Freq = 0.180;

    const double _dattorroSampleRate = 29761.0;
    double _sampleRate = 44100.0;
    double _dattorroScaleFactor = _sampleRate / _dattorroSampleRate;
    double _decay = 0.00;
    double _leftSum = 0.0;
    double _rightSum = 0.0;
    bool _freeze = false;
    OnePoleHPFilter _leftInputDCBlock;
    OnePoleHPFilter _rightInputDCBlock;
    OnePoleLPFilter _inputLpf;
    OnePoleHPFilter _inputHpf;

    InterpDelay<double> _preDelay;

    AllpassFilter<double> _inApf1;
    AllpassFilter<double> _inApf2;
    AllpassFilter<double> _inApf3;
    AllpassFilter<double> _inApf4;
    double _tankFeed = 0.0;

    AllpassFilter<double> _leftApf1;
    InterpDelay<double> _leftDelay1;
    OnePoleLPFilter _leftFilter;
    OnePoleHPFilter _leftHpf;
    AllpassFilter<double> _leftApf2;
    InterpDelay<double> _leftDelay2;

    AllpassFilter<double> _rightApf1;
    InterpDelay<double> _rightDelay1;
    OnePoleLPFilter _rightFilter;
    OnePoleHPFilter _rightHpf;
    AllpassFilter<double> _rightApf2;
    InterpDelay<double> _rightDelay2;

    OnePoleHPFilter _leftOutDCBlock;
    OnePoleHPFilter _rightOutDCBlock;

    TriSawLFO _lfo1;
    TriSawLFO _lfo2;
    TriSawLFO _lfo3;
    TriSawLFO _lfo4;

    // Freeze Cross fade
    double _fade = 1.0;
    double _fadeTime = 0.002;
    double _fadeStep = 1.0 / (_fadeTime * _sampleRate);
    double _fadeDir = 1.0;

    double dattorroScale(double delayTime);
};
