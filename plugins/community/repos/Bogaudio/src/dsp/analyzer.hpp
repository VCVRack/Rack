#pragma once

#include "assert.h"
#include <math.h>

#include "ffft/FFTReal.h"

#include "buffer.hpp"

namespace bogaudio {
namespace dsp {

struct Window {
	int _size;
	float* _window;
	float _sum;

	Window(int size)
	: _size(size)
	, _window(new float[_size] {})
	, _sum(0.0)
	{}
	virtual ~Window() {
		delete[] _window;
	}

	float sum() {
		return _sum;
	}

	void apply(float* in, float* out) {
		for (int i = 0; i < _size; ++i) {
			out[i] = in[i] * _window[i];
		}
	}
};

struct HanningWindow : Window {
	HanningWindow(int size, float alpha = 0.5) : Window(size) {
		const float invAlpha = 1.0 - alpha;
		const float twoPIEtc = 2.0 * M_PI / (float)size;
		for (int i = 0; i < _size; ++i) {
			_sum += _window[i] = invAlpha*cos(twoPIEtc*(float)i + M_PI) + alpha;
		}
	}
};

struct HammingWindow : HanningWindow {
	HammingWindow(int size) : HanningWindow(size, 0.54) {}
};

struct KaiserWindow : Window {
	KaiserWindow(int size, float alpha = 7.865f) : Window(size) {
		float ii0a = 1.0f / i0(alpha);
		float ism1 = 1.0f / (float)(size - 1);
		for (int i = 0; i < _size; ++i) {
			float x = i * 2.0f;
			x *= ism1;
			x -= 1.0f;
			x *= x;
			x = 1.0f - x;
			x = sqrtf(x);
			x *= alpha;
			_sum += _window[i] = i0(x) * ii0a;
		}
	}

	// Rabiner, Gold: "The Theory and Application of Digital Signal Processing", 1975, page 103.
	float i0(float x) {
		assert(x >= 0.0f);
		assert(x < 20.0f);
		float y = 0.5f * x;
		float t = .1e-8f;
		float e = 1.0f;
		float de = 1.0f;
		for (int i = 1; i <= 25; ++i) {
			de = de * y / (float)i;
			float sde = de * de;
			e += sde;
			if (e * t - sde > 0.0f) {
				break;
			}
		}
		return e;
	}
};

struct FFT1024 {
	void* _fft = NULL;
	FFT1024();
	~FFT1024();

	void do_fft(float* out, float* in);
};

struct FFT4096 {
	void* _fft = NULL;
	FFT4096();
	~FFT4096();

	void do_fft(float* out, float* in);
};

struct FFT8192 {
	void* _fft = NULL;
	FFT8192();
	~FFT8192();

	void do_fft(float* out, float* in);
};

struct FFT16384 {
	void* _fft = NULL;
	FFT16384();
	~FFT16384();

	void do_fft(float* out, float* in);
};

struct SpectrumAnalyzer : OverlappingBuffer<float> {
	enum Size {
		SIZE_128 = 128,
		SIZE_256 = 256,
		SIZE_512 = 512,
		SIZE_1024 = 1024,
		SIZE_2048 = 2048,
		SIZE_4096 = 4096,
		SIZE_8192 = 8192,
		SIZE_16384 = 16384
	};

	enum Overlap {
		OVERLAP_1 = 1,
		OVERLAP_2 = 2,
		OVERLAP_4 = 4,
		OVERLAP_8 = 8
	};

	enum WindowType {
		WINDOW_NONE,
		WINDOW_HANNING,
		WINDOW_HAMMING,
		WINDOW_KAISER
	};

	const float _sampleRate;
	ffft::FFTReal<float>* _fft;
	FFT1024* _fft1024;
	FFT4096* _fft4096;
	FFT8192* _fft8192;
	FFT16384* _fft16384;
	Window* _window;
	float* _windowOut;
	float* _fftOut;

