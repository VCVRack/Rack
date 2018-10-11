/* Copyright (C) 2007 Hong Zhiqian */
/**
   @file preprocess_tm.h
   @author Hong Zhiqian
   @brief Various compatibility routines for Speex (TriMedia version)
*/
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <ops/custom_defs.h>
#include "profile_tm.h"

#ifdef FIXED_POINT
#define OVERRIDE_PREPROCESS_ANALYSIS
static void preprocess_analysis(SpeexPreprocessState * restrict st, spx_int16_t * restrict x)
{
	register int i, j, framesize = st->frame_size;
	register int N = st->ps_size;
	register int N3 = 2*N - framesize;
	register int N4 = framesize - N3;
	register int * restrict ps = st->ps;
	register int * restrict frame;
	register int * restrict inbuf;
	register int * restrict ptr;
	register int max_val;

	frame = (int*)(st->frame);
	inbuf = (int*)(st->inbuf);
	ptr = (int*)(st->frame+N3);

	TMDEBUG_ALIGNMEM(x);
   	TMDEBUG_ALIGNMEM(frame);
	TMDEBUG_ALIGNMEM(inbuf);

	PREPROCESSANAYLSIS_START();

	N3 >>= 1;
	framesize >>= 1;
	max_val = 0;
	
	for ( i=0,j=0 ; i<N3 ; i+=2,j+=8 )
	{	register int r1, r2;

		r1 = ld32x(inbuf,i);
		r2 = ld32x(inbuf,i+1);

		st32d(j,   frame, r1);
		st32d(j+4, frame, r2);
	}

	for ( i=0,j=0 ; i<framesize ; i+=2,j+=8 )
	{	register int r1, r2;

		r1 = ld32x(x, i);
		r2 = ld32x(x, i+1);

		st32d(j,  ptr, r1);
		st32d(j+4,ptr, r2);
	}
   
	for ( i=0,j=0,ptr=(int*)(x+N4) ; i<N3 ; i+=2,j+=8 )
	{	register int r1, r2;

		r1 = ld32x(ptr, i);
		r2 = ld32x(ptr, i+1);

		st32d(j,  inbuf, r1);
		st32d(j+4,inbuf, r2);
	}
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unroll=2
#pragma TCS_unrollexact=1
#endif
   for ( i=0,j=0,ptr=(int*)st->window ; i<N ; ++i,j+=4 )
   {	register int f10, w10, r0, r1;

		f10 = ld32x(frame, i);
		w10 = ld32x(ptr,   i);

		r0  = (sex16(f10) * sex16(w10)) >> 15;
		r1	= (asri(16,f10) * asri(16,w10)) >> 15;

		max_val = imax(iabs(sex16(r0)), max_val);
		max_val = imax(iabs(sex16(r1)), max_val);

		r0	= pack16lsb(r1, r0);
		st32d(j, frame, r0);
	}
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
   
	max_val = 14 - spx_ilog2(max_val);
	st->frame_shift = max_val;

	if ( max_val != 0 )
	{
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
		for ( i=0,j=0 ; i<N ; ++i,j+=4 )
		{	register int f10;

			f10 = ld32x(frame, i);
			f10 = dualasl(f10, max_val);
			st32d(j, frame, f10);

		}
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
	}

	spx_fft(st->fft_lookup, st->frame, st->ft);     
	power_spectrum(st->ft, ps, N << 1);
   
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0,ptr=(int*)st->ps,max_val<<=1,j=((1<<((max_val))>>1)) ;i<N ; ++i )
	{	
		ps[i] = (ps[i] + j) >> max_val;
	}
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	filterbank_compute_bank32(st->bank, ps, ps+N);

	PREPROCESSANAYLSIS_STOP();
}

#define _MULT16_32_Q15(a,b,c) ADD32(MULT16_16((a),(b)), SHR(MULT16_16((a),(c)),15))

