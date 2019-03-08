#ifndef DSJ_LFO_HPP
#define DSJ_LFO_HPP
#include <vector>
#include <cmath>

class LFO {
public:
    double output = 0.0;
    double phase = 0.0;

    LFO() {
        _frequency = 1.0;
        _sampleRate = 44100.0;
        _stepSize = _frequency * (double)kTableLength / _sampleRate;
        for(auto i = 0; i < kTableLength; ++i) {
            _sine.push_back(sin(2.0 * M_PI * (double)i / (double)kTableLength));
        }
        _phasor = 0.0;
    }

    double process() {
        _plusPhase = _phasor + phase * kTableLength;
        if(_plusPhase < 0.0) {
            _plusPhase += kTableLength;
        }
        else if(_plusPhase >= kTableLength) {
            _plusPhase -= kTableLength;
        }

        _a = (long)_plusPhase;
        _frac = _plusPhase - _a;
        _b = _a + 1.0;
        _b %= kTableLength;
        output = _sine[_a] * (1.0 - _frac) + _sine[_b] * _frac;

        _phasor += _stepSize;
        if(_phasor >= kTableLength) {
            _phasor -= kTableLength;
        }
        return output;
    }

    void setFrequency(double frequency) {
        _frequency = frequency;
        calcStepSize();
    }
    void setSamplerate(double sampleRate) {
        _sampleRate = sampleRate;
        calcStepSize();
    }
private:
    double _frequency;
    double _sampleRate;
    double _stepSize;
    double _phasor;
    double _plusPhase;
    long _a, _b;
    double _frac;
    const long kTableLength = 4096;
    std::vector<double> _sine;

    void calcStepSize() {
        _stepSize = _frequency * (double)kTableLength / _sampleRate;
    }
};

class TriSawLFO {
public:
    TriSawLFO() {
        phase = 0.0;
        _output = 0.0;
        _sampleRate = 44100.0;
        _step = 0.0;
        _rising = true;
        setFrequency(1.0);
        setRevPoint(0.5);
    }

    double process() {
        if(_step > 1.0) {
            _step -= 1.0;
            _rising = true;
        }

        if(_step >= _revPoint) {
            _rising = false;
        }

        if(_rising) {
            _output = _step * _riseRate;
        }
        else {
            _output = _step * _fallRate - _fallRate;
        }

        _step += _stepSize;
        _output *= 2.0;
        _output -= 1.0;
        return _output;
    }

    void setFrequency(double frequency) {
        _frequency = frequency;
        calcStepSize();
    }

    void setRevPoint(double revPoint) {
        _revPoint = revPoint;
        if(_revPoint < 0.0001) {
            _revPoint = 0.0001;
        }
        if(_revPoint > 0.999) {
            _revPoint = 0.999;
        }

        _riseRate = 1.0 / _revPoint;
        _fallRate = -1.0 / (1.0 - _revPoint);
    }

    void setSamplerate(double sampleRate) {
        _sampleRate = sampleRate;
        calcStepSize();
    }

    double getOutput() const {
        return _output;
    }

    double phase;

private:
    double _output;
    double _sampleRate;
    double _frequency;
    double _revPoint;
    double _riseRate;
    double _fallRate;
    double _step;
    double _stepSize;
    bool _rising;

    void calcStepSize() {
        _stepSize = _frequency / _sampleRate;
    }
};

#endif
