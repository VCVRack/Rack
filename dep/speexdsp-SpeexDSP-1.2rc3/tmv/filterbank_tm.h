/* Copyright (C) 2007 Hong Zhiqian */
/**
   @file filterbank_tm.h
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

#define OVERRIDE_FILTERBANK_COMPUTE_BANK32
void filterbank_compute_bank32(FilterBank * restrict bank, spx_word32_t * restrict ps, spx_word32_t * restrict mel)
{
	register int i, j, k, banks, len, zero, s;
	register int * restrict left;
	register int * restrict right; 
	register int * restrict bleft;
	register int * restrict bright;

	left = (int*)bank->filter_left;
	right = (int*)bank->filter_right;
	bleft = (int*)bank->bank_left;
	bright = (int*)bank->bank_right;

   	TMDEBUG_ALIGNMEM(ps);
	TMDEBUG_ALIGNMEM(mel);
	TMDEBUG_ALIGNMEM(left);
	TMDEBUG_ALIGNMEM(right);
	TMDEBUG_ALIGNMEM(bleft);
	TMDEBUG_ALIGNMEM(bright);

	FILTERBANKCOMPUTEBANK32_START();

	banks = bank->nb_banks << 2;
	zero = 0;
	len = bank->len;
	s = (1<<((15))>>1);

#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEBANK32)
#pragma TCS_unroll=2
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<banks ; i+=4 )
	{	st32d(i, mel, zero);
	}
#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEBANK32)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEBANK32)
#pragma TCS_unroll=2
#pragma TCS_unrollexact=1
#endif
   for ( i=0,j=1,k=0 ; i<len ; i+=2,j+=2,++k )
   {	register int ps1, ps0, _mel, ps0_msb, ps0_lsb, ps1_msb, ps1_lsb; 
		register int left10, right10, left1, left0, right1, right0;
		register int il1, ir1, il0, ir0;

		ps0		= ld32x(ps,i);
		il0		= ld32x(bleft,i);
		_mel	= ld32x(mel,il0);
		left10	= ld32x(left,k);		
		ir0		= ld32x(bright,i);
		right10	= ld32x(right,k);

		ps0_msb	= ps0 >> 15;
		ps0_lsb = ps0 & 0x00007fff;
		left0	= sex16(left10);
		right0	= sex16(right10);

		_mel	+= left0 * ps0_msb + ((left0 * ps0_lsb + s ) >> 15);
		mel[il0]= _mel;
		_mel	= ld32x(mel,ir0);
		_mel	+= right0 * ps0_msb + ((right0 * ps0_lsb + s ) >> 15);
		mel[ir0]= _mel;

		ps1		= ld32x(ps,j);
		il1		= ld32x(bleft,j);
		_mel	= ld32x(mel,il1);
		ir1		= ld32x(bright,j);

		left1	= asri(16,left10);
		right1	= asri(16,right10);
		ps1_msb	= ps1 >> 15;
		ps1_lsb	= ps1 & 0x00007fff;

		_mel	+= left1 * ps1_msb + ((left1 * ps1_lsb + s ) >> 15);
		mel[il1]= _mel;
		_mel	= ld32x(mel,ir1);
		_mel	+= right1 * ps1_msb + ((right1 * ps1_lsb + s ) >> 15);
		mel[ir1]= _mel;
   }
#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEBANK32)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

   FILTERBANKCOMPUTEBANK32_STOP();
}

#define OVERRIDE_FILTERBANK_COMPUTE_PSD16
void filterbank_compute_psd16(FilterBank * restrict bank, spx_word16_t * restrict mel, spx_word16_t * restrict ps)
{
	register int i, j, k, len, s;
	register int * restrict left;
	register int * restrict right; 
	register int * restrict bleft;
	register int * restrict bright;

	left = (int*)bank->filter_left;
	right = (int*)bank->filter_right;
	bleft = (int*)bank->bank_left;
	bright = (int*)bank->bank_right;

   	TMDEBUG_ALIGNMEM(ps);
	TMDEBUG_ALIGNMEM(mel);
	TMDEBUG_ALIGNMEM(left);
	TMDEBUG_ALIGNMEM(right);
	TMDEBUG_ALIGNMEM(bleft);
	TMDEBUG_ALIGNMEM(bright);

	FILTERBANKCOMPUTEPSD16_START();

	len = bank->len;
	s = (1<<((15))>>1);

#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEPSD16)
#pragma TCS_unroll=2
#pragma TCS_unrollexact=1
#endif
	for ( i=0,j=0,k=0 ; i<len ; i+=2,j+=4,++k )
	{
		register int mell0, melr0, mel1, mel0, mell1, melr1;
		register int il1, ir1, il0, ir0;
		register int left10, right10, lr1, lr0;
		register int acc0, acc1, ps10;

		acc0 = acc1 = s;

		il0		= ld32x(bleft, i);
		ir0		= ld32x(bright,i);
		mell0	= mel[il0]; 
		melr0	= mel[ir0];
		left10	= ld32x(left,  k);
		right10 = ld32x(right, k);
		mel0	= pack16lsb(mell0, melr0);
		lr0		= pack16lsb(left10, right10);
		
		acc0	+= ifir16(mel0, lr0);
		acc0	>>= 15;
		
		il1		= ld32x(bleft, i+1);
		ir1		= ld32x(bright,i+1);
		mell1	= mel[il1];
		melr1	= mel[ir1];
		mel1	= pack16lsb(mell1, melr1);
		lr1		= pack16msb(left10, right10);
			
		acc1	+= ifir16(mel1, lr1);
		acc1	>>= 15;

		ps10	= pack16lsb(acc1, acc0);
		
		st32d(j, ps, ps10);
	}
#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEPSD16)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	FILTERBANKCOMPUTEPSD16_STOP();
}

#else

#define OVERRIDE_FILTERBANK_COMPUTE_BANK32
void filterbank_compute_bank32(FilterBank * restrict bank, float * restrict ps, float * restrict mel)
{
	register int i, banks, len;
	register int * restrict bleft, * restrict bright;
	register float * restrict left, * restrict right;

	banks = bank->nb_banks;
	len	 = bank->len;
	bleft = bank->bank_left;
	bright= bank->bank_right;
	left	 = bank->filter_left;
	right = bank->filter_right;

	FILTERBANKCOMPUTEBANK32_START();

	memset(mel, 0, banks * sizeof(float));

#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEBANK32)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
   for ( i=0 ; i<len ; ++i)
   {
      register int id1, id2;
	  register float psi;

      id1 = bleft[i];
      id2 = bright[i];
      psi = ps[i];
	  
	  mel[id1] += left[i] * psi;
      mel[id2] += right[i] * psi;
   }
#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEBANK32)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

   FILTERBANKCOMPUTEBANK32_STOP();
}

#define OVERRIDE_FILTERBANK_COMPUTE_PSD16
void filterbank_compute_psd16(FilterBank * restrict bank, float * restrict mel, float * restrict ps)
{
	register int i, len;
	register int * restrict bleft, * restrict bright;
	register float * restrict left, * restrict right;

	len	  = bank->len;
	bleft = bank->bank_left;
	bright= bank->bank_right;
	left  = bank->filter_left;
	right = bank->filter_right;

	FILTERBANKCOMPUTEPSD16_START();

#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEPSD16)
#pragma TCS_unroll=4
#pragma TCS_unrollexact=1
#endif
	for ( i=0 ; i<len ; ++i )
	{
		register float acc;
		register int id1, id2;

		id1 = bleft[i];
		id2 = bright[i];

		acc = mel[id1] * left[i];
		acc += mel[id2] * right[i];
		
		ps[i] = acc;
	}
#if (TM_UNROLL && TM_UNROLL_FILTERBANKCOMPUTEPSD16)
#pragma TCS_unrollexact=0
#pragma TCS_unroll=0
#endif

	 FILTERBANKCOMPUTEPSD16_STOP();
}


#endif
