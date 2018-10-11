/* Copyright (C) 2007 Hong Zhiqian */
/**
   @file fftwrap_tm.h
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

#define OVERRIDE_MAXIMIZE_RANGE
static int maximize_range(Int16 *in, Int16 *out, int bound, int len)
{
   	register int max_val=0;
	register int shift=0;
	register int i, j;

	TMDEBUG_ALIGNMEM(in);
	TMDEBUG_ALIGNMEM(out);

	MAXIMIZERANGE_START();

	len >>= 1;

	for ( i=0 ; i<len ; i+=4 )
	{
		register int x10, x32, x54, x76;

		x10 = ld32x(in,i);
		x32 = ld32x(in,i+1);
		x54 = ld32x(in,i+2);
		x76 = ld32x(in,i+3);
		
		x10 = dspidualabs(x10);
		x32 = dspidualabs(x32);
		x54 = dspidualabs(x54);
		x76 = dspidualabs(x76);
		
		x10 = imax(sex16(x10), asri(16,x10));
		x32 = imax(sex16(x32), asri(16,x32));
		x54 = imax(sex16(x54), asri(16,x54));
		x76 = imax(sex16(x76), asri(16,x76));

		max_val = imax(max_val,x10);
		max_val = imax(max_val,x32);
		max_val = imax(max_val,x54);
		max_val = imax(max_val,x76);
	}

	while ( max_val <= (bound>>1) && max_val != 0 )
	{	max_val <<= 1;
		shift++;
	}

	if ( shift != 0 )
	{
		for ( i=0,j=0 ; i<len ; i+=4,j+=16 )
		{
			register int x10, x32, x54, x76;

			x10 = ld32x(in,i);
			x32 = ld32x(in,i+1);
			x54 = ld32x(in,i+2);
			x76 = ld32x(in,i+3);

			x10 = dualasl(x10, shift);
			x32 = dualasl(x32, shift);
			x54 = dualasl(x54, shift);
			x76 = dualasl(x76, shift);

			st32d(j,out,x10);
			st32d(j+4,out,x32);
			st32d(j+8,out,x54);
			st32d(j+12,out,x76);
		}   
	}

	MAXIMIZERANGE_STOP();

	return shift;
}

#define OVERRIDE_RENORM_RANGE
static void renorm_range(Int16 *in, Int16 *out, int shift, int len)
{
	register int i, j, s, l;

	TMDEBUG_ALIGNMEM(in);
	TMDEBUG_ALIGNMEM(out);

	RENORMRANGE_START();

	s = (1<<((shift))>>1);
	s = pack16lsb(s,s);

	len >>= 1;
	l = len & (int)0xFFFFFFFE;

   	for ( i=0,j=0 ; i<l; i+=2,j+=8 )
	{
		register int x10, x32;

		x10 = ld32x(in,i);
		x32 = ld32x(in,i+1);

		x10 = dspidualadd(x10, s);
		x32 = dspidualadd(x32, s);

		x10 = dualasr(x10, shift);
		x32 = dualasr(x32, shift);

		st32d(j,out,x10);
		st32d(j+4,out,x32);
	}

	if ( len & (int)0x01 )
	{
		register int x10;

		x10 = ld32x(in,i);
		x10 = dspidualadd(x10, s);
		x10 = dualasr(x10, shift);
		st32d(j,out,x10);
	}

	RENORMRANGE_STOP();
}

#endif

#ifdef USE_COMPACT_KISS_FFT 
#ifdef FIXED_POINT

#define OVERRIDE_POWER_SPECTRUM
void power_spectrum(const spx_word16_t *X, spx_word32_t *ps, int N)
{
	register int x10, x32, x54, x76, *x;
	register int i;

	x = (int*)(X-1);

	TMDEBUG_ALIGNMEM(x);

	POWERSPECTRUM_START();

	x76 = 0;
	ps[0] = MULT16_16(X[0],X[0]);
	N >>= 1;
   
	for( i=1 ; i<N ; i+=4 )
	{	
		x10 = ld32x(x, i);
		x32 = ld32x(x, i+1);
		x54 = ld32x(x, i+2);
		x76 = ld32x(x, i+3);

		ps[i]	= ifir16(x10,x10);
		ps[i+1] = ifir16(x32,x32);
		ps[i+2] = ifir16(x54,x54);
		ps[i+3] = ifir16(x76,x76);
	}

	x76 = sex16(x76);
	ps[N] = x76 * x76;

	POWERSPECTRUM_STOP();
}

#else

#define OVERRIDE_POWER_SPECTRUM
void power_spectrum(const float * restrict X, float * restrict ps, int N)
{
	register int i, j;
	register float xx;
   
	POWERSPECTRUM_START();

	xx = X[0];

	ps[0]=MULT16_16(xx,xx);

#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
   for (i=1,j=1;i<N-1;i+=2,j++)
   {	register float xi, xii;

		xi = X[i];
		xii = X[i+1];
      
		ps[j] =  MULT16_16(xi,xi) + MULT16_16(xii,xii);
   }
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0

   xx = X[i];
   ps[j]=MULT16_16(xx,xx);

   POWERSPECTRUM_STOP();
}

#endif
#endif