#define OVERRIDE_UPDATE_NOISE_PROB
static void update_noise_prob(SpeexPreprocessState * restrict st)
{
	register int i;
	register int min_range, nb_adapt;
	register int N = st->ps_size;
	register int * restrict Smin = (int*)st->Smin;
	register int * restrict Stmp = (int*)st->Stmp;
	register int * restrict S = (int*)st->S;
	
	UPDATENOISEPROB_START();

	{
		register int psi_lsb, psi_msb, ips_lsb, ips_msb, psii_lsb, psii_msb;
		register int psiii_lsb, psiii_msb;
		register int q8, q05, q2, q1;
		register int *ps = (int*)st->ps;
		register int si_lsb, si_msb, sii_lsb, sii_msb;
	
		q8	= QCONST16(.8f,15);
		q05 = QCONST16(.05f,15);
		q2	= QCONST16(.2f,15);
		q1	= QCONST16(.1f,15);

		ips_lsb	 = ps[0];
		psi_lsb	 = ps[1];
		si_lsb	 = S[0];
		ips_msb	 = ips_lsb >> 15;
		psi_msb	 = psi_lsb >> 15;
		si_msb	 = si_lsb >> 15;

		ips_lsb	 &= 0x00007fff;
		psi_lsb	 &= 0x00007fff;
		si_lsb	 &= 0x00007fff;
		
		S[0] = _MULT16_32_Q15(q8,si_msb,si_lsb) +  _MULT16_32_Q15(q2,ips_msb,ips_lsb);

		for ( i=1 ; i<N-1 ; i+=2 )
		{
			psii_lsb = ps[i+1];
			si_lsb	 = S[i];

			psii_msb = psii_lsb >> 15;
			si_msb	 = si_lsb >> 15;
			si_lsb	 &= 0x00007fff;
			psii_lsb &= 0x00007fff;

			S[i]= _MULT16_32_Q15(q8,si_msb,si_lsb) + 
				  _MULT16_32_Q15(q05,ips_msb,ips_lsb) + 
				  _MULT16_32_Q15(q1,psi_msb,psi_lsb) + 
				  _MULT16_32_Q15(q05,psii_msb,psii_lsb);
			
			psiii_lsb = ps[i+2];
			sii_lsb	 = S[i+1];

			sii_msb	 =  sii_lsb >> 15;
			psiii_msb=  psiii_lsb >> 15;
			sii_lsb	 &= 0x00007fff;
			psiii_lsb&= 0x00007fff;

			S[i+1]= _MULT16_32_Q15(q8,sii_msb,sii_lsb) + 
					_MULT16_32_Q15(q05,psi_msb,psi_lsb) + 
					_MULT16_32_Q15(q1,psii_msb,psii_lsb) + 
					_MULT16_32_Q15(q05,psiii_msb,psiii_lsb);

			ips_lsb = psii_lsb;
			ips_msb = psii_msb;
			psi_lsb = psiii_lsb;
			psi_msb = psiii_msb;
		}
   
		S[N-1] = MULT16_32_Q15(q8,S[N-1]) + MULT16_32_Q15(q2,ps[N-1]);
	}

	nb_adapt = st->nb_adapt;
   
	if ( nb_adapt==1 )
	{	for ( i=0 ; i<N ; ++i )
			Smin[i] = Stmp[i] = 0;

	}

	min_range = mux(nb_adapt < 100,		15,
				mux(nb_adapt < 1000,	50,
				mux(nb_adapt < 10000,	150, 300)));

	if ( st->min_count > min_range )
	{
		st->min_count = 0;

#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unroll=2
#pragma TCS_unrollexact=1
#endif
		for ( i=0 ; i<N ; ++i )
		{	register int si, stmpi;

			si		= S[i];
			stmpi	= Stmp[i];
				
			Smin[i] = imin(stmpi,si);
			Stmp[i] = si;
		}
#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
	} else 
	{

#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unroll=2
#pragma TCS_unrollexact=1
#endif
		for ( i=0 ; i<N ; ++i )
		{	register int si, stmpi, smini;

			si		= S[i];
			stmpi	= Stmp[i];
			smini	= Smin[i];

			Smin[i] = imin(smini,si);
			Stmp[i] = imin(stmpi,si);      
		}
#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
	}


	{
		register int q4;
		register int * restrict update_prob = (int*)st->update_prob;

		q4 = QCONST16(.4f,15);

#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
		for ( i=0 ; i<N ; ++i )
		{	register int si;
			register int smini;

			si = S[i];
			smini = Smin[i];
			update_prob[i] = mux(MULT16_32_Q15(q4,si) > ADD32(smini,20), 1, 0);
		}
#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
	}

	UPDATENOISEPROB_STOP();
}

