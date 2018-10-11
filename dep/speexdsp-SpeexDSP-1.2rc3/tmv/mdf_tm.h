/* Copyright (C) 2007 Hong Zhiqian */
/**
   @file mdf_tm.h
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

// shifted power spectrum to fftwrap.c so that optimisation can be shared between mdf.c and preprocess.c
#define OVERRIDE_POWER_SPECTRUM					

#ifdef FIXED_POINT

#else

#define OVERRIDE_FILTER_DC_NOTCH16
void filter_dc_notch16(
	const spx_int16_t * restrict in, 
	float radius, 
	float * restrict out, 
	int len, 
	float * restrict mem
)
{
	register int i;
	register float den2, r1;
	register float mem0, mem1;

	FILTERDCNOTCH16_START();

	r1 = 1 - radius;
	den2 = (radius * radius) + (0.7 * r1 * r1);
	mem0 = mem[0];
	mem1 = mem[1];

#if (TM_UNROLL && TM_UNROLL_FILTERDCNOTCH16)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<len ; ++i )
	{
		register float vin  = in[i];
		register float vout = mem0 + vin;
		register float rvout = radius * vout;

		mem0 = mem1 + 2 * (-vin + rvout);
		mem1 = vin - (den2 * vout);
		
		out[i] = rvout;
	}
#if (TM_UNROLL && TM_UNROLL_FILTERDCNOTCH16)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	mem[0] = mem0;
	mem[1] = mem1;

	FILTERDCNOTCH16_STOP();
}

#define OVERRIDE_MDF_INNER_PROD
float mdf_inner_prod(
	const float * restrict x, 
	const float * restrict y, 
	int len
)
{
	register float sum = 0;
   
	MDFINNERPROD_START();

	len >>= 1;
  
#if (TM_UNROLL && TM_UNROLL_MDFINNERPRODUCT)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	while(len--)
	{
		register float acc0, acc1;

		acc0 = (*x++) * (*y++);
		acc1 = (*x++) * (*y++);

		sum	 += acc0 + acc1;
	}
#if (TM_UNROLL && TM_UNROLL_MDFINNERPRODUCT)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	MDFINNERPROD_STOP();

	return sum;
}

#define OVERRIDE_SPECTRAL_MUL_ACCUM
void spectral_mul_accum(
	const float * restrict X, 
	const float * restrict Y, 
	float * restrict acc, 
	int N, int M
)
{
	register int i, j;
	register float Xi, Yi, Xii, Yii;
	register int _N;

	SPECTRALMULACCUM_START();

	acc[0] = X[0] * Y[0];
	_N = N-1;

	for ( i=1 ; i<_N ; i+=2 )
	{
		Xi = X[i];
		Yi = Y[i];
		Xii = X[i+1];
		Yii = Y[i+1];

		acc[i]	= (Xi  * Yi - Xii * Yii);
		acc[i+1]= (Xii * Yi + Xi  * Yii);
	}
      
	acc[_N] = X[_N] * Y[_N];

	for ( j=1,X+=N,Y+=N ; j<M ; j++ )
	{	
		acc[0] += X[0] * Y[0];

		for ( i=1 ; i<N-1 ; i+=2 )
		{
			Xi = X[i];
			Yi = Y[i];
			Xii = X[i+1];
			Yii = Y[i+1];

			acc[i]	+= (Xi  * Yi - Xii * Yii);
			acc[i+1]+= (Xii * Yi + Xi  * Yii);
		}
      
		acc[_N] += X[_N] * Y[_N];
		X += N;
		Y += N;
	}

	SPECTRALMULACCUM_STOP();
}

#define OVERRIDE_WEIGHTED_SPECTRAL_MUL_CONJ
void weighted_spectral_mul_conj(
	const float * restrict w, 
	const float p, 
	const float * restrict X, 
	const float * restrict Y, 
	float * restrict prod, 
	int N
)
{
	register int i, j;
   	register int _N;

	WEIGHTEDSPECTRALMULCONJ_START();

	prod[0] = p * w[0] * X[0] * Y[0]; 
	_N = N-1;

	for (i=1,j=1;i<_N;i+=2,j++)
	{
		register float W;
		register float Xi, Yi, Xii, Yii;

		Xi = X[i];
		Yi = Y[i];
		Xii = X[i+1];
		Yii = Y[i+1];
		W = p * w[j];


		prod[i]	 = W * (Xi  * Yi + Xii * Yii);
		prod[i+1]= W * (Xi  * Yii - Xii * Yi);
	}
   
	prod[_N] = p * w[j] * X[_N] * Y[_N];

	WEIGHTEDSPECTRALMULCONJ_STOP();
}

#define OVERRIDE_MDF_ADJUST_PROP
void mdf_adjust_prop(
	const float * restrict W, 
	int N, 
	int M, 
	float * restrict prop
)
{
	register int i, j;
	register float max_sum = 1;
	register float prop_sum = 1;

	MDFADJUSTPROP_START();

	for ( i=0 ; i<M ; ++i )
	{
		register float tmp = 1;
		register int k = i * N;
		register int l = k + N;
		register float propi;

#if (TM_UNROLL && TM_UNROLL_MDFADJUSTPROP)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
		for ( j=k ; j<l ; ++j )
		{	
			register float wi = W[j];
			
			tmp += wi * wi;
		}
#if (TM_UNROLL && TM_UNROLL_MDFADJUSTPROP)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

		propi  = spx_sqrt(tmp);
		prop[i]= propi;
		max_sum= fmux(propi > max_sum, propi, max_sum);
	}

	for ( i=0 ; i<M ; ++i )
	{
		register float propi = prop[i];

		propi += .1f * max_sum;
		prop_sum += propi;
		prop[i] = propi;
	}

	prop_sum = 0.99f / prop_sum;
	for ( i=0 ; i<M ; ++i )
	{	prop[i] = prop_sum * prop[i];
	}

	MDFADJUSTPROP_STOP();
}


#define OVERRIDE_SPEEX_ECHO_GET_RESIDUAL
void speex_echo_get_residual(
	SpeexEchoState * restrict st, 
	float * restrict residual_echo, 
	int len
)
{
	register int i;
	register float leak2, leake;
	register int N;
	register float * restrict window;
	register float * restrict last_y;
	register float * restrict y;

	SPEEXECHOGETRESIDUAL_START();

	window = st->window;
	last_y = st->last_y;
	y = st->y;
	N = st->window_size;


#if (TM_UNROLL && TM_UNROLL_SPEEXECHOGETRESIDUAL)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for (i=0;i<N;i++)
	{	y[i] = window[i] * last_y[i];	
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOGETRESIDUAL)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif
      
	spx_fft(st->fft_table, st->y, st->Y);
	power_spectrum(st->Y, residual_echo, N);
      
	leake = st->leak_estimate;
	leak2 = fmux(leake > .5, 1, 2 * leake);
	N = st->frame_size;

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOGETRESIDUAL)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<N ; ++i )
	{	residual_echo[i] *= leak2;
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOGETRESIDUAL)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	residual_echo[N] *= leak2;

#ifndef NO_REMARK
   (void)len;
#endif

   SPEEXECHOGETRESIDUAL_STOP();
}
#endif


void mdf_preemph(
	SpeexEchoState * restrict st, 
	spx_word16_t * restrict x,
	const spx_int16_t * restrict far_end,
	int framesize
)
{
	register spx_word16_t preemph = st->preemph;
	register spx_word16_t memX = st->memX;
	register spx_word16_t memD = st->memD;
	register spx_word16_t * restrict input = st->input;
	register int i;
#ifdef FIXED_POINT
	register int saturated = st->saturated;
#endif

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<framesize ; ++i )
	{
		register spx_int16_t far_endi = far_end[i];
		register spx_word32_t tmp32;
		register spx_word16_t inputi = input[i];

		tmp32 = SUB32(EXTEND32(far_endi), EXTEND32(MULT16_16_P15(preemph,memX)));

#ifdef FIXED_POINT
		saturated = mux(iabs(tmp32) > 32767, M+1, saturated);
		tmp32 = iclipi(tmp32,32767);
#endif

		x[i] = EXTRACT16(tmp32);
		memX = far_endi;
		tmp32 = SUB32(EXTEND32(inputi), EXTEND32(MULT16_16_P15(preemph, memD)));

#ifdef FIXED_POINT
		saturated = mux( ((tmp32 > 32767) && (saturated == 0)), 1,
					mux( ((tmp32 <-32767) && (saturated == 0)), 1, saturated ));
		tmp32 = iclipi(tmp32,32767);
#endif
		memD = inputi;
		input[i] = tmp32;
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	st->memD = memD;
	st->memX = memX;

#ifdef FIXED_POINT
	st->saturated = saturated;
#endif
}

void mdf_sub(
	spx_word16_t * restrict dest,
	const spx_word16_t * restrict src1,
	const spx_word16_t * restrict src2,
	int framesize
)
{
	register int i;

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif

#ifdef FIXED_POINT
	for ( i=0,framesize<<=1 ; i<framesize ; i+=4 )
	{	register int src1i, src2i, desti;

		src1i = ld32d(src1,i);
		src2i = ld32d(src2,i);
		desti = dspidualsub(src1i,src2i);
		st32d(i, dest, desti);
	}
#else
	for ( i=0 ; i<framesize ; ++i )
	{	dest[i] = src1[i] - src2[i];
	}
#endif

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif   
}

void mdf_sub_int(
	spx_word16_t * restrict dest,
	const spx_int16_t * restrict src1,
	const spx_int16_t * restrict src2,
	int framesize
)
{
	register int i, j;

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif

#ifdef FIXED_POINT
	for ( i=0,framesize<<=1 ; i<framesize ; i+=4 )
	{	register int src1i, src2i, desti;

		src1i = ld32d(src1,i);
		src2i = ld32d(src2,i);
		desti = dspidualsub(src1i,src2i);
		st32d(i, dest, desti);
	}
#else
	for ( i=0,j=0 ; i<framesize ; i+=2,++j )
	{	register int src1i, src2i, desti;
		
		
		src1i = ld32d(src1,j);
		src2i = ld32d(src2,j);
		desti = dspidualsub(src1i,src2i);

		dest[i] = sex16(desti);
		dest[i+1] = asri(16,desti);
	}
#endif

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif   
}

void mdf_compute_weight_gradient(
	SpeexEchoState * restrict st,
	spx_word16_t * restrict X,
	int N,
	int M
)
{
	register int i, j;
	register spx_word32_t * restrict PHI = st->PHI;

	for (j=M-1;j>=0;j--)
	{
		register spx_word32_t * restrict W = &(st->W[j*N]);

		weighted_spectral_mul_conj(
			st->power_1, 
			FLOAT_SHL(PSEUDOFLOAT(st->prop[j]),-15), 
			&X[(j+1)*N], 
			st->E, 
			st->PHI, 
			N);
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
		for (i=0;i<N;i++)
		{	W[i] = ADD32(W[i],PHI[i]);
		}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif         
	}
}

void mdf_update_weight(
	SpeexEchoState * restrict st,
	int N,
	int M,
	int framesize
)
{
	register int j;
	register int cancel_count = st->cancel_count;
	register spx_word16_t * restrict wtmp = st->wtmp;
#ifdef FIXED_POINT
	register spx_word16_t * restrict wtmp2 = st->wtmp2;
	register int i;
#endif

	for ( j=0 ; j<M ; j++ )
	{
		register spx_word32_t * restrict W = &(st->W[j*N]);
    
		if (j==0 || cancel_count%(M-1) == j-1)
		{
#ifdef FIXED_POINT

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
			for ( i=0 ; i<N ; i++ )
				wtmp2[i] = EXTRACT16(PSHR32(W[i],NORMALIZE_SCALEDOWN+16));
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif         
			spx_ifft(st->fft_table, wtmp2, wtmp);
			memset(wtmp, 0, framesize * sizeof(spx_word16_t));

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
			for (j=framesize; j<N ; ++j)
			{	wtmp[j]=SHL16(wtmp[j],NORMALIZE_SCALEUP);
			}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif    

			spx_fft(st->fft_table, wtmp, wtmp2);

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
			for (i=0;i<N;i++)
			{	W[i] -= SHL32(EXTEND32(wtmp2[i]),16+NORMALIZE_SCALEDOWN-NORMALIZE_SCALEUP-1);
			}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif    

#else
			spx_ifft(st->fft_table, W, wtmp);
			memset(&wtmp[framesize], 0, (N-framesize) * sizeof(spx_word16_t));
			spx_fft(st->fft_table, wtmp, W);
#endif  
		}
	}
}

#ifdef TWO_PATH
// first four parameters is passed by registers
// generate faster performance with 4 parameters functions
spx_word32_t mdf_update_foreground(
	SpeexEchoState * restrict st,
	spx_word32_t Dbf,
	spx_word32_t Sff,
	spx_word32_t See
)										
{								
	register spx_word32_t Davg1 = st->Davg1;																	
	register spx_word32_t Davg2 = st->Davg2;																	
	register spx_word32_t Dvar1 = st->Dvar1;																	
	register spx_word32_t Dvar2 = st->Dvar2;
	register spx_word16_t * restrict input = st->input;
   	register int framesize = st->frame_size;
	register spx_word16_t * restrict xx = st->x + framesize;
	register spx_word16_t * restrict y = st->y + framesize;
	register spx_word16_t * restrict ee = st->e + framesize;
	register int update_foreground;
	register int i;
    register int N = st->window_size;
	register int M = st->M;
																			
#ifdef FIXED_POINT																								
	register spx_word32_t sc0 = SUB32(Sff,See);																	
	register spx_float_t sc1 = FLOAT_MUL32U(Sff,Dbf);															
																												
	Davg1 = ADD32(MULT16_32_Q15(QCONST16(.6f,15),Davg1), MULT16_32_Q15(QCONST16(.4f,15),sc0));					
	Davg2 = ADD32(MULT16_32_Q15(QCONST16(.85f,15),Davg2), MULT16_32_Q15(QCONST16(.15f,15),sc0));				
	Dvar1 = FLOAT_ADD(																							
				FLOAT_MULT(VAR1_SMOOTH,Dvar1),																	
				FLOAT_MUL32U(MULT16_32_Q15(QCONST16(.4f,15),Sff),												
				MULT16_32_Q15(QCONST16(.4f,15),Dbf)));															
	Dvar2 = FLOAT_ADD(																							
				FLOAT_MULT(VAR2_SMOOTH,Dvar2),																	
				FLOAT_MUL32U(MULT16_32_Q15(QCONST16(.15f,15),Sff),												
				MULT16_32_Q15(QCONST16(.15f,15),Dbf)));															
#else
	register spx_word32_t sc0 = Sff - See;																		
	register spx_word32_t sc1 = Sff * Dbf;																		
																												
	Davg1 = .6*Davg1 + .4*sc0;																					
	Davg2 = .85*Davg2 + .15*sc0;																				
	Dvar1 = VAR1_SMOOTH*Dvar1 + .16*sc1;																		
	Dvar2 = VAR2_SMOOTH*Dvar2 + .0225*sc1;																		
#endif
																												
	update_foreground =																							
	   mux( FLOAT_GT(FLOAT_MUL32U(sc0, VABS(sc0)), sc1), 1,														
	   mux( FLOAT_GT(FLOAT_MUL32U(Davg1, VABS(Davg1)), FLOAT_MULT(VAR1_UPDATE,(Dvar1))), 1,					
	   mux( FLOAT_GT(FLOAT_MUL32U(Davg2, VABS(Davg2)), FLOAT_MULT(VAR2_UPDATE,(Dvar2))), 1, 0)));				

	if ( update_foreground )																					
	{																											
		register spx_word16_t * restrict windowf = st->window + framesize;										
		register spx_word16_t * restrict window = st->window;													
																												
		st->Davg1 = st->Davg2 = 0;																				
		st->Dvar1 = st->Dvar2 = FLOAT_ZERO;																		
																												
		memcpy(st->foreground, st->W, N*M*sizeof(spx_word32_t));												
																												
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)																
#pragma TCS_unroll=4																							
#pragma TCS_unrollexact=1																						
#endif																											
		for ( i=0 ; i<framesize ; ++i)																			
		{	register spx_word16_t wi = window[i];																
			register spx_word16_t wfi = windowf[i];																
			register spx_word16_t ei = ee[i];																	
			register spx_word16_t yi = y[i];																	
																												
			ee[i] = MULT16_16_Q15(wfi,ei) +	MULT16_16_Q15(wi,yi);												
		}																										
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)																
#pragma TCS_unrollexact=0																						
#pragma TCS_unroll=0																							
#endif																											
																												
	} else																										
	{																											
		register int reset_background;																			
																												
		reset_background =																						
			mux( FLOAT_GT(FLOAT_MUL32U(-(sc0),VABS(sc0)), FLOAT_MULT(VAR_BACKTRACK,sc1)), 1,				
			mux( FLOAT_GT(FLOAT_MUL32U(-(Davg1), VABS(Davg1)), FLOAT_MULT(VAR_BACKTRACK,Dvar1)), 1,			
			mux( FLOAT_GT(FLOAT_MUL32U(-(Davg2), VABS(Davg2)), FLOAT_MULT(VAR_BACKTRACK,Dvar2)), 1, 0)));	
																												
		if ( reset_background )																					
		{																										
			memcpy(st->W, st->foreground, N*M*sizeof(spx_word32_t));											
			memcpy(y, ee, framesize * sizeof(spx_word16_t));													
			mdf_sub(xx,input,y,framesize);																		
			See = Sff;																							
			st->Davg1 = st->Davg2 = 0;																			
			st->Dvar1 = st->Dvar2 = FLOAT_ZERO;																	
		} else
		{
			st->Davg1 = Davg1;
			st->Davg2 = Davg2;
			st->Dvar1 = Dvar1;
			st->Dvar2 = Dvar2;
		}
	}

	return See;
}	
#endif
																				   
void mdf_compute_error_signal(
	SpeexEchoState * restrict st,
	const spx_int16_t * restrict in,
	spx_int16_t * restrict out,
	int framesize
)
{
	register spx_word16_t preemph = st->preemph;
	register spx_word16_t memE = st->memE;
	register int saturated = st->saturated;
	register spx_word16_t * restrict e = st->e;
	register spx_word16_t * restrict ee = st->e + framesize;
	register spx_word16_t * restrict input = st->input;
	register spx_word16_t * restrict xx = st->x + framesize;
	register int i;

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<framesize ; ++i )
	{
		register spx_word32_t tmp_out;
		register spx_int16_t ini = in[i];
		register int flg;

#ifdef FIXED_POINT

#ifdef TWO_PATH
		tmp_out = SUB32(EXTEND32(input[i]), EXTEND32(ee[i]));
		tmp_out = iclipi(tmp_out,32767);
#else
		tmp_out = SUB32(EXTEND32(input[i]), EXTEND32(y[i]));
		tmp_out = iclipi(tmp_out,32767);
#endif

#else
#ifdef TWO_PATH
		tmp_out = SUB32(EXTEND32(input[i]), EXTEND32(ee[i]));
#else
		tmp_out = SUB32(EXTEND32(input[i]), EXTEND32(y[i]));
#endif
		tmp_out = 
			fmux( tmp_out > 32767, 32767,
			fmux( tmp_out < -32768, -32768, tmp_out));
#endif

		tmp_out = ADD32(tmp_out, EXTEND32(MULT16_16_P15(preemph,memE)));
		flg = iabs(ini) >= 32000;
		tmp_out = VMUX( flg, 0, tmp_out);
		saturated = mux( flg && (saturated == 0), 1, saturated);

		out[i] = (spx_int16_t)tmp_out;
		memE = tmp_out;
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif 

	st->memE = memE;
	st->saturated = saturated;
	memset(e, 0, framesize * sizeof(spx_word16_t));
	memcpy(ee, xx, framesize * sizeof(spx_word16_t));
}

inline int mdf_check(
  	SpeexEchoState * restrict st,
	spx_int16_t * out,
	spx_word32_t Syy, 
	spx_word32_t Sxx, 
	spx_word32_t See,
	spx_word32_t Sff,
	spx_word32_t Sdd
)
{
    register int N = st->window_size;
	register spx_word32_t N1e9 = N * 1e9;
	register int screwed_up = st->screwed_up;
	register int framesize = st->frame_size;

	if (!(Syy>=0 && Sxx>=0 && See >= 0)
#ifndef FIXED_POINT
		|| !(Sff < N1e9 && Syy < N1e9 && Sxx < N1e9 )
#endif
      )
	{    
		screwed_up += 50;
		memset(out, 0, framesize * sizeof(spx_int16_t));

	} else
	{	screwed_up = mux( SHR32(Sff, 2) > ADD32(Sdd, SHR32(MULT16_16(N, 10000),6)), screwed_up+1, 0);
	}

	st->screwed_up = screwed_up;

	return screwed_up;
}

void mdf_smooth(
	spx_word32_t * restrict power,
	spx_word32_t * restrict Xf,
	int framesize,
	int M
)
{
    register spx_word16_t ss, ss_1, pf, xff;
	register int j;

#ifdef FIXED_POINT
	ss=DIV32_16(11469,M);
	ss_1 = SUB16(32767,ss);
#else
	ss=.35/M;
	ss_1 = 1-ss;
#endif

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif  
	for ( j=0 ; j<framesize ; ++j )
	{	register spx_word32_t pi  = power[j];
		register spx_word32_t xfi = Xf[j];

		power[j] = MULT16_32_Q15(ss_1,pi) + 1 + MULT16_32_Q15(ss,xfi);
	}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif 

	pf = power[framesize];
	xff = Xf[framesize];
	power[framesize] = MULT16_32_Q15(ss_1,pf) + 1 + MULT16_32_Q15(ss,xff);
}

void mdf_compute_filtered_spectra_crosscorrelations(
 	SpeexEchoState * restrict st,
	spx_word32_t Syy, 
	spx_word32_t See,
	int framesize
)
{   
	register spx_float_t Pey = FLOAT_ONE;
	register spx_float_t Pyy = FLOAT_ONE;
	register spx_word16_t spec_average = st->spec_average;
	register spx_word32_t * restrict pRf = st->Rf;
	register spx_word32_t * restrict pYf = st->Yf;
	register spx_word32_t * restrict pEh = st->Eh;
	register spx_word32_t * restrict pYh = st->Yh;
	register spx_word16_t beta0 = st->beta0;
	register spx_word16_t beta_max = st->beta_max;
	register spx_float_t alpha, alpha_1;
	register spx_word32_t tmp32, tmpx;
	register spx_float_t sPey = st->Pey;
	register spx_float_t sPyy = st->Pyy;
	register spx_float_t tmp;
	register spx_word16_t leak_estimate;
	register int j;
	register spx_float_t  Eh, Yh;
	register spx_word32_t _Ehj, _Rfj, _Yfj, _Yhj;

#ifdef FIXED_POINT
	register spx_word16_t spec_average1 = SUB16(32767,spec_average);
#else
	register spx_word16_t spec_average1 = 1 - spec_average;
#endif

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif  
	for (j=framesize; j>0 ; --j)
	{
		_Ehj = pEh[j];
		_Rfj = pRf[j];
		_Yfj = pYf[j];
		_Yhj = pYh[j];

		Eh = PSEUDOFLOAT(_Rfj - _Ehj);
		Yh = PSEUDOFLOAT(_Yfj - _Yhj);

		Pey = FLOAT_ADD(Pey,FLOAT_MULT(Eh,Yh));
		Pyy = FLOAT_ADD(Pyy,FLOAT_MULT(Yh,Yh));

		pEh[j] = MAC16_32_Q15(MULT16_32_Q15(spec_average1, _Ehj), spec_average, _Rfj);
		pYh[j] = MAC16_32_Q15(MULT16_32_Q15(spec_average1, _Yhj), spec_average, _Yfj);
   }
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif 
	_Ehj = pEh[0];
	_Rfj = pRf[0];
	_Yfj = pYf[0];
	_Yhj = pYh[0];

	Eh = PSEUDOFLOAT(_Rfj - _Ehj);
	Yh = PSEUDOFLOAT(_Yfj - _Yhj);

	Pey = FLOAT_ADD(Pey,FLOAT_MULT(Eh,Yh));
	Pyy = FLOAT_ADD(Pyy,FLOAT_MULT(Yh,Yh));

	pEh[0] = MAC16_32_Q15(MULT16_32_Q15(spec_average1, _Ehj), spec_average, _Rfj);
	pYh[0] = MAC16_32_Q15(MULT16_32_Q15(spec_average1, _Yhj), spec_average, _Yfj);

	Pyy = FLOAT_SQRT(Pyy);
	Pey = FLOAT_DIVU(Pey,Pyy);

	tmp32 = MULT16_32_Q15(beta0,Syy);
	tmpx = MULT16_32_Q15(beta_max,See);
	tmp32 = VMUX(tmp32 > tmpx, tmpx, tmp32);
	alpha = FLOAT_DIV32(tmp32, See);
	alpha_1 = FLOAT_SUB(FLOAT_ONE, alpha);
   
	sPey = FLOAT_ADD(FLOAT_MULT(alpha_1,sPey) , FLOAT_MULT(alpha,Pey));
	sPyy = FLOAT_ADD(FLOAT_MULT(alpha_1,sPyy) , FLOAT_MULT(alpha,Pyy));   
	tmp = FLOAT_MULT(MIN_LEAK,sPyy);

#ifndef FIXED_POINT
	sPyy = VMUX(FLOAT_LT(sPyy, FLOAT_ONE), FLOAT_ONE, sPyy);
	sPey = VMUX(FLOAT_LT(sPey, tmp), tmp, sPey);
	sPey = VMUX(FLOAT_LT(sPey, sPyy), sPey, sPyy); 
#else
	sPyy = FLOAT_LT(sPyy, FLOAT_ONE) ? FLOAT_ONE : sPyy;
	sPey = FLOAT_LT(sPey, tmp) ? tmp : sPey;
	sPey = FLOAT_LT(sPey, sPyy) ? sPey : sPyy; 
#endif
	
	leak_estimate = FLOAT_EXTRACT16(FLOAT_SHL(FLOAT_DIVU(sPey, sPyy),14));

	leak_estimate = VMUX( leak_estimate > 16383, 32767, SHL16(leak_estimate,1));
	st->Pey = sPey;
	st->Pyy = sPyy;
	st->leak_estimate = leak_estimate;
}

inline spx_word16_t mdf_compute_RER(
 	spx_word32_t See,
	spx_word32_t Syy,
	spx_word32_t Sey,
	spx_word32_t Sxx,
	spx_word16_t leake
)
{
	register spx_word16_t RER;

#ifdef FIXED_POINT
	register spx_word32_t tmp32;
	register spx_word32_t tmp;
	spx_float_t bound = PSEUDOFLOAT(Sey);
	      
	tmp32 = MULT16_32_Q15(leake,Syy);
	tmp32 = ADD32(SHR32(Sxx,13), ADD32(tmp32, SHL32(tmp32,1)));
   
	bound = FLOAT_DIVU(FLOAT_MULT(bound, bound), PSEUDOFLOAT(ADD32(1,Syy)));
	tmp   = FLOAT_EXTRACT32(bound);
	tmp32 = imux( FLOAT_GT(bound, PSEUDOFLOAT(See)), See,
			imux( tmp32 < tmp,  tmp, tmp32));

	tmp   = SHR32(See,1);
	tmp32 = imux(tmp32 > tmp, tmp, tmp32);
	RER   = FLOAT_EXTRACT16(FLOAT_SHL(FLOAT_DIV32(tmp32,See),15));
#else
	register spx_word32_t r0;

	r0 = (Sey * Sey)/(1 + See * Syy);

	RER = (.0001*Sxx + 3.* MULT16_32_Q15(leake,Syy)) / See;
	RER = fmux( RER < r0, r0, RER);
	RER = fmux( RER > .5, .5, RER);
#endif

	return RER;
}

void mdf_adapt(
 	SpeexEchoState * restrict st,
	spx_word16_t RER,
	spx_word32_t Syy,
	spx_word32_t See, 
	spx_word32_t Sxx
)
{
	register spx_float_t  * restrict power_1 = st->power_1;
	register spx_word32_t  * restrict power = st->power;
	register int adapted = st->adapted;
	register spx_word32_t sum_adapt = st->sum_adapt;
	register spx_word16_t leake = st->leak_estimate;
	register int framesize = st->frame_size;
	register int i;
	register int M = st->M;

	adapted = mux( !adapted && sum_adapt > QCONST32(M,15) && 
					MULT16_32_Q15(leake,Syy) > MULT16_32_Q15(QCONST16(.03f,15),Syy), 1, adapted);

	if ( adapted )
	{	register spx_word32_t * restrict Yf = st->Yf; 
		register spx_word32_t * restrict Rf = st->Rf;
		register spx_word32_t r, e, e2;

#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif  
		for ( i=0 ; i<framesize ; ++i )
		{	
			r = SHL32(Yf[i],3);
			r = MULT16_32_Q15(leake,r);
			e = SHL32(Rf[i],3)+1;

#ifdef FIXED_POINT
			e2 = SHR32(e,1);
			r = mux( r > e2, e2, r);
#else
			e2 = e * .5;
			r = fmux( r > e2, e2, r);
#endif

			r = MULT16_32_Q15(QCONST16(.7,15),r) + 
				MULT16_32_Q15(QCONST16(.3,15),(spx_word32_t)(MULT16_32_Q15(RER,e)));

			power_1[i] = FLOAT_SHL(FLOAT_DIV32_FLOAT(r,FLOAT_MUL32U(e,power[i]+10)),WEIGHT_SHIFT+16);
		}
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif 
		
		r = SHL32(Yf[framesize],3);
		r = MULT16_32_Q15(leake,r);
		e = SHL32(Rf[framesize],3)+1;

#ifdef FIXED_POINT
		e2 = SHR32(e,1);
		r = mux( r > e2, e2, r);
#else
		e2 = e * .5;
		r = fmux( r > e2, e2, r);
#endif

		r = MULT16_32_Q15(QCONST16(.7,15),r) + 
			MULT16_32_Q15(QCONST16(.3,15),(spx_word32_t)(MULT16_32_Q15(RER,e)));

		power_1[framesize] = FLOAT_SHL(FLOAT_DIV32_FLOAT(r,FLOAT_MUL32U(e,power[framesize]+10)),WEIGHT_SHIFT+16);

	} else 
	{
		register spx_word16_t adapt_rate=0;
		register int N = st->window_size;

		if ( Sxx > SHR32(MULT16_16(N, 1000),6) ) 
		{	register spx_word32_t tmp32, tmp32q;
         
			tmp32 = MULT16_32_Q15(QCONST16(.25f, 15), Sxx);
#ifdef FIXED_POINT
			tmp32q = SHR32(See,2);
			tmp32  = mux(tmp32 > tmp32q, tmp32q, tmp32);
#else
			tmp32q = 0.25 * See;
			tmp32 = fmux(tmp32 > tmp32q, tmp32q, tmp32);
#endif
			adapt_rate = FLOAT_EXTRACT16(FLOAT_SHL(FLOAT_DIV32(tmp32, See),15));
		}
      
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif  		
		for (i=0;i<framesize;i++)
			power_1[i] = FLOAT_SHL(FLOAT_DIV32(EXTEND32(adapt_rate),ADD32(power[i],10)),WEIGHT_SHIFT+1);
#if (TM_UNROLL && TM_UNROLL_SPEEXECHOCANCELLATION)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif 
		power_1[framesize] = FLOAT_SHL(FLOAT_DIV32(EXTEND32(adapt_rate),ADD32(power[framesize],10)),WEIGHT_SHIFT+1);
		sum_adapt = ADD32(sum_adapt,adapt_rate);
	}
	
	st->sum_adapt = sum_adapt;
	st->adapted = adapted;
}

#define OVERRIDE_ECHO_CANCELLATION
void speex_echo_cancellation(
	SpeexEchoState * restrict st, 
	const spx_int16_t * restrict in, 
	const spx_int16_t * restrict far_end, 
	spx_int16_t * restrict out
)
{ 
   	register int framesize = st->frame_size;
   	register spx_word16_t * restrict x = st->x;
	register spx_word16_t * restrict xx = st->x + framesize;
	register spx_word16_t * restrict yy = st->y + framesize;
	register spx_word16_t * restrict ee = st->e + framesize;
	register spx_word32_t Syy, See, Sxx, Sdd, Sff;
	register spx_word16_t RER;
	register spx_word32_t Sey;
	register int j;
	register int N,M;
#ifdef TWO_PATH
	register spx_word32_t Dbf;
#endif

	N = st->window_size;
	M = st->M;
	st->cancel_count++;

	filter_dc_notch16(in, st->notch_radius, st->input, framesize, st->notch_mem);
	mdf_preemph(st, xx, far_end, framesize);
   
   	{

	register spx_word16_t * restrict X = st->X;

	for ( j=M-1 ; j>=0 ; j-- )
	{	register spx_word16_t * restrict Xdes = &(X[(j+1)*N]);
		register spx_word16_t * restrict Xsrc = &(X[j*N]);

		memcpy(Xdes, Xsrc, N * sizeof(spx_word16_t));
	}

	spx_fft(st->fft_table, x, X);
	memcpy(st->last_y, st->x, N * sizeof(spx_word16_t));
	Sxx = mdf_inner_prod(xx, xx, framesize);
	memcpy(x, xx, framesize * sizeof(spx_word16_t));
	
#ifdef TWO_PATH
	spectral_mul_accum(st->X, st->foreground, st->Y, N, M);   
	spx_ifft(st->fft_table, st->Y, st->e);
	mdf_sub(xx, st->input, ee, framesize);
	Sff = mdf_inner_prod(xx, xx, framesize);
#endif
   
	mdf_adjust_prop (st->W, N, M, st->prop);
  
	if (st->saturated == 0)
	{	mdf_compute_weight_gradient(st, X, N, M);
	} else 
	{	st->saturated--;
	}
	}

	mdf_update_weight(st, N, M, framesize);
	spectral_mul_accum(st->X, st->W, st->Y, N, M);
	spx_ifft(st->fft_table, st->Y, st->y);

#ifdef TWO_PATH
	mdf_sub(xx, ee, yy, framesize);   
	Dbf = 10+mdf_inner_prod(xx, xx, framesize);
#endif

	mdf_sub(xx, st->input, yy, framesize);
	See = mdf_inner_prod(xx, xx, framesize);

#ifndef TWO_PATH
	Sff = See;
#else
	See = mdf_update_foreground(st,Dbf,Sff,See);
#endif

	
	mdf_compute_error_signal(st, in, out, framesize);
	Sey = mdf_inner_prod(ee, yy, framesize);
	Syy = mdf_inner_prod(yy, yy, framesize);
	Sdd = mdf_inner_prod(st->input, st->input, framesize);
   
	if ( mdf_check(st,out,Syy,Sxx,See,Sff,Sdd) >= 50 )
	{	speex_warning("The echo canceller started acting funny and got slapped (reset). It swears it will behave now.");
		speex_echo_state_reset(st);
		return;
	}

	See = MAX32(See, SHR32(MULT16_16(N, 100),6));
	spx_fft(st->fft_table, st->e, st->E);
	memset(st->y, 0, framesize * sizeof(spx_word16_t));
	spx_fft(st->fft_table, st->y, st->Y);
	power_spectrum(st->E, st->Rf, N);
	power_spectrum(st->Y, st->Yf, N);
	power_spectrum(st->X, st->Xf, N);

	mdf_smooth(st->power,st->Xf,framesize, M);
	mdf_compute_filtered_spectra_crosscorrelations(st,Syy,See,framesize);
	RER = mdf_compute_RER(See,Syy,Sey,Sxx,st->leak_estimate);
	mdf_adapt(st, RER, Syy, See, Sxx);
	
	if ( st->adapted )
	{	register spx_word16_t * restrict last_yy = st->last_y + framesize;
		
		memcpy(st->last_y, last_yy, framesize * sizeof(spx_word16_t));
		mdf_sub_int(last_yy, in, out, framesize);

	}
}



