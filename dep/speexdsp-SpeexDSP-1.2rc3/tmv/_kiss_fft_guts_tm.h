/* Copyright (C) 2007 Hong Zhiqian */
/**
   @file _kiss_fft_guts_tm.h
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

#ifndef _KISS_FFT_GUTS_TM_
#define _KISS_FFT_GUTS_TM_

#ifdef TM_ASM

#include <ops/custom_defs.h>

#ifdef FIXED_POINT

#undef	sround
#define sround(x)	sex16(((x) + (1<<(FRACBITS-1)) ) >> FRACBITS)

#undef	MIN
#undef	MAX
#define MIN(a,b)	imin(a,b)
#define	MAX(a,b)	imax(a,b)

#define TM_MUL(res,a,b)																\
	{	register int a0, a1, b0, b1;												\
																					\
		a0	 = sex16((a));															\
		a1	 = asri(16,(a));														\
		b0	 = sex16((b));															\
		b1	 = asri(16,(b));														\
		(res)= pack16lsb(															\
			sround(ifir16((a),funshift2((b),(b)))),									\
			sround(a0*b0-a1*b1));													\
	}																				\

#define TM_ADD(res,a,b)																\
	{	(res)=dspidualadd((a),(b));													\
	}																				\

#define TM_SUB(res,a,b)																\
	{	(res)=dspidualsub((a),(b));													\
	}																				\

#define TM_SHR(res,a,shift)															\
	{	(res)=dualasr((a),(shift));													\
	}																				\

#define TM_DIV(res,c,frac)															\
	{	register int c1, c0;														\
																					\
		c1 = asri(16,(c));															\
		c0 = sex16((c));															\
		(res) = pack16lsb(sround(c1 * (32767/(frac))), sround(c0 * (32767/(frac))));\
	}																				\

#define TM_NEGMSB(res, a)															\
	{	(res) = pack16lsb((ineg(asri(16,(a)))), (a));								\
	}																				\

#else

#undef	MIN
#undef	MAX
#define MIN(a,b)	fmin(a,b)
#define MAX(a,b)	fmax(a,b)

#endif
#endif

#undef	CHECKBUF
#define CHECKBUF(buf,nbuf,n)													\
    {																			\
        if ( nbuf < (size_t)(n) ) {												\
            speex_free(buf);													\
            buf = (kiss_fft_cpx*)KISS_FFT_MALLOC(sizeof(kiss_fft_cpx)*(n)); 	\
            nbuf = (size_t)(n);													\
        }																		\
	}																			\

#undef	 C_ADD
#define  C_ADD( res, a,b)														\
   {																			\
	    CHECK_OVERFLOW_OP((a).r,+,(b).r)										\
	    CHECK_OVERFLOW_OP((a).i,+,(b).i)										\
	    (res).r=(a).r+(b).r;  (res).i=(a).i+(b).i;								\
    }																			\


#undef	 C_SUB
#define  C_SUB( res, a,b)														\
    {																			\
	    CHECK_OVERFLOW_OP((a).r,-,(b).r)										\
	    CHECK_OVERFLOW_OP((a).i,-,(b).i)										\
	    (res).r=(a).r-(b).r;  (res).i=(a).i-(b).i;								\
    }																			\

#undef	C_ADDTO
#define C_ADDTO( res , a)														\
    {																			\
	    CHECK_OVERFLOW_OP((res).r,+,(a).r)										\
	    CHECK_OVERFLOW_OP((res).i,+,(a).i)										\
	    (res).r += (a).r;  (res).i += (a).i;									\
    }																			\

#undef	C_SUBFROM
#define C_SUBFROM( res, a)														\
    {																			\
	    CHECK_OVERFLOW_OP((res).r,-,(a).r)										\
	    CHECK_OVERFLOW_OP((res).i,-,(a).i)										\
	    (res).r -= (a).r;  (res).i -= (a).i;									\
	}																			\

#undef	 kf_cexp
#define  kf_cexp(x,phase)														\
	{	(x)->r = KISS_FFT_COS(phase);											\
		(x)->i = KISS_FFT_SIN(phase); }											\

#undef	 kf_cexp2
#define  kf_cexp2(x,phase)														\
    {	(x)->r = spx_cos_norm((phase));											\
		(x)->i = spx_cos_norm((phase)-32768); }									\


#ifdef FIXED_POINT

#undef	C_MUL
#define C_MUL(m,a,b)															\
	{	(m).r = sround( smul((a).r,(b).r) - smul((a).i,(b).i) );				\
		(m).i = sround( smul((a).r,(b).i) + smul((a).i,(b).r) ); }				\

#undef	C_FIXDIV
#define C_FIXDIV(c,div)															\
	{   DIVSCALAR( (c).r , div);												\
		DIVSCALAR( (c).i  , div); }												\

#undef	C_MULBYSCALAR
#define C_MULBYSCALAR( c, s )													\
    {	(c).r =  sround( smul( (c).r , s ) ) ;									\
        (c).i =  sround( smul( (c).i , s ) ) ; }								\

#else

#undef	C_MUL
#define C_MUL(m,a,b)															\
	{	(m).r = (a).r*(b).r - (a).i*(b).i;										\
        (m).i = (a).r*(b).i + (a).i*(b).r; }									\


#undef	C_MULBYSCALAR
#define C_MULBYSCALAR( c, s )													\
    {	(c).r *= (s);															\
        (c).i *= (s); }															\



#endif

#endif

