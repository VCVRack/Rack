#include "FreqLUT.hpp"

FreqLUT::FreqLUT() {
    _resolution = 100000.f;
    _pitch = 0;
    _pos = 0;
    _inputLow = -5.f;
    _inputHigh = 6.f;
    makeLUT();
}

float FreqLUT::getFrequency(float pitch) {
    _pitch = pitch * _resolution + (5.f * _resolution);
    if(_pitch > _lut.size() - 2) {
        _pitch = _lut.size() - 2;
    }
    if(_pitch < 0) {
        _pitch = 0;
    }
    _pos = (long)_pitch;
    _frac = _pitch - (float)_pos;
    return _lut[_pos] + _frac * (_lut[_pos + 1] - _lut[_pos]);
}

void FreqLUT::makeLUT() {
    float startPitch = _inputLow * _resolution;
    float endPitch = _inputHigh * _resolution;

    _lut.clear();
    for(float i = startPitch; i <= endPitch; i += 1.f) {
        _lut.push_back(261.6255f * powf(2.f, i / _resolution));
    }
}
