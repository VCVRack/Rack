/* Copyright (C) 2007 Hong Zhiqian */
/**
   @file kiss_fftr_tm.h
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
#include "_kiss_fft_guts_tm.h"

#ifdef TM_ASM

#include "profile_tm.h"

#ifdef FIXED_POINT

#define TM_NDIV(res,c,frac)															\
	{	register int c1, c0;														\
																					\
		c1 = -asri(16,(c));															\
		c0 = sex16((c));															\
		(res) = pack16lsb(sround(c1 * (32767/(frac))), sround(c0 * (32767/(frac))));\
	}	


#define OVERRIDE_KISS_FFTR
void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar * restrict timedata, kiss_fft_cpx * restrict freqdata)
{
	register int ncfft, ncfft2, k;
	register int * restrict tmpbuf;
	register int * restrict twiddles;

	ncfft = st->substate->nfft;
	ncfft2 = ncfft >> 1;
	tmpbuf = (int*)st->tmpbuf;
	twiddles = (int*)st->super_twiddles;

	TMDEBUG_ALIGNMEM(timedata);
	TMDEBUG_ALIGNMEM(freqdata);
	TMDEBUG_ALIGNMEM(tmpbuf);
	TMDEBUG_ALIGNMEM(twiddles);

	kiss_fft(st->substate , (const kiss_fft_cpx*)timedata, st->tmpbuf);

	 {
		register int tdcr, tdci;
		tdcr = sround(st->tmpbuf[0].r * (32767/2));
		tdci = sround(st->tmpbuf[0].i * (32767/2));

		freqdata[0].r = tdcr + tdci;
		freqdata[ncfft].r = tdcr - tdci;
		freqdata[ncfft].i = freqdata[0].i = 0;
	 }

	 for ( k=1 ; k <= ncfft2 ; ++k ) 
	 {	
		register int fpk, fpnk, i, tw, f1k, f2k;
		register int fq1, fq2;

		i = ncfft-k;

		fpk  = ld32x(tmpbuf,k);
		tw   = ld32x(twiddles,k);
		fpnk = ld32x(tmpbuf,i);

		TM_DIV(fpk, fpk, 2);
		TM_NDIV(fpnk,fpnk,2);
 
        TM_ADD( f1k, fpk , fpnk );
        TM_SUB( f2k, fpk , fpnk );
		TM_MUL( tw , f2k, tw );
		TM_ADD( fq1, f1k, tw );
		TM_SHR( fq1, fq1, 1  );
		TM_SUB( fq2, f1k, tw );
		TM_NEGMSB( fq2, fq2 );
		TM_SHR( fq2, fq2, 1 );


		st32d( k<<2, freqdata, fq1 );
		st32d( i<<2, freqdata, fq2 );
    }
}

#define OVERRIDE_KISS_FFTRI
void kiss_fftri(kiss_fftr_cfg st,const kiss_fft_cpx * restrict freqdata,kiss_fft_scalar * restrict timedata)
{
	register int k, ncfft, ncfft2;
	register int * restrict tmpbuf;
	register int * restrict twiddles;

    ncfft = st->substate->nfft;
	ncfft2 = ncfft >> 1;
	tmpbuf = (int*)st->tmpbuf;
	twiddles = (int*)st->super_twiddles;

	TMDEBUG_ALIGNMEM(freqdata);
	TMDEBUG_ALIGNMEM(timedata);
	TMDEBUG_ALIGNMEM(tmpbuf);
	TMDEBUG_ALIGNMEM(twiddles);

	{
		register int fqr, fqnr;

		fqr  = freqdata[0].r;
		fqnr = freqdata[ncfft].r;

		st->tmpbuf[0].r = fqr + fqnr;
		st->tmpbuf[0].i = fqr - fqnr;
	}

    for ( k=1 ; k <= ncfft2 ; ++k ) 
	{
		register int fk, fnkc, i, tw, fek, fok, tmp;
		register int tbk, tbn;

		i = ncfft-k;

		fk = ld32x(freqdata,k);
		tw = ld32x(twiddles,k);
		fnkc = pack16lsb(-freqdata[i].i, freqdata[i].r);
	
        TM_ADD (fek, fk, fnkc);
        TM_SUB (tmp, fk, fnkc);
        TM_MUL (fok, tmp, tw );
		TM_ADD (tbk, fek, fok);
		TM_SUB (tbn, fek, fok);
		TM_NEGMSB(tbn, tbn);

		st32d(k<<2, tmpbuf, tbk);
		st32d(i<<2, tmpbuf, tbn);
    }
    kiss_fft (st->substate, st->tmpbuf, (kiss_fft_cpx *) timedata);
}

#else

#define OVERRIDE_KISS_FFTR
void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar * restrict timedata,kiss_fft_cpx * restrict freqdata)
{
    register kiss_fft_cpx fpnk, fpk, f1k, f2k, twk;
    register int k, ncfft;
	register kiss_fft_cpx * restrict tmpbuf, * restrict tw;
	register float tdcr, tdci;

    ncfft = st->substate->nfft;
	tmpbuf= st->tmpbuf;
	tw	  = st->super_twiddles;

	kiss_fft( st->substate , (const kiss_fft_cpx*)timedata, tmpbuf );

	tdcr = tmpbuf[0].r;
    tdci = tmpbuf[0].i;
    
	freqdata[0].r = tdcr + tdci;
    freqdata[ncfft].r = tdcr - tdci;
    freqdata[ncfft].i = freqdata[0].i = 0;

    for ( k=1;k <= ncfft/2 ; ++k ) 
	{
        fpk    = tmpbuf[k]; 
        fpnk.r = tmpbuf[ncfft-k].r;
        fpnk.i = -tmpbuf[ncfft-k].i;

        C_ADD( f1k, fpk , fpnk );
        C_SUB( f2k, fpk , fpnk );
        C_MUL( twk, f2k , tw[k]);

        freqdata[k].r = HALF_OF(f1k.r + twk.r);
        freqdata[k].i = HALF_OF(f1k.i + twk.i);
        freqdata[ncfft-k].r = HALF_OF(f1k.r - twk.r);
        freqdata[ncfft-k].i = HALF_OF(twk.i - f1k.i);
    }
}

#define OVERRIDE_KISS_FFTRI
void kiss_fftri(kiss_fftr_cfg st,const kiss_fft_cpx * restrict freqdata,kiss_fft_scalar * restrict timedata)
{
    register int k, ncfft;
	register kiss_fft_cpx * restrict tmpbuf, * restrict tw;
	
		
    ncfft = st->substate->nfft;
	tmpbuf= st->tmpbuf;
	tw	  = st->super_twiddles;
		
    tmpbuf[0].r = freqdata[0].r + freqdata[ncfft].r;
    tmpbuf[0].i = freqdata[0].r - freqdata[ncfft].r;

    for (k = 1; k <= ncfft / 2; ++k) 
	{
        register kiss_fft_cpx fk, fnkc, fek, fok, tmp;
        fk = freqdata[k];
        fnkc.r = freqdata[ncfft - k].r;
        fnkc.i = -freqdata[ncfft - k].i;

        C_ADD (fek, fk, fnkc);
        C_SUB (tmp, fk, fnkc);
        C_MUL (fok,tmp,tw[k]);
        C_ADD (tmpbuf[k],fek, fok);
        C_SUB (tmp, fek, fok);
		tmpbuf[ncfft - k].r = tmp.r;
        tmpbuf[ncfft - k].i = -tmp.i;
	}
    kiss_fft (st->substate, st->tmpbuf, (kiss_fft_cpx *) timedata);
}

#endif
#endif