	SpectrumAnalyzer(
		Size size,
		Overlap overlap,
		WindowType windowType,
		float sampleRate,
		bool autoProcess = true
	)
	: OverlappingBuffer(size, overlap, autoProcess)
	, _sampleRate(sampleRate)
	, _fft(NULL)
	, _fft1024(NULL)
	, _fft4096(NULL)
	, _fft8192(NULL)
	, _fft16384(NULL)
	, _window(NULL)
	, _windowOut(NULL)
	, _fftOut(new float[_size])
	{
		assert(_sampleRate > size);

		switch (size) {
			case SIZE_1024: {
				_fft1024 = new FFT1024();
				break;
			}
			case SIZE_4096: {
				_fft4096 = new FFT4096();
				break;
			}
			case SIZE_8192: {
				_fft8192 = new FFT8192();
				break;
			}
			case SIZE_16384: {
				_fft16384 = new FFT16384();
				break;
			}
			default: {
				_fft = new ffft::FFTReal<float>(size);
			}
		}

		switch (windowType) {
			case WINDOW_NONE: {
				break;
			}
			case WINDOW_HANNING: {
				_window = new HanningWindow(size);
				_windowOut = new float[size];
				break;
			}
			case WINDOW_HAMMING: {
				_window = new HammingWindow(size);
				_windowOut = new float[size];
				break;
			}
			case WINDOW_KAISER: {
				_window = new KaiserWindow(size);
				_windowOut = new float[size];
				break;
			}
		}
	}

	virtual ~SpectrumAnalyzer() {
		if (_fft) {
			delete _fft;
		}
		if (_fft1024) {
			delete _fft1024;
		}
		if (_fft4096) {
			delete _fft4096;
		}
		if (_fft8192) {
			delete _fft8192;
		}
		if (_fft16384) {
			delete _fft16384;
		}

		if (_window) {
			delete _window;
			delete[] _windowOut;
		}

		delete[] _fftOut;
	}

	void processBuffer(float* samples) override {
		float* input = samples;
		if (_window) {
			_window->apply(samples, _windowOut);
			input = _windowOut;
		}
		if (_fft1024) {
			_fft1024->do_fft(_fftOut, input);
		}
		else if (_fft4096) {
			_fft4096->do_fft(_fftOut, input);
		}
		else if (_fft8192) {
			_fft8192->do_fft(_fftOut, input);
		}
		else if (_fft16384) {
			_fft16384->do_fft(_fftOut, input);
		}
		else {
			_fft->do_fft(_fftOut, input);
		}
	}

	void getMagnitudes(float* bins, int nBins) {
		assert(nBins <= _size / 2);
		assert(_size % nBins == 0);

		const int bands = _size / 2;
		const int binWidth = bands / nBins;
		const float invBinWidth = 1.0 / (float)binWidth;
		const float normalization = 2.0 / powf(_window ? _window->sum() : _size, 2.0);
		for (int bin = 0; bin < nBins; ++bin) {
			float sum = 0.0;
			int binEnd = bin * binWidth + binWidth;
			for (int i = binEnd - binWidth; i < binEnd; ++i) {
				sum += (_fftOut[i]*_fftOut[i] + _fftOut[i + bands]*_fftOut[i + bands]) * normalization;
			}
			bins[bin] = sum * invBinWidth;
		}
	}

	// void getFrequencies(float* bins, int nBins) {
	//   assert(nBins <= _size / 2);
	//   assert(_size % nBins == 0);
	//
	//   const int bands = _size / 2;
	//   const int binWidth = bands / nBins;
	//   const float fundamental = _sampleRate / (float)_size;
	//   for (int bin = 0; bin < nBins; ++bin) {
	//     bins[bin] = roundf(bin*binWidth*fundamental + binWidth*fundamental/2.0);
	//   }
	// }
};

} // namespace dsp
} // namespace bogaudio
