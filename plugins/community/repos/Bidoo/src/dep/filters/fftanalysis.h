#include <string.h>
#include <math.h>
#include <stdio.h>
/* #include "../pffft/pffft.h" */
#include "pffft.h"
#include <vector>
#include <algorithm>

using namespace std;

struct FfftAnalysis {

	float *gInFIFO;
	float *gFFTworksp;
	float *gFFTworkspOut;
	float *gLastPhase;
	float *gAnaFreq;
	float *gAnaMagn;
	float sampleRate;
	PFFFT_Setup *pffftSetup;
	long gRover = false;
	double magn, phase, tmp, window, real, imag;
	double freqPerBin, expct, invOsamp, invFftFrameSize, invFftFrameSize2, invPi;
	long fftFrameSize, osamp, i,k, qpd, index, inFifoLatency, stepSize, fftFrameSize2;

	FfftAnalysis(long fftFrameSize, long osamp, float sampleRate) {
		this->fftFrameSize = fftFrameSize;
		this->osamp = osamp;
		this->sampleRate = sampleRate;
		pffftSetup = pffft_new_setup(fftFrameSize, PFFFT_REAL);
		fftFrameSize2 = fftFrameSize/2;
		stepSize = fftFrameSize/osamp;
		freqPerBin = sampleRate/(double)fftFrameSize;
		expct = 2.0f * M_PI * (double)stepSize/(double)fftFrameSize;
		inFifoLatency = fftFrameSize-stepSize;
		invOsamp = 1.0f/osamp;
		invFftFrameSize = 1.0f/fftFrameSize;
		invFftFrameSize2 = 1.0f/fftFrameSize2;
		invPi = 1.0f/M_PI;

		gInFIFO = (float*)calloc(fftFrameSize,sizeof(float));
		gFFTworksp = (float*)pffft_aligned_malloc(fftFrameSize*sizeof(float));
		gFFTworkspOut =  (float*)pffft_aligned_malloc(fftFrameSize*sizeof(float));
		gLastPhase = (float*)calloc((fftFrameSize/2+1),sizeof(float));
		gAnaFreq = (float*)calloc(fftFrameSize,sizeof(float));
		gAnaMagn = (float*)calloc(fftFrameSize,sizeof(float));
	}

	~FfftAnalysis() {
		pffft_destroy_setup(pffftSetup);
		free(gInFIFO);
		free(gLastPhase);
		free(gAnaFreq);
		free(gAnaMagn);
		pffft_aligned_free(gFFTworksp);
		pffft_aligned_free(gFFTworkspOut);
	}

	void process(const float *input, vector<vector<float>> *result, mutex *mylock) {

			for (i = 0; i < fftFrameSize; i++) {

				/* As long as we have not yet collected enough data just read in */
				gInFIFO[gRover] = input[i];
				gRover++;

				/* now we have enough data for processing */
				if (gRover >= fftFrameSize) {
					gRover = inFifoLatency;

					memset(gFFTworksp, 0, fftFrameSize*sizeof(float));
					memset(gFFTworkspOut, 0, fftFrameSize*sizeof(float));

					/* do windowing and re,im interleave */
					for (k = 0; k < fftFrameSize;k++) {
						window = -0.5 * cos(2.0f * M_PI * (double)k * invFftFrameSize) + 0.5f;
						gFFTworksp[k] = gInFIFO[k] * window;
					}

					/* ***************** ANALYSIS ******************* */
					/* do transform */
					pffft_transform_ordered(pffftSetup, gFFTworksp, gFFTworkspOut, NULL, PFFFT_FORWARD);

					/* this is the analysis step */
					for (k = 0; k <= fftFrameSize2; k++) {

						/* de-interlace FFT buffer */
						real = gFFTworkspOut[2*k];
						imag = gFFTworkspOut[2*k+1];

						/* compute magnitude and phase */
						magn = 2.*sqrt(real*real + imag*imag);
						phase = atan2(imag,real);

						/* compute phase difference */
						tmp = phase - gLastPhase[k];
						gLastPhase[k] = phase;

						/* subtract expected phase difference */
						tmp -= (double)k*expct;

						/* map delta phase into +/- Pi interval */
						qpd = tmp * invPi;
						if (qpd >= 0) qpd += qpd&1;
						else qpd -= qpd&1;
						tmp -= M_PI*(double)qpd;

						/* get deviation from bin frequency from the +/- Pi interval */
						tmp = osamp * tmp * invPi * 0.5f;

						/* compute the k-th partials' true frequency */
						tmp = (double)k*freqPerBin + tmp*freqPerBin;

						/* store magnitude and true frequency in analysis arrays */
						gAnaMagn[k] = magn;
						gAnaFreq[k] = tmp;
					}

					std::vector<float> v(gAnaMagn, gAnaMagn + fftFrameSize2);
					mylock->lock();
					if (result->size() == 0) {
						result->push_back(v);
					}
					else if (result->size() >= 200) {
						std::rotate(result->rbegin(), result->rbegin() + 1, result->rend());
						vector<vector<float>>& resultRef = *result;
						resultRef[0] = v;
					}
					else {
						result->push_back(v);
						std::rotate(result->rbegin(), result->rbegin() + 1, result->rend());
					}
					mylock->unlock();

					/* move input FIFO */
					for (k = 0; k < inFifoLatency; k++) gInFIFO[k] = gInFIFO[k+stepSize];
				}
			}
	}
};
