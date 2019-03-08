
#include "ffft/FFTRealFixLen.h"

#include "buffer.hpp"
#include "analyzer.hpp"

using namespace bogaudio::dsp;


typedef ffft::FFTRealFixLen<10> FIXED_FFT1024;

FFT1024::FFT1024() {
	_fft = new FIXED_FFT1024();
}

FFT1024::~FFT1024() {
	delete (FIXED_FFT1024*)_fft;
}

void FFT1024::do_fft(float* out, float* in) {
	((FIXED_FFT1024*)_fft)->do_fft(out, in);
}


typedef ffft::FFTRealFixLen<12> FIXED_FFT4096;

FFT4096::FFT4096() {
	_fft = new FIXED_FFT4096();
}

FFT4096::~FFT4096() {
	delete (FIXED_FFT4096*)_fft;
}

void FFT4096::do_fft(float* out, float* in) {
	((FIXED_FFT4096*)_fft)->do_fft(out, in);
}


typedef ffft::FFTRealFixLen<13> FIXED_FFT8192;

FFT8192::FFT8192() {
	_fft = new FIXED_FFT8192();
}

FFT8192::~FFT8192() {
	delete (FIXED_FFT8192*)_fft;
}

void FFT8192::do_fft(float* out, float* in) {
	((FIXED_FFT8192*)_fft)->do_fft(out, in);
}


typedef ffft::FFTRealFixLen<14> FIXED_FFT16384;

FFT16384::FFT16384() {
	_fft = new FIXED_FFT16384();
}

FFT16384::~FFT16384() {
	delete (FIXED_FFT16384*)_fft;
}

void FFT16384::do_fft(float* out, float* in) {
	((FIXED_FFT16384*)_fft)->do_fft(out, in);
}