#else

#define OVERRIDE_PREPROCESS_ANALYSIS
static void preprocess_analysis(SpeexPreprocessState * restrict st, spx_int16_t * restrict x)
{
	register int i;
	register int framesize = st->frame_size;
	register int N = st->ps_size;
	register int N3 = 2*N - framesize;
	register int N4 = framesize - N3;
	register float * restrict ps = st->ps;
	register float * restrict frame = st->frame;
	register float * restrict inbuf = st->inbuf;

	PREPROCESSANAYLSIS_START();

#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<N3 ; ++i )
	{
		frame[i] = inbuf[i];
	}
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
   	for ( i=0 ; i<framesize ; ++i )
	{	frame[N3+i] = x[i];
	}
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0,x+=N4 ; i<N3 ; ++i )
	{	inbuf[i]  = x[i];
	}
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	inbuf = st->window;

#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<2*N ; ++i )
	{
		frame[i] = frame[i] * inbuf[i];
	}
#if (TM_UNROLL && TM_UNROLL_PREPROCESSANALYSIS)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	spx_fft(st->fft_lookup, frame, st->ft);         
	power_spectrum(st->ft, ps, N << 1);
	filterbank_compute_bank32(st->bank, ps, ps+N);

	PREPROCESSANAYLSIS_STOP();
}


