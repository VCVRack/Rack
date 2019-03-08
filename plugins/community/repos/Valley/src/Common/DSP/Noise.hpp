#pragma once
#include <random>
#include <cmath>

class WhiteNoise {
public:
    WhiteNoise() :_rand(_seed()),
                  _uniform(-1.0, 1.0) {
        _value = 0.f;
    }

    inline float process() {
        _value = _uniform(_rand);
        return _value;
    }

    float getValue() const {
        return _value;
    }
private:
    std::random_device _seed;
    std::minstd_rand _rand;
    std::uniform_real_distribution<float> _uniform;
    float _value;
};

class PinkNoise {
public:
    PinkNoise() {
        setSampleRate(44100.f);
        _white = 0.0;
        _pink = 0.0;
    }

    inline float process() {
        _white = _whiteGen.process();
        _b[0] = _b[0] + (_a[0] * ((_white * 48.69991228070175) - _b[0]));
        _b[1] = _b[1] + (_a[1] * ((_white * 11.23890718562874) - _b[1]));
        _b[2] = _b[2] + (_a[2] * ((_white * 4.96296774193548) - _b[2]));
        _b[3] = _b[3] + (_a[3] * ((_white * 2.32573483146067) - _b[3]));
        _b[4] = _b[4] + (_a[4] * ((_white * 1.18433822222222) - _b[4]));
        _b[5] = -0.7616 * _b[5] - _white * 0.0168980;
        _pink = (_b[0] + _b[1] + _b[2] + _b[3] + _b[4] + _b[5] + _b[6] + _white * 0.5362);
        _b[6] = _white * 0.115926;
        _pink *= 0.15;
        return _pink;
    }

    void setSampleRate(double sampleRate) {
        _sampleRate = sampleRate;
        for(auto i = 0; i < 7; ++i) {
            _b[i] = 0.0;
        }
        _white = 0.0;
        _pink = 0.0;
        calcAValues();
    }

    double getValue() {
        return _pink;
    }

private:
    WhiteNoise _whiteGen;

    double _b[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double _a[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    double _alpha[5] = {M_PI * 2.0 * 8.00135734209627,
                        M_PI * 2.0 * 46.88548507044182,
                        M_PI * 2.0 * 217.61558695916962,
                        M_PI * 2.0 * 939.80665948455472,
                        M_PI * 2.0 * 3276.10128392439381};
    double _sampleRate, _white, _pink;

    void calcAValues() {
        for(auto i = 0; i < 5; ++i) {
            _a[i] = sin(_alpha[i] / _sampleRate);
        }
    }
};
