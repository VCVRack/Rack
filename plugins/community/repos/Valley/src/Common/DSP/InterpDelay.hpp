#pragma once
#include <vector>
#include <cmath>
#include <pmmintrin.h>
#define MAX_DELAY_TAP_GROUPS 512
#define MAX_DELAY_LENGTH 65536

template<class T>
class InterpDelay
{
public:
    InterpDelay() {
        input = 0.0;
        output = 0.0;
        delayTime = 0.0;
        _length = 2;
        _buffer.assign(_length, 0.0);
        _readPos = 1.0;
        _phasedPos = 0.0;
        _lowerReadPos = 0;
        _upperReadPos = 0;
        _writePos = 0;
    }

    InterpDelay(long maxLength, long initDelay) {
        input = 0;
        output = 0;
        delayTime = initDelay;
        _length = maxLength;
        _buffer.assign(_length, 0);
        _readPos = maxLength - 1;
        _phasedPos = 0;
        _lowerReadPos = 0;
        _upperReadPos = 0;
        _writePos = 0;
    }

    T inline process() {
        _time = delayTime;
        if(_time < 0.0) {
            _time = 0.0;
        }
        else if(_time >= _length) {
            _time = _length - 1;
        }
        _buffer[_writePos] = input;
        _phasedPos = (T)_writePos - _time;
        if(_phasedPos < 0.0) {
            _phasedPos += (T)_length;
        }
        _lowerReadPos = (long)_phasedPos;
        _upperReadPos = _lowerReadPos + 1;
        if(_upperReadPos >= _length) {
            _upperReadPos -= _length;
        }
        _ratio = _phasedPos - _lowerReadPos;

        output = _buffer[_lowerReadPos] + _ratio * (_buffer[_upperReadPos] - _buffer[_lowerReadPos]);
        _writePos++;
        if(_writePos >= _length) {
            _writePos -= _length;
        }

        return output;
    }

    T tap(long i) const {
        i = _writePos - i;
        if(i < 0) {
            i += _length;
        }
        return _buffer[i];
    }

    void clear() {
        _buffer.assign(_length, 0);
        input = 0;
        output = 0;
    }

    void setMaxDelaySamples(long maxDelaySamples) {
        _length = maxDelaySamples;
        _buffer.assign(_length, 0);
    }

    T input;
    T delayTime;
    T output;
private:
    std::vector<T> _buffer;
    long _length;
    T _time;
    T _readPos;
    T _phasedPos;
    long _upperReadPos;
    long _lowerReadPos;
    T _ratio;
    long _writePos;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, const int kNumTaps>
class MultiTapInterpDelay {
public:
    MultiTapInterpDelay() {
        _length = 2;
        _buffer.assign(_length, 0);
        for(auto i = 0; i < kNumTaps; ++i) {
            _output[i] = 0;
            _time[i] = 0;
        }
        _readPos = 1.0;
        _phasedPos = 0.0;
        _lower = 0;
        _upper = 0;
        _writePos = 0;
    }

    MultiTapInterpDelay(long maxLength) {
        _length = maxLength;
        _buffer.assign(_length, 0);
        for(auto i = 0; i < kNumTaps; ++i) {
            _output[i] = 0;
            _time[i] = 0;
        }
        _readPos = 1.0;
        _phasedPos = 0.0;
        _lower = 0;
        _upper = 0;
        _writePos = 0;
    }

    void process(T input) {
        _buffer[_writePos] = input;
        for(auto i = 0; i < kNumTaps; ++i) {
            _phasedPos = (T)_writePos - _time[i];
            if(_phasedPos < 0) {
                _phasedPos += (T)_length;
            }
            _frac = _phasedPos - (long)_phasedPos;
            _lower = (long)_phasedPos;
            _upper = _lower + 1;
            if(_upper >= _length) {
                _upper -= _length;
            }
            _output[i] = _buffer[_lower] + _frac * (_buffer[_upper] - _buffer[_lower]);
        }
        _writePos++;
        if(_writePos >= _length) {
            _writePos -= _length;
        }
    }

    T operator[](const int i) const {
        return _output[i];
    }

    void setDelaySamples(int tap, T dSamples) {
        _time[tap] = dSamples;
    }

    void clear() {
        _buffer.assign(_length, 0);
        for(auto i = 0; i < kNumTaps; ++i) {
            _output[i] = 0;
        }
    }
private:
    std::vector<T> _buffer;
    long _length;
    T _output[kNumTaps];
    T _time[kNumTaps];
    long _readPos;
    T _phasedPos;
    long _upper;
    long _lower;
    T _frac;
    long _writePos;
};