#define OVERRIDE_UPDATE_NOISE_PROB
static void update_noise_prob(SpeexPreprocessState * restrict st)
{
   
	register float * restrict S = st->S;
   	register float * restrict ps = st->ps;
	register int N = st->ps_size;
    register int min_range;
	register int i;
	register int nb_adapt;
	register float * restrict Smin = st->Smin;
	register float * restrict Stmp = st->Stmp;

	UPDATENOISEPROB_START();

	{
		register float ips, psi;

		ips  = ps[0];
		psi  = ps[1];

		S[0] = .8f * S[0] + .2f * ips;

		for ( i=1 ; i<N-1 ; i+=2 )
		{
			register float psii, psiii;

			psii	= ps[i+1];
			psiii	= ps[i+2];
			S[i]	= .8f * S[i]	+ .05f * ips + .1f * psi  + .05f * psii;
			S[i+1]	= .8f * S[i+1]	+ .05f * psi + .1f * psii + .05f * psiii; 
			ips		= psii;
			psi		= psiii;
		}

		S[N-1] = .8f * S[N-1] + .2f * ps[N-1];
	}

	nb_adapt = st->nb_adapt;
   
   	if ( nb_adapt==1 )
	{	
		for (i=0;i<N;i++)
			Smin[i] = st->Stmp[i] = 0;
	}

	min_range = mux(nb_adapt < 100,		15,
				mux(nb_adapt < 1000,	50,
				mux(nb_adapt < 10000,	150, 300)));

   
	if ( st->min_count > min_range )
	{
		st->min_count = 0;
#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
		for ( i=0 ; i<N ; ++i )
		{
			register float stmpi, si;

			stmpi	= Stmp[i];
			si		= S[i];

			Smin[i] = fmin(stmpi,si);
			Stmp[i] = si;
		}
#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	} else 
	{
		register float * restrict Smin = st->Smin;

#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
		for ( i=0 ; i<N ; ++i )
		{
			register float stmpi, si, smini;

			stmpi	= Stmp[i];
			si		= S[i];
			smini	= Smin[i];
			
			Smin[i] = fmin(smini,si);
			Stmp[i] = fmin(stmpi,si);      
		}
#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
	}

	{
		register int * restrict update_prob = (int*)st->update_prob;

#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
		for (i=0;i<N;i++)
		{	register float si;
			register float smini;

			si		= S[i];
			smini	= Smin[i];
			update_prob[i] = mux( (.4 * si) > (smini + 20.f), 1, 0);
			
		}
#if (TM_UNROLL && TM_UNROLL_UPDATENOISEPROB)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
	}

	UPDATENOISEPROB_STOP();
}


#define OVERRIDE_COMPUTE_GAIN_FLOOR
static void compute_gain_floor(
	int noise_suppress, 
	int effective_echo_suppress, 
	float * restrict noise, 
	float * restrict echo, 
	float * gain_floor, 
	int len
)
{
	register int i;
	register float echo_floor;
	register float noise_floor;

	COMPUTEGAINFLOOR_START();

	noise_floor = exp(.2302585f*noise_suppress);
	echo_floor = exp(.2302585f*effective_echo_suppress);

#if (TM_UNROLL && TM_UNROLL_COMPUTEGAINFLOOR)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for (i=0;i<len;i++)
	{	register float noisei, echoi;

		noisei = noise[i];
		echoi  = echo[i];

		gain_floor[i] = FRAC_SCALING * sqrt(noise_floor * noisei +	echo_floor * echoi) / sqrt(1+noisei+echoi);

	}
#if (TM_UNROLL && TM_UNROLL_COMPUTEGAINFLOOR)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	COMPUTEGAINFLOOR_STOP();
}

#endif

static inline spx_word32_t hypergeom_gain(spx_word32_t xx);
static inline spx_word16_t qcurve(spx_word16_t x);
static void compute_gain_floor(int noise_suppress, int effective_echo_suppress, spx_word32_t *noise, spx_word32_t *echo, spx_word16_t *gain_floor, int len);
void speex_echo_get_residual(SpeexEchoState *st, spx_word32_t *Yout, int len);

#ifndef FIXED_POINT
static void speex_compute_agc(SpeexPreprocessState *st, spx_word16_t Pframe, spx_word16_t *ft);
#endif

void preprocess_residue_echo(
	SpeexPreprocessState * restrict st,
	int N,
	int NM
)
{
	if (st->echo_state)
	{
		register spx_word32_t * restrict r_echo = st->residual_echo;
		register spx_word32_t * restrict e_noise = st->echo_noise;
		register int i;

#ifndef FIXED_POINT
		register spx_word32_t r;
#endif

		speex_echo_get_residual(st->echo_state, r_echo, N);

#ifndef FIXED_POINT
		r = r_echo[0];
		if (!(r >=0 && r < N*1e9f) )
		{	
			memset(r_echo, 0, N * sizeof(spx_word32_t));
		}
#endif

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
		for (i=0;i<N;i++)
		{	register spx_word32_t eni = e_noise[i];
			e_noise[i] = MAX32(MULT16_32_Q15(QCONST16(.6f,15),eni), r_echo[i]);
		}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
		filterbank_compute_bank32(st->bank, e_noise, e_noise+N);
   
	} else 
	{	memset(st->echo_noise, 0, (NM) * sizeof(spx_word32_t));
	}
}

void preprocess_update_noise(
	SpeexPreprocessState * restrict st,
	spx_word32_t * restrict ps,
	int N
)
{
	register spx_word16_t beta, beta_1;
	register int * restrict up = st->update_prob;
	register spx_word32_t * restrict noise = st->noise;
	register int i;

	beta = MAX16(QCONST16(.03,15),DIV32_16(Q15_ONE,st->nb_adapt));
	beta_1 = Q15_ONE-beta;

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for (i=0;i<N;i++)
	{	register spx_word32_t ni = noise[i];
		register spx_word32_t psi = ps[i];

		if ( !up[i] || psi < PSHR32(ni, NOISE_SHIFT) )
		{	noise[i] =	MAX32(EXTEND32(0),MULT16_32_Q15(beta_1,ni) + 
						MULT16_32_Q15(beta,SHL32(psi,NOISE_SHIFT)));
		}
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
	filterbank_compute_bank32(st->bank, noise, noise+N);
}

void preprocess_compute_SNR(
	SpeexPreprocessState * restrict st,
	spx_word32_t * restrict ps,
	int NM
)
{
	register spx_word32_t * restrict noise = st->noise;
	register spx_word32_t * restrict echo = st->echo_noise;
	register spx_word32_t * restrict reverb = st->reverb_estimate;
	register spx_word16_t * restrict post = st->post;
	register spx_word32_t * restrict old_ps = st->old_ps;
	register spx_word16_t * restrict prior = st->prior;
	register int i;

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<NM ; i++)
	{
		register spx_word16_t gamma;
		register spx_word32_t tot_noise;
		register spx_word16_t posti;
		register spx_word32_t opsi;
		register spx_word16_t priori;

		tot_noise = ADD32(ADD32(ADD32(EXTEND32(1), PSHR32(noise[i],NOISE_SHIFT)), echo[i]) , reverb[i]);
      
		posti = SUB16(DIV32_16_Q8(ps[i],tot_noise), QCONST16(1.f,SNR_SHIFT));
		posti = MIN16(posti, QCONST16(100.f,SNR_SHIFT));
		post[i] = posti;

		opsi = old_ps[i];

		gamma = QCONST16(.1f,15)+MULT16_16_Q15(QCONST16(.89f,15),SQR16_Q15(DIV32_16_Q15(opsi,ADD32(opsi,tot_noise))));
      
		priori = EXTRACT16(PSHR32(ADD32(MULT16_16(gamma,MAX16(0,posti)), MULT16_16(Q15_ONE-gamma,DIV32_16_Q8(opsi,tot_noise))), 15));
		prior[i]=MIN16(priori, QCONST16(100.f,SNR_SHIFT));
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
}

spx_word32_t preprocess_smooth_SNR(
	SpeexPreprocessState * restrict st,
	int N,
	int NM
)
{
	register spx_word16_t * restrict zeta = st->zeta;
	register spx_word16_t * restrict prior = st->prior;
	register spx_word32_t Zframe;
	register spx_word16_t iprior, priori;
	register int _N = N-1;
	register int i;

	iprior = prior[0];
	priori = prior[1];
	zeta[0] = PSHR32(ADD32(MULT16_16(QCONST16(.7f,15),zeta[0]), MULT16_16(QCONST16(.3f,15),iprior)),15);
   
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=2
#pragma TCS_unrollexact=1
#endif
	for ( i=1 ; i<_N ; i++)
	{	register spx_word16_t zetai = zeta[i];
		register spx_word16_t priorii = prior[i+1];

		zeta[i] = PSHR32(ADD32(ADD32(ADD32(MULT16_16(QCONST16(.7f,15),zetai), MULT16_16(QCONST16(.15f,15),priori)),
                  MULT16_16(QCONST16(.075f,15),iprior)), MULT16_16(QCONST16(.075f,15),priorii)),15);

		iprior = priori;
		priori = priorii;
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	for (i=_N; i<NM ; i++)
	{	register spx_word16_t zetai = zeta[i];
		
		priori = prior[i];
		zeta[i] = PSHR32(ADD32(MULT16_16(QCONST16(.7f,15),zetai), MULT16_16(QCONST16(.3f,15),priori)),15);
	}

	Zframe = 0;
   
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=N ; i<NM ; i++ )
	{	Zframe = ADD32(Zframe, EXTEND32(zeta[i]));
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	return Zframe;
}

void preprocess_compute_emgain(
	SpeexPreprocessState * restrict st,
	spx_word32_t * restrict ps,
	spx_word16_t Pframe,
	int NM
)
{
	register spx_word16_t * restrict zeta = st->zeta;
	register spx_word16_t * restrict prior = st->prior;
	register spx_word16_t * restrict gain = st->gain;
	register spx_word32_t * restrict old_ps = st->old_ps;
	register spx_word16_t * restrict post = st->post;
	register spx_word16_t * restrict gain2 = st->gain2;
	register int i;
	register int N=st->ps_size;
	
	for ( i=N ; i<NM ; ++i )
	{
		register spx_word32_t theta;
		register spx_word32_t MM;
		register spx_word16_t prior_ratio;
		register spx_word16_t P1;
		register spx_word16_t q;

#ifdef FIXED_POINT
		register spx_word16_t tmp;
#endif
		register spx_word16_t priori = prior[i];
		
		prior_ratio = PDIV32_16(SHL32(EXTEND32(priori), 15), ADD16(priori, SHL32(1,SNR_SHIFT)));
		theta = MULT16_32_P15(prior_ratio, QCONST32(1.f,EXPIN_SHIFT)+SHL32(EXTEND32(post[i]),EXPIN_SHIFT-SNR_SHIFT));

		MM = hypergeom_gain(theta);
		gain[i] = EXTRACT16(MIN32(Q15_ONE, MULT16_32_Q15(prior_ratio, MM)));
		old_ps[i] = MULT16_32_P15(QCONST16(.2f,15),old_ps[i]) + MULT16_32_P15(MULT16_16_P15(QCONST16(.8f,15),SQR16_Q15(gain[i])),ps[i]);

		P1 = QCONST16(.199f,15)+MULT16_16_Q15(QCONST16(.8f,15),qcurve (zeta[i]));
		q = Q15_ONE-MULT16_16_Q15(Pframe,P1);

#ifdef FIXED_POINT
		theta = MIN32(theta, EXTEND32(32767));
		tmp = MULT16_16_Q15((SHL32(1,SNR_SHIFT)+priori),EXTRACT16(MIN32(Q15ONE,SHR32(spx_exp(-EXTRACT16(theta)),1))));
		tmp = MIN16(QCONST16(3.,SNR_SHIFT), tmp);
		tmp = EXTRACT16(PSHR32(MULT16_16(PDIV32_16(SHL32(EXTEND32(q),8),(Q15_ONE-q)),tmp),8));
		gain2[i]=DIV32_16(SHL32(EXTEND32(32767),SNR_SHIFT), ADD16(256,tmp));
#else
		gain2[i]=1/(1.f + (q/(1.f-q))*(1+priori)*exp(-theta));
#endif
	}

	filterbank_compute_psd16(st->bank,gain2+N, gain2);
	filterbank_compute_psd16(st->bank,gain+N, gain);
}

void preprocess_compute_linear_gain(
	SpeexPreprocessState * restrict st,
	spx_word32_t * restrict ps,	
	int N
)
{
	register spx_word16_t * restrict gain_floor = st->gain_floor;
	register spx_word16_t * restrict prior = st->prior;
	register spx_word16_t * restrict gain = st->gain;
	register spx_word32_t * restrict old_ps = st->old_ps;
	register spx_word16_t * restrict post = st->post;
	register spx_word16_t * restrict gain2 = st->gain2;
	register int i;

	filterbank_compute_psd16(st->bank,gain_floor+N,gain_floor);

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif   
	for (i=0;i<N;i++)
    {
         register spx_word32_t MM;
         register spx_word32_t theta;
         register spx_word16_t prior_ratio;
         register spx_word16_t tmp;
         register spx_word16_t p;
         register spx_word16_t g;
		 register spx_word16_t gfi = gain_floor[i];

		 prior_ratio = PDIV32_16(SHL32(EXTEND32(st->prior[i]), 15), ADD16(prior[i], SHL32(1,SNR_SHIFT)));
         theta = MULT16_32_P15(prior_ratio, QCONST32(1.f,EXPIN_SHIFT)+SHL32(EXTEND32(post[i]),EXPIN_SHIFT-SNR_SHIFT));
		 MM = hypergeom_gain(theta);

		 g = EXTRACT16(MIN32(Q15_ONE, MULT16_32_Q15(prior_ratio, MM)));
		 p = gain2[i];
         
		 g = VMUX( MULT16_16_Q15(QCONST16(.333f,15),g) > gain[i], MULT16_16(3,gain[i]), g);

         old_ps[i]= MULT16_32_P15(QCONST16(.2f,15),old_ps[i]) + 
					MULT16_32_P15(MULT16_16_P15(QCONST16(.8f,15),SQR16_Q15(g)),ps[i]);
         
		 g = VMUX( g < gfi, gfi, g );
         gain[i] = g;

         tmp =	MULT16_16_P15(p,spx_sqrt(SHL32(EXTEND32(g),15))) + 
				MULT16_16_P15(SUB16(Q15_ONE,p),spx_sqrt(SHL32(EXTEND32(gfi),15)));

         gain2[i]=SQR16_Q15(tmp);

         /* Use this if you want a log-domain MMSE estimator instead */
         /* gain2[i] = pow(g, p) * pow(gfi,1.f-p);*/
    }
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
}


#if 0
void preprocess_compute_bark_gain(
	SpeexPreprocessState * restrict st,
	int N,
	int NM
)
{
	register spx_word16_t * restrict gain_floor = st->gain_floor;
	register spx_word16_t * restrict gain = st->gain;
	register spx_word16_t * restrict gain2 = st->gain2;
	register int i;

    for (i=N;i<NM;i++)
    {
		register spx_word16_t tmp;
		register spx_word16_t p = gain2[i];
		register spx_word16_t gaini;
		register spx_word16_t gfi = gain_floor[i];

		gaini = MAX16(gain[i], gfi); 

		gain[i] = gaini;

		tmp =	MULT16_16_P15(p,spx_sqrt(SHL32(EXTEND32(gaini),15))) + 
			MULT16_16_P15(SUB16(Q15_ONE,p),spx_sqrt(SHL32(EXTEND32(gfi),15)));

		gain2[i]=SQR16_Q15(tmp);
    }

    filterbank_compute_psd16(st->bank,gain2+N, gain2);
}
#endif

void preprocess_apply_gain(
	SpeexPreprocessState * restrict st,
	int N
)
{
	register spx_word16_t * restrict ft = st->ft;
	register spx_word16_t * restrict gain2 = st->gain2;
	register int j, i;

	ft[0] = MULT16_16_P15(gain2[0],ft[0]);

	for (i=1,j=1; i<N ; i++,j+=2)
	{
		register spx_word16_t gain2i = gain2[i];
		register spx_word16_t ftj = ft[j];
		register spx_word16_t ftjj = ft[j+1];

		ft[j] = MULT16_16_P15(gain2i,ftj);
		ft[j+1] = MULT16_16_P15(gain2i,ftjj);
	}

	ft[(N<<1)-1] = MULT16_16_P15(gain2[N-1],ft[(N<<1)-1]);
}

#ifdef FIXED_POINT
void preprocess_scale(
	SpeexPreprocessState * restrict st,
	int N
)
{
	register spx_word16_t * restrict frame = st->frame;
	register int shift = st->frame_shift;
	register int i;
	register int N2 = N << 1;

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif    
	for ( i=0 ; i<N2 ;i++)
	{	register spx_word16_t framei = frame[i];
		
		frame[i] = PSHR16(framei,shift);
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
}

#else

void preprocess_apply_agc(
	SpeexPreprocessState * restrict st,
	int N				  
)
{				  
	register spx_word16_t max_sample=0;
	register spx_word16_t * restrict frame = st->frame;
	register int i;
	register int N2 = N << 1;

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif    
	for (i=0;i<N2;i++)
	{	register spx_word16_t framei = VABS(frame[i]);

		max_sample = VMUX( framei > max_sample, framei, max_sample);
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif    

	if ( max_sample > 28000.f )
	{
		float damp = 28000.f/max_sample;

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif  
		for ( i=0 ; i< N2 ; i++ )
		{	frame[i] *= damp;
		}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif  
	}
}
#endif


void preprocess_update(
	SpeexPreprocessState * restrict st,
	spx_int16_t * restrict x,
	int N	
)
{
	register spx_word16_t * restrict frame = st->frame;
	register spx_word16_t * restrict window = st->window;
	register spx_word16_t * restrict outbuf = st->outbuf;
	register int framesize = st->frame_size;
	register int N2 = N << 1;
	register int N3 = N2 - framesize;
	register int N4 = (framesize) - N3;
	register int i;

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif  
	for ( i=0 ; i<N2 ; i++)
	{	register spx_word16_t fi = frame[i];
		register spx_word16_t wi = window[i];

		frame[i] = MULT16_16_Q15(fi, wi);
	}
	for (i=0;i<N3;i++)
	{	x[i] = outbuf[i] + frame[i];
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif 

	for ( i=0;i<N4;i++)
	{	x[N3+i] = frame[N3+i];
	}
	
	memcpy(outbuf, frame+framesize, (N3) * sizeof(spx_word16_t));
}

#define OVERRIDE_SPEEX_PREPROCESS_RUN
int speex_preprocess_run(SpeexPreprocessState * restrict st, spx_int16_t * restrict x)
{
	register int i, N, M, NM;
	register spx_word32_t * restrict ps=st->ps;
	register spx_word32_t Zframe;
	register spx_word16_t Pframe;

	st->nb_adapt++;
	st->min_count++;
	N = st->ps_size;
	M = st->nbands;
	NM = N + M;

	preprocess_residue_echo(st, N, NM);
	preprocess_analysis(st, x);
	update_noise_prob(st);
	preprocess_update_noise(st, ps, N);

	if ( st->nb_adapt == 1 )
	{	memcpy(st->old_ps, ps, (NM) * sizeof(spx_word32_t));
	}

	preprocess_compute_SNR(st, ps, NM);
	Zframe = preprocess_smooth_SNR(st, N, NM);
	

	{
	register spx_word16_t effective_echo_suppress;
	
	Pframe = QCONST16(.1f,15)+MULT16_16_Q15(QCONST16(.899f,15),qcurve(DIV32_16(Zframe,M)));
	effective_echo_suppress =	EXTRACT16(PSHR32(ADD32(MULT16_16(SUB16(Q15_ONE,Pframe), st->echo_suppress), 
								MULT16_16(Pframe, st->echo_suppress_active)),15));
	compute_gain_floor(st->noise_suppress, effective_echo_suppress, st->noise+N, st->echo_noise+N, st->gain_floor+N, M);
    
	}

	preprocess_compute_emgain(st, ps, Pframe, NM);
	preprocess_compute_linear_gain(st, ps, N);

   
	if (!st->denoise_enabled)
	{
	   register spx_word16_t * restrict gain2 = st->gain2;

#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif  
		for ( i=0 ; i<NM ; i++ )
		{   gain2[i] = Q15_ONE;
		}
#if (TM_UNROLL && TM_UNROLL_SPEEXPREPROCESSRUN)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
	}
     
	preprocess_apply_gain(st, N);

#ifndef FIXED_POINT
	if (st->agc_enabled)
	{	speex_compute_agc(st, Pframe, st->ft);
	}
#endif


   spx_ifft(st->fft_lookup, st->ft, st->frame);

#ifdef FIXED_POINT
	preprocess_scale(st, N);
#endif

#ifndef FIXED_POINT
	if ( st->agc_enabled )
	{	preprocess_apply_agc(st, N);
	}
#endif

	preprocess_update(st, x, N);

	if ( st->vad_enabled )
	{
		if (Pframe > st->speech_prob_start || (st->was_speech && Pframe > st->speech_prob_continue))
		{	st->was_speech=1;
			return 1;
	  
		} else
		{	st->was_speech=0;
			return 0;
		}
	} else 
	{	return 1;
	}
}
