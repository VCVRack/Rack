#pragma once
#include <iostream>
#include <cmath>
#include "./OnePoleFilters.hpp"

class DOsc {
public:
    float _saw, _pulse, _pulsePhase;
    float _subSaw, _subPulse;
    float _pwm;

    enum Harmonic {
        SUB_2_OCTAVE_HARMONIC = 0,
        SUB_OCTAVE_HARMONIC,
        SUB_FIFTH_HARMONIC,
        ZEROETH_HARMONIC,
        FIFTH_HARMONIC,
        OCTAVE_HARMONIC,
        DOUBLE_OCTAVE_HARMONIC,
        NUM_HARMONICS
    };

    DOsc() {
        _sampleRate = 44100.f;
        _step = 0.f;
        setFrequency(100.f);
        setSubOctave(0);
        _subOffsetLevel = 0;
        _subOffsetDegree = 0;
        _subWidth = 0.5f;
        _saw = 0.f;
        _subSaw = 0.f;
        _pwm = 0.5f;
    }

    inline void process() {
        //// Fundamental Wave
        _saw = (_step * 2.f - 1.f) - PolyBLEP(_step, _stepSize);
        _pulse = _step + _pwm;
        xInt = (int)_pulse;
        _pulse -= (float)xInt;
        _pulse = 1.f - _pulse;
        _pulse = _saw + (_pulse * 2.f - 1.f) - PolyBLEP(_pulse, _stepSize);
        _pulse = _pulse + (_pwm - 0.5f) * 2.f;
        _saw = _sawHPF.process(_saw);
        _pulse = _pulseHPF.process(_pulse);

        //// Derive Sub Wave
        _sub1Step = (_step * _subScale + (float)_subOffsetLevel * _subOffsetDegree);// + 0.75f;
        xInt = (int)_sub1Step;
        _sub1Step -= (float)xInt;
        _subSaw = (_sub1Step * 2.f - 1.f) - PolyBLEP(_sub1Step, _subStepSize);

        _subPulse = _sub1Step + _subWidth;
        xInt = (int)_subPulse;
        _subPulse -= (float)xInt;
        _subPulse = 1.f - _subPulse;
        _subPulse = _subSaw + (_subPulse * 2.f - 1.f) - PolyBLEP(_subPulse, _subStepSize);
        _subPulse = _subPulse + (_subWidth - 0.5f) * 2.f;
        _subPulse = _subHPF.process(_subPulse);

        // Increment phasor
        _step += _stepSize;
        if(_step > 1.f) {
            _step -= 1.f;
            _subOffsetLevel++;
            _subOffsetLevel -= (_subOffsetLevel >= _subLimit) ? _subLimit : 0;
        }
    }

    inline void setFrequency(float f) {
        _freq = f;
        _calcStepSize();
    }

    void setSampleRate(float sampleRate) {
        _sampleRate = sampleRate;
        _calcStepSize();
        _sawHPF.setSampleRate(sampleRate);
        _pulseHPF.setSampleRate(sampleRate);
        _subHPF.setSampleRate(sampleRate);
    }

    void setSubWave(int subWave) {
        switch(subWave) {
            case 0:
                _subWidth = 0.f;
                break;
            case 1:
                _subWidth = 0.5;
                break;
            case 2:
                _subWidth = 0.75f;
                break;
            default:
                _subWidth = 0.f;
                break;
        }
    }

    void setSubOctave(int octave) {
        switch(octave) {
            case SUB_2_OCTAVE_HARMONIC:
                _subScale = 0.25f;
                _subLimit = 4;
                _subOffsetDegree = 0.25f;
                break;
            case SUB_OCTAVE_HARMONIC:
                _subScale = 0.5f;
                _subLimit = 2;
                _subOffsetDegree = 0.5f;
                break;
            case SUB_FIFTH_HARMONIC:
                _subScale = 0.75f;
                _subLimit = 4;
                _subOffsetDegree = 0.75;
                break;
            case ZEROETH_HARMONIC:
                _subScale = 1.f;
                _subLimit = 1;
                _subOffsetDegree = 0.f;
                break;
            /*case THIRD_HARMONIC:
                _subScale = 1.333333f;
                _subWidth = 0.5f;
                _subLimit = 3;
                _subOffsetDegree = 0.333333f;
                break;*/
            case FIFTH_HARMONIC:
                _subScale = 1.5f;
                _subLimit = 2;
                _subOffsetDegree = 0.5f;
                break;
            case OCTAVE_HARMONIC:
                _subScale = 2.f;
                _subLimit = 1;
                _subOffsetDegree = 0.f;
                break;
            case DOUBLE_OCTAVE_HARMONIC:
                _subScale = 4.f;
                _subLimit = 1;
                _subOffsetDegree = 0.f;
                break;
            default:
                _subScale = 0.5f;
                _subLimit = 2;
        }
        _subStepSize = _stepSize * _subScale;
    }
private:

    float _freq, _sampleRate;

    float _stepSize, _subStepSize, _subWidth;
    float _step, _sub1Step;
    float _subScale, _subOffsetDegree;
    int _subOffsetLevel, _subLimit;
    int xInt;
    DCBlocker _sawHPF, _pulseHPF, _subHPF;
    void _calcStepSize() {
        _stepSize = _freq / _sampleRate;
        _stepSize = _stepSize > 0.5f ? 0.5f : _stepSize;
        _subStepSize = _stepSize * _subScale;
    }

    inline float PolyBLEP(float t, float dt) {
        if(t < dt) {
            t /= dt;
            return t+t - t*t - 1.f;
        }
        else if(t > (1.f - dt)) {
            t = (t - 1.f) / dt;
            return t*t + t + t + 1.f;
        }
        return 0.f;
    }
};
