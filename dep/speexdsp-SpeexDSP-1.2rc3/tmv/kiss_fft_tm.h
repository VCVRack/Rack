/* Copyright (C) 2007 Hong Zhiqian */
/**
   @file kiss_fft_tm.h
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

#define OVERRIDE_KFBFLY2
static void kf_bfly2(
		kiss_fft_cpx		*Fout,
        const int			fstride,
        const kiss_fft_cfg	st,
        int					m
        )
{
	register int * restrict Fout2;
    register int * restrict tw1 = (int*)st->twiddles;
    register int i, j;
	register int _inv = !st->inverse;
    
	Fout2 = (int*)Fout + m;
	
	for ( i=0,j=0 ; i<m ; ++i,j+=4,tw1+=fstride )
	{	register int tw_10, ff_10, f2_10;

		ff_10	= ld32x(Fout, i);
		f2_10	= ld32x(Fout2, i);
		tw_10	= ld32(tw1);
		
		if ( _inv ) 
		{	TM_SHR(f2_10, f2_10, 1);
			TM_SHR(ff_10, ff_10, 1);
		}

		TM_MUL(tw_10, tw_10, f2_10);
		TM_SUB(f2_10, ff_10, tw_10);
		TM_ADD(ff_10, ff_10, tw_10);

		st32d(j, Fout2, f2_10);
		st32d(j,  Fout, ff_10);
	}
}

#define OVERRIDE_KFBFLY4
static void kf_bfly4(
        kiss_fft_cpx		*Fout,
        const int			fstride,
        const kiss_fft_cfg	st,
        const int			m
        )
{
    register int * restrict tw1;
	register int * restrict tw2;
	register int * restrict tw3;
	register int * restrict Fout1;
	register int * restrict Fout2;
	register int * restrict Fout3;
	register int i, j;
	register int fstride2, fstride3;
	register int _inv = !st->inverse;

	tw3  = tw2 = tw1 = (int*)st->twiddles;
	fstride2 = fstride << 1;
	fstride3 = fstride * 3;

	Fout1 = (int*)Fout + m;
	Fout2 = (int*)Fout + (m << 1);
	Fout3 = (int*)Fout + (m *  3);


	for ( i=0,j=0 ; i<m ; ++i,j+=4,tw1+=fstride,tw2+=fstride2,tw3+=fstride3 )
	{	register int sc0, sc1, sc2, sc3, sc4, sc5;
		register int ff0;

		sc0   = ld32x(Fout1,i);
		sc3   = ld32(tw1);
		sc1   = ld32x(Fout2, i);
		sc4   = ld32(tw2);			
		sc2   = ld32x(Fout3, i);
		sc5   = ld32(tw3);
		ff0   = ld32x(Fout,i);
		
		if ( _inv )
		{
			TM_ADD(sc0, sc0, 0x00020002);
			TM_ADD(sc1, sc1, 0x00020002);
			TM_ADD(sc2, sc2, 0x00020002);
			TM_ADD(ff0, ff0, 0x00020002);
			TM_SHR(sc0, sc0, 2);
			TM_SHR(sc1, sc1, 2);
			TM_SHR(sc2, sc2, 2);
			TM_SHR(ff0, ff0, 2);
		}

		TM_MUL(sc0, sc0, sc3);
		TM_MUL(sc1, sc1, sc4);
		TM_MUL(sc2, sc2, sc5);
		TM_SUB(sc5, ff0, sc1);
		TM_ADD(ff0, ff0, sc1);
		TM_ADD(sc3, sc0, sc2);
		TM_SUB(sc4, sc0, sc2);
		TM_SUB(sc1, ff0, sc3);
		TM_ADD(ff0, ff0, sc3);
		
		st32d(j, Fout2, sc1);
		st32d(j, Fout,  ff0);

		sc5 = funshift2(sc5, sc5);
		
		if ( _inv )
		{	TM_ADD(ff0, sc5, sc4);
			TM_SUB(sc1, sc5, sc4);
		} else
		{	TM_ADD(sc1, sc5, sc4);
			TM_SUB(ff0, sc5, sc4);
		}

		sc0 = funshift2(sc1, ff0);
		sc2 = funshift2(ff0, sc1);

		st32d(j, Fout1, sc0);
		st32d(j, Fout3, sc2);
	}
}


#define OVERRIDE_KFBFLY3
static void kf_bfly3(
         kiss_fft_cpx	*Fout,
         const int		fstride,
         const			kiss_fft_cfg st,
         int			m
         )
{
    register int * restrict tw1;
	register int * restrict tw2;
	register int * restrict Fout1;
	register int * restrict Fout2;
    register int epi;
	register int i, j;
	register int fstride2;
	register int _inv = !st->inverse;

    tw1  = tw2 = (int*)st->twiddles;
	Fout1 = (int*)Fout + m;
	Fout2 = (int*)Fout + (m << 1);
	epi = tw1[fstride*m];
    epi = pack16lsb(epi,epi);
	fstride2 = fstride << 1;

	 for ( i=0,j=0 ; i<m ; ++i,j+=4,tw1+=fstride,tw2+=fstride2 )
	 {	register int sc0, sc1, sc2, sc3, sc4, sc5;
		register int ff0;

		sc1 = ld32x(Fout1,i);
		sc2 = ld32x(Fout2,i);
		sc3 = ld32(tw1);
		sc4 = ld32(tw2);
		ff0 = ld32x(Fout,i);
		
		if ( _inv )
		{
			TM_DIV(sc1, sc1, 3);
			TM_DIV(sc2, sc2, 3);
			TM_DIV(ff0, ff0, 3);
		}

		TM_MUL(sc1, sc1,  sc3);
		TM_MUL(sc2, sc2,  sc4);
		TM_ADD(sc3, sc1,  sc2);
		TM_SUB(sc0, sc1,  sc2);
		TM_SHR(sc4, sc3,    1);
		TM_SUB(sc1, ff0,  sc4);

		sc0 = dspidualmul(sc0, epi);
		sc0 = funshift2(sc0, sc0);

		TM_ADD(ff0, ff0, sc3);
		TM_ADD(sc4, sc1, sc0);
		TM_SUB(sc5, sc1, sc0);

		sc1 = funshift2(sc4, sc5);
		sc2 = funshift2(sc5, sc4);
		sc2 = funshift2(sc2, sc2);

		st32d(j, Fout1, sc1);
		st32d(j, Fout,  ff0);
		st32d(j, Fout2, sc2);
	 }
}


#define OVERRIDE_KFBFLY5
static void kf_bfly5(
        kiss_fft_cpx		*Fout,
        const int			fstride,
        const kiss_fft_cfg	st,
        int m
        )
{   
    register int * restrict tw1;
	register int * restrict tw2;
	register int * restrict tw3;
	register int * restrict tw4;
	register int * restrict Fout1;
	register int * restrict Fout2;
	register int * restrict Fout3;
	register int * restrict Fout4;
	register int fstride2, fstride3, fstride4;
	register int i, j;
	register int yab_msb, yab_lsb, yba_msb, yba_lsb;
	register int _inv = !st->inverse;


    Fout1=(int*)Fout+m;
    Fout2=(int*)Fout+(m<<1);
    Fout3=(int*)Fout+(3 *m);
    Fout4=(int*)Fout+(m<<2);

    tw1 = tw2 = tw3 = tw4 = (int*)st->twiddles;

	i = tw1[fstride*m];
    yab_lsb = tw1[fstride*(m<<1)];
	yab_msb = pack16msb(i, yab_lsb);
	yab_lsb = pack16lsb(i, yab_lsb);
	yba_msb = funshift2(-sex16(yab_msb), yab_msb);
	yba_lsb = funshift2(yab_lsb, yab_lsb);

	fstride2 = fstride << 1;
	fstride3 = fstride *  3;
	fstride4 = fstride << 2;

	for ( i=0,j=0 ; i<m ; ++i,j+=4,tw1+=fstride,tw2+=fstride2,tw3+=fstride3,tw4+=fstride4 )
	{	register int sc0, sc1, sc2, sc3, sc4, sc5, sc6;
		register int sc7, sc8, sc9, sc10, sc11, sc12;
		register int ff0, sc78_msb, sc78_lsb, sc90_msb, sc90_lsb;

		sc0 = ld32x(Fout,i);
		sc1 = ld32x(Fout1,i);
		sc2 = ld32x(Fout2,i);
		sc3 = ld32x(Fout3,i);
		sc4 = ld32x(Fout4,i);
		sc5 = ld32(tw1);
		sc6 = ld32(tw2);
		sc7 = ld32(tw3);
		sc8 = ld32(tw4);

		if ( _inv )
		{
			TM_DIV(sc0, sc0, 5);
			TM_DIV(sc1, sc1, 5);
			TM_DIV(sc2, sc2, 5);
			TM_DIV(sc3, sc3, 5);
			TM_DIV(sc4, sc4, 5);
		}

		ff0 = sc0;

		TM_MUL(sc1, sc1, sc5);
		TM_MUL(sc2, sc2, sc6);
		TM_MUL(sc3, sc3, sc7);
		TM_MUL(sc4, sc4, sc8);
		TM_ADD(sc7, sc1, sc4);
		TM_SUB(sc10,sc1, sc4);
		TM_ADD(sc8, sc2, sc3);
		TM_SUB(sc9, sc2, sc3);

		TM_ADD(ff0, ff0, sc7);
		TM_ADD(ff0, ff0, sc8);
		st32d(j, Fout,  ff0);

		sc78_msb = pack16msb(sc7,sc8);
		sc78_lsb = pack16lsb(sc7,sc8);
		sc90_msb = pack16msb(sc10,sc9);
		sc90_lsb = pack16lsb(sc10,sc9);
		
		sc5 = pack16lsb( sround(ifir16(sc78_msb,yab_lsb)), sround(ifir16(sc78_lsb,yab_lsb)));
		sc6 = pack16lsb(-sround(ifir16(sc90_lsb,yab_msb)), sround(ifir16(sc90_msb,yab_msb)));

		TM_ADD(sc5, sc5, sc0);
		TM_SUB(sc1, sc5, sc6);
		TM_ADD(sc4, sc5, sc6);
		st32d(j, Fout1, sc1);
		st32d(j, Fout4, sc4);

		sc11 = pack16lsb( sround(ifir16(sc78_msb,yba_lsb)), sround(ifir16(sc78_lsb,yba_lsb)));
		sc12 = pack16lsb(-sround(ifir16(sc90_lsb,yba_msb)), sround(ifir16(sc90_msb,yba_msb)));

		TM_ADD(sc11, sc11, sc0);
		TM_ADD(sc2, sc11, sc12);
		TM_SUB(sc3, sc11, sc12);
		st32d(j, Fout2, sc2);
		st32d(j, Fout3, sc3);

	}
}


#define OVERRIDE_KF_BFLY_GENERIC
static void kf_bfly_generic(
        kiss_fft_cpx * restrict Fout,
        const size_t fstride,
        const kiss_fft_cfg st,
        int m,
        int p
        )
{
	register int _inv = !st->inverse;
	register int i, j, k, l;
    register int * restrict twiddles = (int*)st->twiddles;
    register int Norig = st->nfft;

    CHECKBUF(scratchbuf,nscratchbuf,p);

    for ( i=0; i<m; ++i ) 
	{	register int sc10;

        for ( j=0,k=i ; j<p ; ++j,k+=m ) 
		{	register int f10;

			f10 = ld32x(Fout,k);

			if ( _inv ) 
			{	TM_DIV(f10, f10, p);
			}

			st32d(j<<2, scratchbuf, f10);
        }

        for ( j=0,k=i,sc10=ld32(scratchbuf) ; j<p ; ++j,k+=m ) 
		{
            register int twidx = 0;
			register int f10;

            for ( l=1,f10 = sc10 ; l<p ; ++l ) 
			{	register int tw, sc;

                twidx += fstride * k;
				if ( twidx>=Norig ) 
				{	twidx -= Norig;
				}
				
				sc = ld32x(scratchbuf,l);
				tw = ld32x(twiddles,twidx);
                
				TM_MUL(sc, sc, tw);
				TM_ADD(f10, f10, sc);
			}
			st32d(k<<2, Fout, f10); 
		}
	}
}

#else

#define OVERRIDE_KFBFLY2
static void kf_bfly2(
        kiss_fft_cpx * Fout,
        const size_t fstride,
        const kiss_fft_cfg st,
        int m
        )
{
    register kiss_fft_cpx * restrict Fout2;
    register kiss_fft_cpx * restrict tw1 = st->twiddles;

    Fout2 = Fout + m;

    do
	{
		register kiss_fft_cpx _fout2, _fout, t;
		
		_fout2 = *Fout2;
		_fout  = *Fout;
		
        C_MUL	(	  t,  _fout2,   *tw1);
        C_SUB	(_fout2,   _fout,	   t);
        C_ADD	(_fout,    _fout,	   t);
		
		*Fout2 = _fout2;
		*Fout  = _fout;
		
		tw1	+= fstride;
        ++Fout2;
        ++Fout;

    } while ( --m );
}

#define OVERRIDE_KFBFLY4
static void kf_bfly4(
        kiss_fft_cpx * Fout,
        const int fstride,
        const kiss_fft_cfg st,
        int m
        )
{
    register kiss_fft_cpx * restrict tw1,* restrict tw2,* restrict tw3;
	register kiss_fft_cpx * restrict Fout1, * restrict Fout2, * restrict Fout3; 
	register int _inv = !st->inverse;
	
    tw3 = tw2 = tw1 = st->twiddles;
	
	Fout1 = Fout + m;
	Fout2 = Fout + (m << 1);
	Fout3 = Fout + (m * 3);

	do {
	   
		register kiss_fft_cpx _fout;
		register kiss_fft_cpx sc0, sc1, sc2, sc3, sc4, sc5;
		
		_fout = *Fout;
		
		C_MUL(   sc0,*Fout1, *tw1);
		C_MUL(   sc1,*Fout2, *tw2);
		C_MUL(   sc2,*Fout3, *tw3);
		C_SUB(   sc5, _fout,  sc1);
		C_ADD( _fout, _fout,  sc1);
		C_ADD(   sc3,   sc0,  sc2);
		C_SUB(   sc4,   sc0,  sc2);
		C_SUB(*Fout2, _fout,  sc3);
		C_ADD( *Fout, _fout,  sc3);

		tw1 += fstride;		
		tw2 += (fstride << 1);
		tw3 += (fstride *  3);
		
		if ( _inv )
		{
			Fout1->r = sc5.r + sc4.i;
			Fout1->i = sc5.i - sc4.r;
			Fout3->r = sc5.r - sc4.i;
			Fout3->i = sc5.i + sc4.r;
		}
		else
		{	Fout1->r = sc5.r - sc4.i;
			Fout1->i = sc5.i + sc4.r;
			Fout3->r = sc5.r + sc4.i;
			Fout3->i = sc5.i - sc4.r;
		}
		  
		  
        ++Fout; ++Fout1; ++Fout2; ++Fout3;
		
    } while(--m);
}

#define OVERRIDE_KFBFLY3
static void kf_bfly3(
         kiss_fft_cpx * Fout,
         const int fstride,
         const kiss_fft_cfg st,
         int m
         )
{
	register kiss_fft_cpx * restrict Fout1, * restrict Fout2;
	register kiss_fft_cpx * restrict tw1,* restrict tw2;
	register float epi;
	
    tw1 = tw2 = st->twiddles;
    epi = st->twiddles[fstride*m].i;
	Fout1 = Fout + m;
	Fout2 = Fout + (m << 1);

    do {
        
		register kiss_fft_cpx _fout;
		register kiss_fft_cpx sc0, sc1, sc2, sc3;
		
		_fout = *Fout;
		
        C_MUL(   sc1, *Fout1,  *tw1);
        C_MUL(   sc2, *Fout2,  *tw2);
        C_ADD(	 sc3,    sc1,   sc2);
        C_SUB(   sc0,    sc1,   sc2);
        tw1 += fstride;
        tw2 += (fstride << 1);
		
        sc1.r = _fout.r - HALF_OF(sc3.r);
        sc1.i = _fout.i - HALF_OF(sc3.i);
		
        C_MULBYSCALAR(sc0,  epi);
        C_ADD(*Fout, _fout, sc3);
		
        Fout2->r = sc1.r + sc0.i;
        Fout2->i = sc1.i - sc0.r;
		
        Fout1->r = sc1.i - sc0.i;
        Fout1->i = sc1.r + sc0.r;
		
        ++Fout; ++Fout1; ++Fout2;
		
	} while(--m);
}

#define OVERRIDE_KFBFLY5
static void kf_bfly5(
        kiss_fft_cpx * Fout,
        const size_t fstride,
        const kiss_fft_cfg st,
        int m
        )
{
    register kiss_fft_cpx * restrict Fout1,* restrict Fout2,* restrict Fout3,* restrict Fout4;
	register int u;
    register kiss_fft_cpx *tw;
    register float yar, yai, ybr, ybi;

    Fout1=Fout+m;
    Fout2=Fout+(m<<1);
    Fout3=Fout+(m*3);
    Fout4=Fout+(m<<2);

    tw = st->twiddles;
    yar = tw[fstride*m].r;
	yai = tw[fstride*m].i;
    ybr = tw[fstride*2*m].r;
	ybi = tw[fstride*2*m].i;
   
	for ( u=0; u<m; ++u )
	{
		register kiss_fft_cpx sc0, sc1, sc2, sc3, sc4, sc5, sc6, sc7, sc8, sc9, sc10, sc11, sc12;
		
		sc0 = *Fout;
		
        C_MUL(   sc1,*Fout1,   tw[u*fstride]);
        C_MUL(   sc2,*Fout2, tw[2*u*fstride]);
        C_MUL(   sc3,*Fout3, tw[3*u*fstride]);
        C_MUL(   sc4,*Fout4, tw[4*u*fstride]);
		
        C_ADD(   sc7,   sc1,   sc4);
        C_SUB(  sc10,   sc1,   sc4);
        C_ADD(   sc8,   sc2,   sc3);
        C_SUB(   sc9,   sc2,   sc3);
		
        Fout->r = sc0.r + sc7.r + sc8.r;
        Fout->i = sc0.i + sc7.i + sc8.i;
		
        sc5.r = sc0.r + S_MUL(sc7.r,yar) + S_MUL(sc8.r,ybr);
        sc5.i = sc0.i + S_MUL(sc7.i,yar) + S_MUL(sc8.i,ybr);
		
        sc6.r =  S_MUL(sc10.i,yai) + S_MUL(sc9.i,ybi);
        sc6.i = -S_MUL(sc10.r,yai) - S_MUL(sc9.r,ybi);
		
        C_SUB(*Fout1,sc5,sc6);
        C_ADD(*Fout4,sc5,sc6);
		
        sc11.r = sc0.r + S_MUL(sc7.r,ybr) + S_MUL(sc8.r,yar);
        sc11.i = sc0.i + S_MUL(sc7.i,ybr) + S_MUL(sc8.i,yar);
        sc12.r = - S_MUL(sc10.i,ybi) + S_MUL(sc9.i,yai);
        sc12.i = S_MUL(sc10.r,ybi) - S_MUL(sc9.r,yai);
        C_ADD(*Fout2,sc11,sc12);
        C_SUB(*Fout3,sc11,sc12);
		
        ++Fout1; ++Fout2; ++Fout3; ++Fout4;
	}
}


#endif

#endif
