#include <string.h>
#include <math.h>
#include <stdio.h>
/* #include "../pffft/pffft.h" */
#include "pffft.h"

using namespace std;

struct FftSynth {
	float *gFFTworksp;
	float *gFFTworkspOut;
	float *gLastPhase;
	float *gSumPhase;
	float *gOutputAccum;
	float *gAnaFreq;
	float *gAnaMagn;
	float *gSynFreq;
	float *gSynMagn;
	float sampleRate;
	PFFFT_Setup *pffftSetup;
	long gRover = false;
	double magn, phase, tmp, window, real, imag;
	double freqPerBin, expct, invOsamp, invFftFrameSize, invFftFrameSize2, invPi;
	long fftFrameSize, osamp, i,k, qpd, index, inFifoLatency, stepSize, fftFrameSize2;

	FftSynth(long fftFrameSize, long osamp, float sampleRate) {
		this->fftFrameSize = fftFrameSize;
		this->osamp = osamp;
		this->sampleRate = sampleRate;
		pffftSetup = pffft_new_setup(fftFrameSize, PFFFT_COMPLEX);
		fftFrameSize2 = fftFrameSize/2;
		stepSize = fftFrameSize/osamp;
		freqPerBin = sampleRate/(double)fftFrameSize;
		expct = 2.0f * M_PI * (double)stepSize/(double)fftFrameSize;
		inFifoLatency = fftFrameSize-stepSize;
		invFftFrameSize = 1.0f/fftFrameSize;
		invFftFrameSize2 = 1.0f/fftFrameSize2;
		invPi = 1.0f/M_PI;
		invOsamp = 1.0f/osamp;

		gFFTworksp = (float*)pffft_aligned_malloc(2*fftFrameSize*sizeof(float));
		gFFTworkspOut =  (float*)pffft_aligned_malloc(2*fftFrameSize*sizeof(float));
		gOutputAccum = (float*)calloc(2*fftFrameSize,sizeof(float));
	}

	~FftSynth() {
		pffft_destroy_setup(pffftSetup);
		free(gOutputAccum);
		pffft_aligned_free(gFFTworksp);
		pffft_aligned_free(gFFTworkspOut);
	}

	void process(const float * magn, const float *phase, float *output) {

		memset(gFFTworksp, 0, 2*fftFrameSize*sizeof(float));
		memset(gFFTworkspOut, 0, 2*fftFrameSize*sizeof(float));

		for (k = 0; k <= fftFrameSize2; k++) {
			/* get real and imag part and re-interleave */
			gFFTworksp[2*k] = magn[k]*cos(phase[k]);
			gFFTworksp[2*k+1] = magn[k]*sin(phase[k]);
		}

		for (k = fftFrameSize+2; k < 2*fftFrameSize; k++) gFFTworksp[k] = 0.0f;

		pffft_transform_ordered(pffftSetup, gFFTworksp, gFFTworkspOut , NULL, PFFFT_BACKWARD);

		for(k=0; k < fftFrameSize; k++) {
			window = -0.5f * cos(2.0f * M_PI *(double)k * invFftFrameSize) + 0.5f;
			gOutputAccum[k] += 2.0f * window * gFFTworkspOut[2*k] * invFftFrameSize2;
		}

		for (k = 0; k < stepSize; k++) output[k] = gOutputAccum[k];

		memmove(gOutputAccum, gOutputAccum+stepSize, fftFrameSize*sizeof(float));
	}
};
