#include <string.h>
#include <math.h>
#include <stdio.h>
/* #include "../pffft/pffft.h" */
#include "pffft.h"

using namespace std;

struct PitchShifter {

	float *gInFIFO;
	float *gOutFIFO;
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

   static void *my_calloc(size_t _size) {
      // (note) same as calloc, this does not fix the noise issue in REI (edit: see below for actual fix)
      void *r = malloc(_size);
      if(NULL != r)
      {
         memset(r, 0, _size);
      }
      return r;
   }

	PitchShifter(long fftFrameSize, long osamp, float sampleRate) {
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

		gInFIFO = (float*)my_calloc(fftFrameSize * sizeof(float));
		gOutFIFO =  (float*)my_calloc(fftFrameSize * sizeof(float));
		gFFTworksp = (float*)pffft_aligned_malloc(fftFrameSize*sizeof(float));
      memset((void*)gFFTworksp, 0, sizeof(fftFrameSize*sizeof(float)));
		gFFTworkspOut =  (float*)pffft_aligned_malloc(fftFrameSize*sizeof(float));
      memset((void*)gFFTworkspOut, 0, sizeof(fftFrameSize*sizeof(float)));
		gLastPhase = (float*)my_calloc((fftFrameSize/2+1) * sizeof(float));
		gSumPhase = (float*)my_calloc((fftFrameSize/2+1) * sizeof(float));
		gOutputAccum = (float*)my_calloc(2*fftFrameSize * sizeof(float));
		gAnaFreq = (float*)my_calloc(fftFrameSize * sizeof(float));
		gAnaMagn = (float*)my_calloc(fftFrameSize * sizeof(float));
		gSynFreq = (float*)my_calloc(fftFrameSize * sizeof(float));
		gSynMagn = (float*)my_calloc(fftFrameSize * sizeof(float));
	}

	~PitchShifter() {
		pffft_destroy_setup(pffftSetup);
		free(gInFIFO);
		free(gOutFIFO);
		free(gLastPhase);
		free(gSumPhase);
		free(gOutputAccum);
		free(gAnaFreq);
		free(gAnaMagn);
		free(gSynFreq);
		free(gSynMagn);
		pffft_aligned_free(gFFTworksp);
		pffft_aligned_free(gFFTworkspOut);
	}

	void process(const float pitchShift, const float *input, float *output) {


			for (i = 0; i < fftFrameSize; i++) {

				/* As long as we have not yet collected enough data just read in */
				gInFIFO[gRover] = input[i];

				if(gRover >= inFifoLatency)  // [bsp] 09Mar2019: this fixes the noise burst issue in REI
					 output[i] = gOutFIFO[gRover-inFifoLatency];
				else
					 output[i] = 0.0f;
					 
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

					/* ***************** PROCESSING ******************* */
					/* this does the actual pitch shifting */

					memset(gSynMagn, 0, fftFrameSize*sizeof(float));
					memset(gSynFreq, 0, fftFrameSize*sizeof(float));

					for (k = 0; k <= fftFrameSize2; k++) {
						index = k*pitchShift;
						if (index <= fftFrameSize2) {
							gSynMagn[index] += gAnaMagn[k];
							gSynFreq[index] = gAnaFreq[k] * pitchShift;
						}
					}

					memset(gFFTworksp, 0, fftFrameSize*sizeof(float));
					memset(gFFTworkspOut, 0, fftFrameSize*sizeof(float));

					/* ***************** SYNTHESIS ******************* */
					/* this is the synthesis step */
					for (k = 0; k <= fftFrameSize2; k++) {

						/* get magnitude and true frequency from synthesis arrays */
						magn = k==0?0:gSynMagn[k];
						tmp = gSynFreq[k];

						/* subtract bin mid frequency */
						tmp -= (double)k*freqPerBin;

						/* get bin deviation from freq deviation */
						tmp /= freqPerBin;

						/* take osamp into account */
						tmp = 2.0f * M_PI * tmp * invOsamp;

						/* add the overlap phase advance back in */
						tmp += (double)k*expct;

						/* accumulate delta phase to get bin phase */
						gSumPhase[k] += tmp;
						phase = gSumPhase[k];

						/* get real and imag part and re-interleave */
						gFFTworksp[2*k] = magn*cos(phase);
						gFFTworksp[2*k+1] = magn*sin(phase);
					}

					// /* zero negative frequencies */
					// for (k = fftFrameSize+2; k < 2*fftFrameSize; k++) gFFTworksp[k] = 0.0f;

					/* do inverse transform */
					pffft_transform_ordered(pffftSetup, gFFTworksp, gFFTworkspOut , NULL, PFFFT_BACKWARD);

					/* do windowing and add to output accumulator */
					for(k=0; k < fftFrameSize; k++) {
						window = -0.5f * cos(2.0f * M_PI *(double)k * invFftFrameSize) + 0.5f;
						gOutputAccum[k] += 2.0f * window * gFFTworkspOut[k] * invFftFrameSize2 * invOsamp;
					}
					for (k = 0; k < stepSize; k++) gOutFIFO[k] = gOutputAccum[k];

					/* shift accumulator */
					memmove(gOutputAccum, gOutputAccum+stepSize, fftFrameSize*sizeof(float));

					/* move input FIFO */
					for (k = 0; k < inFifoLatency; k++) gInFIFO[k] = gInFIFO[k+stepSize];
				}
			}
	}
};
