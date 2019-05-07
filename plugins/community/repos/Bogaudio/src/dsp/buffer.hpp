#pragma once

#include "assert.h"
#include "math.h"
#include <algorithm>

namespace bogaudio {
namespace dsp {

template<typename T>
struct OverlappingBuffer {
	const int _size;
	const int _overlap;
	const bool _autoProcess;
	const int _overlapN;
	const int _samplesN;
	T* _samples;
	int _sample;

	OverlappingBuffer(int size, int o, bool autoProcess = true)
	: _size(size)
	, _overlap(o)
	, _autoProcess(autoProcess)
	, _overlapN(_size / _overlap)
	, _samplesN(2*_size - _overlapN)
	, _samples(new T[_samplesN])
	, _sample(0)
	{
		assert(_size > 0);
		assert(_overlap > 0 && _overlap <= _size && _size % _overlap == 0);
	}
	virtual ~OverlappingBuffer() {
		delete[] _samples;
	}

	inline void process() { processBuffer(_samples + _sample - _size); }
	virtual void processBuffer(T* samples) = 0;
	void postProcess() {
		if (_overlap == 1) {
			_sample = 0;
		}
		else if (_sample == _samplesN) {
			std::copy(_samples + _size, _samples + _samplesN, _samples);
			_sample = _samplesN - _size;
		}
	}

	virtual bool step(T sample) {
		_samples[_sample++] = sample;
		assert(_sample <= _samplesN);

		if (_sample >= _size && _sample % _overlapN == 0) {
			if (_autoProcess) {
				process();
				postProcess();
			}
			return true;
		}
		return false;
	}
};


template<typename T>
struct AveragingBuffer {
	const int _size;
	const int _framesN;
	const float _inverseFramesN;
	T* _sums;
	T* _averages;
	T* _frames;
	int _currentFrame;
	const int _resetsPerCommit;
	int _currentReset;

	AveragingBuffer(
		int size,
		int framesToAverage
	)
	: _size(size)
	, _framesN(framesToAverage)
	, _inverseFramesN(1.0 / (float)_framesN)
	, _sums(new T[_size] {})
	, _averages(new T[_size] {})
	, _frames(new T[_size * _framesN] {})
	, _currentFrame(0)
	, _resetsPerCommit(std::max(_size / 100, 10))
	, _currentReset(0)
	{
		assert(framesToAverage > 0);
	}
	~AveragingBuffer() {
		delete[] _sums;
		delete[] _averages;
		delete[] _frames;
	}

	T* getInputFrame() {
		float* frame = _frames + _currentFrame*_size;
		for (int i = 0; i < _size; ++i) {
			_sums[i] -= frame[i];
		}
		return frame;
	}

	void commitInputFrame() {
		float* frame = _frames + _currentFrame*_size;
		for (int i = 0; i < _size; ++i) {
			_sums[i] += frame[i];
			_averages[i] = _sums[i] * _inverseFramesN;
		}

		// Reset the average for some bins, such that reset overhead is even between calls -- avoids buildup of floating point error.
		for (int i = 0; i < _resetsPerCommit; ++i) {
			_sums[_currentReset] = 0.0;
			for (int j = 0; j < _framesN; ++j) {
				_sums[_currentReset] += _frames[j*_size + _currentReset];
			}
			_currentReset = (_currentReset + 1) % _size;
		}

		_currentFrame = (_currentFrame + 1) % _framesN;
	}

	const T* getAverages() {
		return _averages;
	}
};

template<typename T>
struct HistoryBuffer {
	int _size, _i;
	T* _buf;

	HistoryBuffer(int size, T initialValue)
	: _size(size)
	, _i(0)
	{
		assert(size > 0);
		_buf = new T[size];
		std::fill(_buf, _buf + size, initialValue);
	}
	~HistoryBuffer() {
		delete[] _buf;
	}

	inline void push(T s) {
		++_i;
		_i %= _size;
		_buf[_i] = s;
	}

	inline T value(int i) {
		assert(i >= 0 && i < _size);
		int j = _i - i;
		if (j < 0) {
			j += _size;
		}
		return _buf[j];
	}
};

} // namespace dsp
} // namespace bogaudio
