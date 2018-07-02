
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
