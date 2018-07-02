/*******************************************************************************

"A Collection of Useful C++ Classes for Digital Signal Processing"
By Vincent Falco

Official project location:
http://code.google.com/p/dspfilterscpp/

See DspFilter.cpp for notes and bibliography.

--------------------------------------------------------------------------------

License: MIT License (http://www.opensource.org/licenses/mit-license.php)
Copyright (c) 2009 by Vincent Falco

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

********************************************************************************

Please direct all comments to either the music-dsp mailing list or
the DSP and Plug-in Development forum:

	http://music.columbia.edu/cmc/music-dsp/
	
	http://www.kvraudio.com/forum/viewforum.php?f=33
	http://www.kvraudio.com/forum/

Support is provided for performing N-order Dsp floating point filter
operations on M-channel data with a caller specified floating point type.
The implementation breaks a high order IIR filter down into a series of
cascaded second order stages. Tests conclude that numerical stability is
maintained even at higher orders. For example the Butterworth low pass
filter is stable at up to 53 poles.

Processing functions are provided to use either Direct Form I or Direct
Form II of the filter transfer function. Direct Form II is slightly faster
but can cause discontinuities in the output if filter parameters are changed
during processing. Direct Form I is slightly slower, but maintains fidelity
even when parameters are changed during processing.

To support fast parameter changes, filters provide two functions for
adjusting parameters. A high accuracy Setup() function, and a faster
form called SetupFast() that uses approximations for trigonometric
functions. The approximations work quite well and should be suitable for
most applications.

Channels are stored in an interleaved format with M samples per frame
arranged contiguously. A single class instance can process all M channels
simultaneously in an efficient manner. A 'skip' parameter causes the
processing function to advance by skip additional samples in the destination
buffer in between every frame. Through manipulation of the skip paramter it
is possible to exclude channels from processing (for example, only processing
the left half of stereo interleaved data). For multichannel data which is
not interleaved, it will be necessary to instantiate multiple instance of
the filter and set skip=0.

There are a few other utility classes and functions included that may prove useful.

Classes:

Order for PoleFilterSpace derived classes is specified in the number of poles,
except for band pass and band stop filters, for which the number of pole pairs
is specified.

For some filters there are two versions of Setup(), the one called
SetupFast() uses approximations to trigonometric functions for speed.
This is an option if you are doing frequent parameter changes to the filter.

There is an example function at the bottom that shows how to use the classes.


For a tutorial on digital filter design these are useful resources:

http://crca.ucsd.edu/~msp/techniques/v0.08/book-html/node1.html

	- Need fast version of pow( 10, x )

	- Able to specify band-width in octaves

	- Specify center frequency by midi note number
		F = 440 * 2^((d-69)/12) where D is your midi note.
		know that middle C is considered 60.
		http://en.wikipedia.org/wiki/MIDI_Tuning_Standard

--------------------------------------------------------------------------------

PoleFilterSpace ideas are based on a java applet (http://www.falstad.com/dfilter/)
developed by Paul Falstad.

All of this code was written by the author Vincent Falco except where marked.

*******************************************************************************/

#ifdef _UNITTEST

#include "DebugUtil.h"
#define bassert assert
#endif

#include "DspFilter.h"
#include <limits>
#include <algorithm>



//------------------------------------------------------------------------------

#ifdef _MSC_VER
	#define DSP_SSE3_AVAIL
#else
	// other build environments
#endif

//------------------------------------------------------------------------------

#ifdef _MSC_VER
	#ifdef DSP_SSE3_AVAIL
		#include <intrin.h>
		#include <pmmintrin.h>
	#endif
#else
	// other build environments
#endif


//static void dumpRoots(const char * title, Dsp::Roots &roots);

//******************************************************************************

namespace Dsp
{
	
	//--------------------------------------------------------------------------

	// Brent's method is a numerical analysis technique for finding the
	// minimum (or maximum) of a function. For filters, we use this to
	// normalize the gain of a filter by finding the location where the
	// filter has the highest magnitude response, and scaling the output
	// by the reciprocal of the magnitude to reach unity gain. For most
	// filters this is unnecessary, because we can just sample the response
	// at a well known angular frequency. For example, w=0 for a low pass,
	// w=pi for a high pass, the geometric center of the passband on a band-pass
	// or 0 or pi for a band-stop. For filters with ripple we use a simple
	// adjustment based on the ripple amount and whether or not the filter
	// is of odd order. However, Brent's method is included in case anyone
	// needs it.

	// Implementation of Brent's Method provided by
	// John D. Cook (http://www.johndcook.com/)
	// The return value of Minimize is the minimum of the function f.
	// The location where f takes its minimum is returned in the variable minLoc.
	// Notation and implementation based on Chapter 5 of Richard Brent's book
	// "Algorithms for Minimization Without Derivatives".
	template<class TFunction>
	CalcT BrentMinimize
	(
		TFunction& f,	// [in] objective function to minimize
		CalcT leftEnd,	// [in] smaller value of bracketing interval
		CalcT rightEnd,	// [in] larger value of bracketing interval
		CalcT epsilon,	// [in] stopping tolerance
		CalcT& minLoc	// [out] location of minimum
	)
	{
		CalcT d, e, m, p, q, r, tol, t2, u, v, w, fu, fv, fw, fx;
		static const CalcT c = 0.5*(3.0 - ::std::sqrt(5.0));
		static const CalcT SQRT_DBL_EPSILON = ::std::sqrt(DBL_EPSILON);

		CalcT& a = leftEnd; CalcT& b = rightEnd; CalcT& x = minLoc;

		v = w = x = a + c*(b - a); d = e = 0.0;
		fv = fw = fx = f(x);
		int counter = 0;
	loop:
		counter++;
		m = 0.5*(a + b);
		tol = SQRT_DBL_EPSILON*::fabs(x) + epsilon; t2 = 2.0*tol;
		// Check stopping criteria
		if (::fabs(x - m) > t2 - 0.5*(b - a))
		{
			p = q = r = 0.0;
			if (::fabs(e) > tol)
			{
				// fit parabola
				r = (x - w)*(fx - fv);
				q = (x - v)*(fx - fw);
				p = (x - v)*q - (x - w)*r;
				q = 2.0*(q - r);
				(q > 0.0) ? p = -p : q = -q;
				r = e; e = d;
			}
			if (::fabs(p) < ::fabs(0.5*q*r) && p < q*(a - x) && p < q*(b - x))
			{
				// A parabolic interpolation step
				d = p/q;
				u = x + d;
				// f must not be evaluated too close to a or b
				if (u - a < t2 || b - u < t2)
					d = (x < m) ? tol : -tol;
			}
			else
			{
				// A golden section step
				e = (x < m) ? b : a;
				e -= x;
				d = c*e;
			}
			// f must not be evaluated too close to x
			if (::fabs(d) >= tol)
				u = x + d;
			else if (d > 0.0)
				u = x + tol;
			else
				u = x - tol;
			fu = f(u);
			// Update a, b, v, w, and x
			if (fu <= fx)
			{
				(u < x) ? b = x : a = x;
				v = w; fv = fw; 
				w = x; fw = fx; 
				x = u; fx = fu;
			}
			else
			{
				(u < x) ? a = u : b = u;
				if (fu <= fw || w == x)
				{
					v = w; fv = fw; 
					w = u; fw = fu;
				}
				else if (fu <= fv || v == x || v == w)
				{
					v = u; fv = fu;
				}
			}
			goto loop;  // Yes, the dreaded goto statement. But the code
						// here is faithful to Brent's orginal pseudocode.
		}
		return  fx;
	}

	//--------------------------------------------------------------------------
	//
	//	Fast Trigonometric Functions
	//
	//--------------------------------------------------------------------------

	// Three approximations for both sine and cosine at a given angle.
	// The faster the routine, the larger the error.
	// From http://lab.polygonal.de/2007/07/18/fast-and-accurate-sinecosine-approximation/

	// Tuned for maximum pole stability. r must be in the range 0..kPi
	// This one breaks down considerably at the higher angles. It is
	// included only for educational purposes.
	inline void fastestsincos( CalcT r, CalcT *sn, CalcT *cs )
	{
		const CalcT c=0.70710678118654752440; // std::sqrt(2)/2
		CalcT v=(2-4*c)*r*r+c;
		if(r<kPi_2)
		{
			*sn=v+r; *cs=v-r;
		}
		else
		{
			*sn=r+v; *cs=r-v;
		}
	}

	// Lower precision than ::fastsincos() but still decent
	inline void fastersincos( CalcT x, CalcT *sn, CalcT *cs )
	{
		//always wrap input angle to -PI..PI
		if		(x < -kPi) x += 2*kPi;
		else if (x >  kPi) x -= 2*kPi;
		//compute sine
		if (x < 0)	*sn = 1.27323954 * x + 0.405284735 * x * x;
		else		*sn = 1.27323954 * x - 0.405284735 * x * x;
		//compute cosine: sin(x + PI/2) = cos(x)
		x += kPi_2;
		if (x > kPi ) x -= 2*kPi;
		if (x < 0)	*cs = 1.27323954 * x + 0.405284735 * x * x;
		else		*cs = 1.27323954 * x - 0.405284735 * x * x;
	}

	// Slower than ::fastersincos() but still faster than
	// sin(), and with the best accuracy of these routines.
	inline void fastsincos( CalcT x, CalcT *sn, CalcT *cs )
	{
		CalcT s, c;
		//always wrap input angle to -PI..PI
			 if (x < -kPi) x += 2*kPi;
		else if (x >  kPi) x -= 2*kPi;
		//compute sine
		if (x < 0)
		{
			s = 1.27323954 * x + .405284735 * x * x;
			if (s < 0)	s = .225 * (s * -s - s) + s;
			else		s = .225 * (s *  s - s) + s;
		}
		else
		{
			s = 1.27323954 * x - 0.405284735 * x * x;
			if (s < 0)	s = .225 * (s * -s - s) + s;
			else		s = .225 * (s *  s - s) + s;
		}
		*sn=s;
		//compute cosine: sin(x + PI/2) = cos(x)
		x += kPi_2;
		if (x > kPi ) x -= 2*kPi;
		if (x < 0)
		{
			c = 1.27323954 * x + 0.405284735 * x * x;
			if (c < 0)	c = .225 * (c * -c - c) + c;
			else		c = .225 * (c *  c - c) + c;
		}
		else
		{
			c = 1.27323954 * x - 0.405284735 * x * x;
			if (c < 0)	c = .225 * (c * -c - c) + c;
			else		c = .225 * (c *  c - c) + c;
		}
		*cs=c;
	}

	// Faster approximations to std::sqrt()
	//	From http://ilab.usc.edu/wiki/index.php/Fast_Square_Root
	//	The faster the routine, the more error in the approximation.

	// Log Base 2 Approximation
	// 5 times faster than std::sqrt()

	inline float fastsqrt1( float x )  
	{
		union { Int32 i; float x; } u;
		u.x = x;
		u.i = (Int32(1)<<29) + (u.i >> 1) - (Int32(1)<<22); 
		return u.x;
	}

	inline double fastsqrt1( double x )  
	{
		union { Int64 i; double x; } u;
		u.x = x;
		u.i = (Int64(1)<<61) + (u.i >> 1) - (Int64(1)<<51); 
		return u.x;
	}

	// Log Base 2 Approximation with one extra Babylonian Step
	// 2 times faster than std::sqrt()

	inline float fastsqrt2( float x )  
	{
		float v=fastsqrt1( x );
		v = 0.5f * (v + x/v); // One Babylonian step
		return v;
	}

	inline double fastsqrt2(const double x)  
	{
		double v=fastsqrt1( x );
		v = 0.5f * (v + x/v); // One Babylonian step
		return v;
	}

	// Log Base 2 Approximation with two extra Babylonian Steps
	// 50% faster than std::sqrt()

	inline float fastsqrt3( float x )  
	{
		float v=fastsqrt1( x );
		v =		   v + x/v;
		v = 0.25f* v + x/v; // Two Babylonian steps
		return v;
	}

	inline double fastsqrt3(const double x)  
	{
		double v=fastsqrt1( x );
		v =		   v + x/v;
		v = 0.25 * v + x/v; // Two Babylonian steps
		return v;
	}
};

//******************************************************************************

using namespace Dsp;

//******************************************************************************

// Lightweight class for retrieving CPU information.
struct Cpu
{
public:
	struct Info
	{
		bool bMMX;
		bool bSSE;
		bool bSSE2;
		bool bSSE3;
		bool bSSSE3;	// supplemental SSE3
		bool bSSE41;
		bool bSSE42;
	};

	Cpu();

	const Info &GetInfo( void );

protected:
	Info m_info;
};

//------------------------------------------------------------------------------

Cpu::Cpu()
{
	m_info.bMMX=false;
	m_info.bSSE=false;
	m_info.bSSE2=false;
	m_info.bSSE3=false;
	m_info.bSSSE3=false;
	m_info.bSSE41=false;
	m_info.bSSE42=false;

#ifdef _MSC_VER
	int inf[4];
	__cpuid( inf, 0 );
	int nIds=inf[0];

	if( nIds>=1 )
	{
		__cpuid( inf, 1 );

		m_info.bMMX		=(inf[3]&(1<<23))!=0;
		m_info.bSSE		=(inf[3]&(1<<24))!=0;
		m_info.bSSE2	=(inf[3]&(1<<25))!=0;
		m_info.bSSE3	=(inf[2]&(1<<0))!=0;
		m_info.bSSSE3	=(inf[2]&(1<<9))!=0;
		m_info.bSSE41	=(inf[2]&(1<<19))!=0;
		m_info.bSSE42	=(inf[2]&(1<<20))!=0;
	}
#endif
}

//------------------------------------------------------------------------------

const Cpu::Info &Cpu::GetInfo( void )
{
	return m_info;
}

//------------------------------------------------------------------------------

static Cpu gCpu;

//******************************************************************************
//
// Cascade
//
//******************************************************************************

Cascade::Cascade()
{
	m_stageCount=0;
}

//------------------------------------------------------------------------------

int Cascade::GetStageCount( void )
{
	return m_stageCount;
}

//------------------------------------------------------------------------------

void Cascade::SetStageCount( int n )
{
	assert( n>=1 && n<=m_stageMax );
	m_stageCount=n;
}

//------------------------------------------------------------------------------

Cascade::Stage *Cascade::Stages( void )
{
	return m_stage;
}

//------------------------------------------------------------------------------

void Cascade::SetStage1( CalcT a1, CalcT a2, CalcT b0, CalcT b1, CalcT b2 )
{
	m_stage->a[1]=a1;
	m_stage->a[2]=a2;
	m_stage->b[0]=b0;
	m_stage->b[1]=b1;
	m_stage->b[2]=b2;
}

//------------------------------------------------------------------------------

void Cascade::Reset( void )
{
	for( int i=0;i<m_stageCount;i++ )
		m_stage[i].Reset();
}

//------------------------------------------------------------------------------

void Cascade::Scale( CalcT factor )
{
	m_stage[0].b[0]*=factor;
	m_stage[0].b[1]*=factor;
	m_stage[0].b[2]*=factor;
#ifdef _LOG
	{
			char buf[512];
			sprintf(buf, "Scale set b0 to %f b1 to %f b2 %f\n", 
				m_stage[0].b[0],
				m_stage[0].b[1],
				m_stage[0].b[2]
			);
			DebugUtil::trace(buf);
		}
#endif
}

//------------------------------------------------------------------------------

Complex	Cascade::Response_radian( CalcT w ) const
{
	Complex ch( 1 );
	Complex cbot( 1 );
	Complex czn1=std::polar( 1., -w );
	Complex czn2=std::polar( 1., -2*w );
	for( int i=0;i<m_stageCount;i++ )
	{
		const Stage &s=m_stage[i];
		Complex ct( s.b[0] );
		Complex cb( 1 );
		ct=addmul( ct,  s.b[1], czn1 );
		cb=addmul( cb, -s.a[1], czn1 );
		ct=addmul( ct,  s.b[2], czn2 );
		cb=addmul( cb, -s.a[2], czn2 );
		ch  *=ct;
		cbot*=cb;
	}
	return ch/cbot;
}

//******************************************************************************

Cascade::ResponseFunctor::ResponseFunctor( Cascade *cascade )
{
	m_cascade=cascade;
}

//------------------------------------------------------------------------------

CalcT Cascade::ResponseFunctor::operator()( CalcT w )
{
	assert (false );	// does ths caller know that this is a "radian" function?
	// Reflect around x axis so Brent's
	// method finds a maximum instead of a minimum.
	return -std::abs( m_cascade->Response_radian( w ) );
}

//******************************************************************************
//
// CascadeFilter
//
//******************************************************************************

void CascadeFilter::Clear( void )
{
	int n=m_nchan*GetStageCount();
	memset( m_histp, 0, n*sizeof(m_histp[0]) );
}

//------------------------------------------------------------------------------

// ALL SSE OPTIMIZATIONS ASSUME CalcT as double

#ifdef DSP_SSE3_AVAIL

template<typename Ty>
static void ProcessISSEStageStereo( size_t frames, Ty *dest, Cascade::Stage *s,
								    CascadeFilter::Hist *h, int skip )
{
#if 1
	CalcT b0=s->b[0];
	__m128d m0=_mm_loadu_pd( &s->a[1] );	// a1 , a2
	__m128d m1=_mm_loadu_pd( &s->b[1] );	// b1 , b2
	__m128d m2=_mm_loadu_pd( &h[0].v[0] );	// h->v[0] , h->v[1]
	__m128d m3=_mm_loadu_pd( &h[0].v[2] );	// h->v[2] , h->v[3]
	__m128d m4=_mm_loadu_pd( &h[1].v[0] );	// h->v[0] , h->v[1]
	__m128d m5=_mm_loadu_pd( &h[1].v[2] );	// h->v[2] , h->v[3]

	while( frames-- )
	{
		CalcT in, b0in, out;

		__m128d m6;
		__m128d m7;

		in=CalcT(*dest);
		b0in=b0*in;

		m6=_mm_mul_pd ( m1, m2 );	// b1*h->v[0] , b2*h->v[1]
		m7=_mm_mul_pd ( m0, m3 );	// a1*h->v[2] , a2*h->v[3]
		m6=_mm_add_pd ( m6, m7 );	// b1*h->v[0] + a1*h->v[2], b2*h->v[1] + a2*h->v[3]
		m7=_mm_load_sd( &b0in );	// b0*in , 0
		m6=_mm_add_sd ( m6, m7 );	// b1*h->v[0] + a1*h->v[2] + in*b0 , b2*h->v[1] + a2*h->v[3] + 0
		m6=_mm_hadd_pd( m6, m7 );	// b1*h->v[0] + a1*h->v[2] + in*b0 + b2*h->v[1] + a2*h->v[3], in*b0
		   _mm_store_sd( &out, m6 );
		m6=_mm_loadh_pd( m6, &in );	// out , in
		m2=_mm_shuffle_pd( m6, m2, _MM_SHUFFLE2( 0, 1 ) ); // h->v[0]=in , h->v[1]=h->v[0]
		m3=_mm_shuffle_pd( m6, m3, _MM_SHUFFLE2( 0, 0 ) ); // h->v[2]=out, h->v[3]=h->v[2]

		*dest++=Ty(out);

		in=CalcT(*dest);
		b0in=b0*in;

		m6=_mm_mul_pd ( m1, m4 );	// b1*h->v[0] , b2*h->v[1]
		m7=_mm_mul_pd ( m0, m5 );	// a1*h->v[2] , a2*h->v[3]
		m6=_mm_add_pd ( m6, m7 );	// b1*h->v[0] + a1*h->v[2], b2*h->v[1] + a2*h->v[3]
		m7=_mm_load_sd( &b0in );	// b0*in , 0
		m6=_mm_add_sd ( m6, m7 );	// b1*h->v[0] + a1*h->v[2] + in*b0 , b2*h->v[1] + a2*h->v[3] + 0
		m6=_mm_hadd_pd( m6, m7 );	// b1*h->v[0] + a1*h->v[2] + in*b0 + b2*h->v[1] + a2*h->v[3], in*b0
		   _mm_store_sd( &out, m6 );
		m6=_mm_loadh_pd( m6, &in );	// out , in
		m4=_mm_shuffle_pd( m6, m4, _MM_SHUFFLE2( 0, 1 ) ); // h->v[0]=in , h->v[1]=h->v[0]
		m5=_mm_shuffle_pd( m6, m5, _MM_SHUFFLE2( 0, 0 ) ); // h->v[2]=out, h->v[3]=h->v[2]

		*dest++=Ty(out);

		dest+=skip;
	}

	// move history from registers back to state
	_mm_storeu_pd( &h[0].v[0], m2 );
	_mm_storeu_pd( &h[0].v[2], m3 );
	_mm_storeu_pd( &h[1].v[0], m4 );
	_mm_storeu_pd( &h[1].v[2], m5 );

#else
	// Template-specialized version from which the assembly was modeled
	CalcT a1=s->a[1];
	CalcT a2=s->a[2];
	CalcT b0=s->b[0];
	CalcT b1=s->b[1];
	CalcT b2=s->b[2];
	while( frames-- )
	{
		CalcT in, out;

		in=CalcT(*dest);
		out=b0*in+b1*h[0].v[0]+b2*h[0].v[1] +a1*h[0].v[2]+a2*h[0].v[3];
		h[0].v[1]=h[0].v[0]; h[0].v[0]=in;
		h[0].v[3]=h[0].v[2]; h[0].v[2]=out;
		in=out;
		*dest++=Ty(in);

		in=CalcT(*dest);
		out=b0*in+b1*h[1].v[0]+b2*h[1].v[1] +a1*h[1].v[2]+a2*h[1].v[3];
		h[1].v[1]=h[1].v[0]; h[1].v[0]=in;
		h[1].v[3]=h[1].v[2]; h[1].v[2]=out;
		in=out;
		*dest++=Ty(in);

		dest+=skip;
	}
#endif
}

#endif

//------------------------------------------------------------------------------

template<typename Ty>
void CascadeFilter::ProcessI( size_t frames, Ty *dest, int skip )
{
#ifdef DSP_SSE3_AVAIL
	// Note there could be a loss of accuracy here. Unlike the original version
	// of Process...() we are applying each stage to all of the input data.
	// Since the underlying type Ty could be float, the results from this function
	// may be different than the unoptimized version. However, it is much faster.
	if( m_nchan==2 && gCpu.GetInfo().bSSE3 )
	{
		int nstage=GetStageCount();
		Cascade::Stage *s=Stages();
		Hist *h=m_histp;
		for( int i=nstage;i;i--,h+=2,s++ )
		{
			ProcessISSEStageStereo( frames, dest, s, h, skip );
		}
	}
	else
#endif
	{
		int nstage=GetStageCount();
		Cascade::Stage *stagep=Stages();
		while( frames-- )
		{
			Hist *h=m_histp;
			for( int j=m_nchan;j;j-- )
			{
				CalcT in=CalcT(*dest);
				Cascade::Stage *s=stagep;
				for( int i=nstage;i;i--,h++,s++ )
				{
					CalcT out;
					out=s->b[0]*in		+ s->b[1]*h->v[0] + s->b[2]*h->v[1] +
						s->a[1]*h->v[2] + s->a[2]*h->v[3];
					h->v[1]=h->v[0]; h->v[0]=in;
					h->v[3]=h->v[2]; h->v[2]=out;
					in=out;
				}
				*dest++=Ty(in);
			}
			dest+=skip;
		}
	}
}

//------------------------------------------------------------------------------

template<typename Ty>
void CascadeFilter::ProcessII( size_t frames, Ty *dest, int skip )
{
	int nstage=GetStageCount();
	Cascade::Stage *stagep=Stages();
	while( frames-- )
	{
		Hist *h=m_histp;
		for( int j=m_nchan;j;j-- )
		{
			CalcT in=CalcT(*dest);
			Cascade::Stage *s=stagep;
			for( int i=nstage;i;i--,h++,s++ )
			{
				CalcT d2=h->v[2]=h->v[1];
				CalcT d1=h->v[1]=h->v[0];
				CalcT d0=h->v[0]=
					in+s->a[1]*d1 + s->a[2]*d2;
					in=s->b[0]*d0 + s->b[1]*d1 + s->b[2]*d2;
			}
			*dest++=Ty(in);
		}
		dest+=skip;
	}
}

//******************************************************************************
//
// Biquad
//
//******************************************************************************

// Formulas from http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

//------------------------------------------------------------------------------

Biquad::Biquad()
{
	SetStageCount( 1 );
}

//******************************************************************************

void BiquadLp::SetupCommon( CalcT sn, CalcT cs, CalcT q  )
{
	CalcT alph = sn / ( 2 * q );
	CalcT a0 =  1 / ( 1 + alph );
	CalcT b1 =  1 - cs;
	CalcT b0 = a0 * b1 * 0.5;
	CalcT a1 =  2 * cs;
	CalcT a2 = alph - 1;
	SetStage1( a1*a0, a2*a0, b0, b1*a0, b0 );
}

//------------------------------------------------------------------------------

void BiquadLp::Setup( CalcT normFreq, CalcT q  )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);
	SetupCommon( sn, cs, q );
}

//------------------------------------------------------------------------------

void BiquadLp::SetupFast( CalcT normFreq, CalcT q  )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );
	SetupCommon( sn, cs, q );
}

//******************************************************************************

void BiquadHp::SetupCommon( CalcT sn, CalcT cs, CalcT q  )
{
	CalcT alph = sn / ( 2 * q );
	CalcT a0 = -1 / ( 1 + alph );
	CalcT b1 = -( 1 + cs );
	CalcT b0 = a0 * b1 * -0.5;
	CalcT a1 = -2 * cs;
	CalcT a2 =  1 - alph;
	SetStage1( a1*a0, a2*a0, b0, b1*a0, b0 );
}

//------------------------------------------------------------------------------

void BiquadHp::Setup( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);
	SetupCommon( sn, cs, q );
}

//------------------------------------------------------------------------------

void BiquadHp::SetupFast( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );
	SetupCommon( sn, cs, q );
}

//******************************************************************************

void BiquadBp1::SetupCommon( CalcT sn, CalcT cs, CalcT q  )
{
	CalcT alph = sn / ( 2 * q );
	CalcT a0 = -1 / ( 1 + alph );
	CalcT b0 = a0 * ( sn * -0.5 );
	CalcT a1 = -2 * cs;
	CalcT a2 =  1 - alph;
	SetStage1( a1*a0, a2*a0, b0, 0, -b0 );
}

//--------------------------------------------------------------------------

void BiquadBp1::Setup( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);
	SetupCommon( sn, cs, q );
}

//--------------------------------------------------------------------------

void BiquadBp1::SetupFast( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );
	SetupCommon( sn, cs, q );
}

//******************************************************************************

void BiquadBp2::SetupCommon( CalcT sn, CalcT cs, CalcT q  )
{
	CalcT alph = sn / ( 2 * q );
	CalcT b0 = -alph;
	CalcT b2 =  alph;
	CalcT a0 = -1 / ( 1 + alph );
	CalcT a1 = -2 * cs;
	CalcT a2 =  1 - alph;
	SetStage1( a1*a0, a2*a0, b0*a0, 0, b2*a0 );
}

//--------------------------------------------------------------------------

void BiquadBp2::Setup( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);
	SetupCommon( sn, cs, q );
}

//--------------------------------------------------------------------------

void BiquadBp2::SetupFast( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );
	SetupCommon( sn, cs, q );
}

//******************************************************************************

void BiquadBs::SetupCommon( CalcT sn, CalcT cs, CalcT q  )
{
	CalcT alph = sn / ( 2 * q );
	CalcT a0 = 1 / ( 1 + alph );
	CalcT b1 = a0 * ( -2 * cs );
	CalcT a2 = alph - 1;
	SetStage1( -b1, a2*a0, a0, b1, a0 );
}

//--------------------------------------------------------------------------

void BiquadBs::Setup( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);
	SetupCommon( sn, cs, q );
}

//--------------------------------------------------------------------------

void BiquadBs::SetupFast( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );
	SetupCommon( sn, cs, q );
}

//******************************************************************************

void BiquadAp::SetupCommon( CalcT sn, CalcT cs, CalcT q  )
{
	CalcT alph = sn / ( 2 * q );
	CalcT b2 =  1 + alph;
	CalcT a0 =  1 / b2;
	CalcT b0 =( 1 - alph ) * a0;
	CalcT b1 = -2 * cs * a0;
	SetStage1( -b1, -b0, b0, b1, b2*a0 );
}

//--------------------------------------------------------------------------

void BiquadAp::Setup( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);
	SetupCommon( sn, cs, q );
}

//--------------------------------------------------------------------------

void BiquadAp::SetupFast( CalcT normFreq, CalcT q )
{
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );
	SetupCommon( sn, cs, q );
}

//******************************************************************************

void BiquadLs::SetupCommon( CalcT cs, CalcT A, CalcT sa )
{
	CalcT An	= A-1;
	CalcT Ap	= A+1;
	CalcT Ancs	= An*cs;
	CalcT Apcs	= Ap*cs;
	CalcT b0 =	   A * (Ap - Ancs + sa );
	CalcT b2 =	   A * (Ap - Ancs - sa );
	CalcT b1 = 2 * A * (An - Apcs);
	CalcT a2 =    sa - (Ap + Ancs);
	CalcT a0 =	   1 / (Ap + Ancs + sa );
	CalcT a1 = 2 *	   (An + Apcs);
	SetStage1( a1*a0, a2*a0, b0*a0, b1*a0, b2*a0 );
}

//--------------------------------------------------------------------------

void BiquadLs::Setup( CalcT normFreq, CalcT dB, CalcT shelfSlope )
{
	CalcT A  = pow( 10, dB/40 );
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);
	CalcT al = sn / 2 * ::std::sqrt( (A + 1/A) * (1/shelfSlope - 1) + 2 );
	CalcT sa =		2 * ::std::sqrt( A ) * al;
	SetupCommon( cs, A, sa );
}

//--------------------------------------------------------------------------

// This could be optimized further
void BiquadLs::SetupFast( CalcT normFreq, CalcT dB, CalcT shelfSlope )
{
	CalcT A  = pow( 10, dB/40 );
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );
	CalcT al = sn / 2 * fastsqrt1( (A + 1/A) * (1/shelfSlope - 1) + 2 );
	CalcT sa =		2 * fastsqrt1( A ) * al;
	SetupCommon( cs, A, sa );
}

//******************************************************************************

void BiquadHs::SetupCommon( CalcT cs, CalcT A, CalcT sa )
{
	CalcT An	= A-1;
	CalcT Ap	= A+1;
	CalcT Ancs	= An*cs;
	CalcT Apcs	= Ap*cs;
	CalcT b0 =		A * (Ap + Ancs + sa );
	CalcT b1 = -2 * A * (An + Apcs);
	CalcT b2 =		A * (Ap + Ancs - sa );
	CalcT a0 =		1 / (Ap - Ancs + sa );
	CalcT a2 =			 Ancs + sa - Ap;
	CalcT a1 = -2	*	(An - Apcs);
	SetStage1( a1*a0, a2*a0, b0*a0, b1*a0, b2*a0 );
}

//--------------------------------------------------------------------------

void BiquadHs::Setup( CalcT normFreq, CalcT dB, CalcT shelfSlope )
{
	CalcT A  = pow( 10, dB/40 );
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);

	CalcT alph = sn / 2 * ::std::sqrt( (A + 1/A) * (1/shelfSlope - 1) + 2 );
	CalcT sa   =	  2 * ::std::sqrt( A ) * alph;
	SetupCommon( cs, A, sa );
}

//--------------------------------------------------------------------------

void BiquadHs::SetupFast( CalcT normFreq, CalcT dB, CalcT shelfSlope )
{
	CalcT A  = pow( 10, dB/40 );
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );

	CalcT alph = sn / 2 * fastsqrt1( (A + 1/A) * (1/shelfSlope - 1) + 2 );
	CalcT sa   =	  2 * fastsqrt1( A ) * alph;
	SetupCommon( cs, A, sa );
}

//******************************************************************************

void BiquadEq::SetupCommon( CalcT sn, CalcT cs, CalcT alph, CalcT A )
{
	CalcT t=alph*A;
	CalcT b0 =  1 - t;
	CalcT b2 =  1 + t;
	t=alph/A;
	CalcT a0 =  1 / ( 1 + t );
	CalcT a2 =  t - 1;
	CalcT b1 = a0 * ( -2 * cs );
	CalcT a1 = -b1;

	SetStage1( a1, a2*a0, b0*a0, b1, b2*a0 );
}

//--------------------------------------------------------------------------

void BiquadEq::Setup( CalcT normFreq, CalcT dB, CalcT bandWidth )
{
	CalcT A  = pow( 10, dB/40 );
	CalcT w0 = 2 * kPi * normFreq;
	CalcT cs = cos(w0);
	CalcT sn = sin(w0);
	CalcT alph = sn * sinh( kLn2/2 * bandWidth * w0/sn );
	SetupCommon( sn, cs, alph, A );
}

//--------------------------------------------------------------------------

void BiquadEq::SetupFast( CalcT normFreq, CalcT dB, CalcT bandWidth )
{
	CalcT A  = pow( 10, dB/40 );
	CalcT w0 = 2 * kPi * normFreq;
	CalcT sn, cs;
	fastsincos( w0, &sn, &cs );
	CalcT alph = sn * sinh( kLn2/2 * bandWidth * w0/sn );
	SetupCommon( sn, cs, alph, A );
}

//******************************************************************************
//
// Layout
//
//******************************************************************************

void Layout::Realize( Cascade *cascade ) {
	Realize_custom(cascade);
	//Realize_orig(cascade);
}

// I had as .00000001, but couldn't make 12th order butter
const double smallNumber = .00000001;
bool closeTo(double x, double y, double tolerance) {
	return std::abs(x - y) < tolerance;
}

void fillStageOneRoot_notNormalized(double * taps, Complex root) {
	if (!closeTo(root.imag(), 0, smallNumber)) {
		throw std::runtime_error("unmatched complex root");
	}


	/*  h(z) = root - (z**-1)
	 * will be zero when z**-1 == root
	 * or z == 1/root
	 */
	taps[0] = 1.0 / root.real();
	taps[1] = -1;
	taps[2] = 0;
}

bool areConjugates(const Complex& root1, const Complex& root2) {
	return closeTo(root1.real(), root2.real(), smallNumber) && 
		closeTo(root1.imag(), -root2.imag(), smallNumber);
	//return (root1.real() == root2.real() &&
	//	closeTo(root1.imag(), -root2.imag(), smallNumber));
}

bool isComplex(const Complex& x) {

	return !closeTo(x.imag(), 0, smallNumber);
}

// (z-p1)*(z-p2) = z**2 -  ( p1 +  p2) z + p1 * p2
// = (p1 * p2) +  ( -(p1 + p2)) * z +  z2 
// = (p1 * p2) * z**-2 - (p1 + p2) * z**-1 + 1
void fillStageTwoRoots_notNormalized(double * taps, Complex root1, Complex root2) {
	if ( (root1.imag() == 0) && (root2.imag()==0)) {
		// both are real is ok
	}
	else if ( areConjugates(root1, root2)) {
	}
	else
		throw std::runtime_error("unmatched complex roots");


	//  h(z) = root - z  ;will be zero when z == root
	taps[0] = 1.0;
	taps[1] = -(root1 + root2).real();
	taps[2] =(root1 * root2).real();
	//taps[0] = (root1 * root2).real();
	//taps[1] = -(root1 + root2).real();
	//taps[2] =1;
}
/* On input: H(z) = 1 /  (a0 + a1z + a2z);
 * on output H(z) = 1 /  (1 - a1z - a2z)
 *
 * we drop the scale factor - it will be recalculated later
 */
void normalizeDenominator(double * denTaps) {
	denTaps[1] *= -1.0;
	denTaps[2] *= -1.0;		// now a0 - a1z -a2z

	double scale = 1.0 / denTaps[0];

	denTaps[0] *= scale;
	denTaps[1] *= scale;
	denTaps[2] *= scale;
	
}
void fillStageOnePole(Cascade::Stage * stage, Complex pole) {
	double * taps = stage->a;
	fillStageOneRoot_notNormalized(taps, pole);
	normalizeDenominator(taps);
}

void fillStageTwoPoles(Cascade::Stage * stage, Complex pole1, Complex pole2) {
	double * taps = stage->a;
	fillStageTwoRoots_notNormalized(taps, pole1, pole2);
	normalizeDenominator(taps);
}
void fillStageTwoZeros(Cascade::Stage * stage, Complex zero1, Complex zero2) {
	double * taps = stage->b;
	fillStageTwoRoots_notNormalized(taps, zero1, zero2);
}

void fillStageOneZero(Cascade::Stage * stage, Complex zero) {
	double * taps = stage->b;
	fillStageOneRoot_notNormalized(taps, zero);
}

int locateMatchingRoot(Roots& roots, int indexThatNeedsMatch, int indexMatchDestination) {
	const Complex&  rootToMatch = roots.GetNth(indexThatNeedsMatch);
	for (int i = indexThatNeedsMatch+1; i< roots.GetCount(); ++i) {
		Complex& root = roots.GetNth(i);
		if (areConjugates(rootToMatch, root)) {
			return i;
		}
	}
	throw std::runtime_error("unmatched complex roots c");
}

void pairRoots2(Roots& roots) {
	int num = roots.GetCount();
	for (int i=0; i<num; ++i) {
		//int stageNum = i/2;
		Complex& root = roots.GetNth(i);
		if ( isComplex(root)) {
			if (i % 2) {
				// if odd, we have an unmatched (we are the second half of a biquad
				const int match = locateMatchingRoot(roots, i, i-1);

				// now swap the unmatched root for the match
				Complex temp = roots.GetNth(i-1);	// save off the one we are moving
				roots.GetNth(i-1) = roots.GetNth(match);
				roots.GetNth(match) = temp;	
			}
			else if (i >= (num-1)) {
				// if no more to pair with, we are hosed
				throw std::runtime_error("unmatched complex roots b");
			}
			else if (areConjugates(root, roots.GetNth(i + 1)) ){
				// if next root already a match, we are cool
				++i;			// skip over the matching
			}
			else {
				// here we are first of the pair, but our mate is missing
				const int match = locateMatchingRoot(roots, i, i+1);

				// now swap the unmatched root for the match
				Complex temp = roots.GetNth(i+1);	// save off the one we are moving
				roots.GetNth(i+1) = roots.GetNth(match);
				roots.GetNth(match) = temp;	
				++i;		// advance over our mate

			}
		}
	}

}

#if 0 // unused, for now
static void dumpRoots(const char * title, Roots &roots) {
	char buf[512];
	sprintf_s(buf, sizeof(buf), "\nDump Roots: %s\n", title);
	//DebugUtil::trace(buf);

	for (int i=0; i<roots.GetCount(); ++i) {
		sprintf_s(buf, sizeof(buf), "[%d] = %f,%f\n", i, roots.GetNth(i).real(), roots.GetNth(i).imag());
		//DebugUtil::trace(buf);
	}

}
#endif


void Layout::pairRoots() {
#ifdef _LOG
	dumpRoots(" poles ", Poles());
	dumpRoots(" zeros ", Zeros());
#endif
	pairRoots2(Poles());
	pairRoots2(Zeros());
}

void Layout::Realize_custom( Cascade *cascade )
{
	// Calculate number of second order sections required.
	int poles = CountPoles();		
	int zeros = CountZeros();
	const int stages=(poles+1)/2;
#ifdef _LOG
	
	{
		char buf[512];
		DebugUtil::trace("enter realize, poles = ");
		for (int i=0; i<poles; ++i) {
			sprintf(buf, " [%d]=%f,%f", i, Pole(i).real(), Pole(i).imag());
			DebugUtil::trace(buf);
		}
		DebugUtil::trace("\n");
		DebugUtil::trace("enter realize, zeros = ");
		for (int i=0; i<zeros; ++i) {
			sprintf(buf, " [%d]=%f,%f", i, Zero(i).real(), Zero(i).imag());
			DebugUtil::trace(buf);
		}
		DebugUtil::trace("\n");
	}
#endif
	pairRoots();

#ifdef _LOG
	{
		char buf[512];
		DebugUtil::trace("sorted, poles = ");
		for (int i=0; i<poles; ++i) {
			sprintf(buf, " [%d]=%f,%f", i, Pole(i).real(), Pole(i).imag());
			DebugUtil::trace(buf);
		}
		DebugUtil::trace("\n");
		DebugUtil::trace("sorted, zeros = ");
		for (int i=0; i<zeros; ++i) {
			sprintf(buf, " [%d]=%f,%f", i, Zero(i).real(), Zero(i).imag());
			DebugUtil::trace(buf);
		}
		DebugUtil::trace("\n");
	}
#endif

	int sz = (zeros +1)/2;
	if (stages != sz) {
			throw std::runtime_error("realize with mismatched stage count");
	}
	cascade->SetStageCount( stages );
	

	cascade->Reset();

	for (int i=0; i<stages; ++ i) {
		Cascade::Stage *s= (cascade->Stages()+ i);
		if (poles == 1) {
			Complex c=Pole(i * 2);
			fillStageOnePole(s, c);
			--poles;
		}
		else {
			Complex c1=Pole(i * 2);
			Complex c2=Pole(i * 2 + 1);
			fillStageTwoPoles(s, c1, c2);
			poles -= 2;
		}
		if (zeros == 1) {
			Complex c=Zero(i * 2);
			fillStageOneZero(s, c);
			--zeros;
		}
		else {
			Complex c1=Zero(i * 2);
			Complex c2=Zero(i * 2 + 1);
			fillStageTwoZeros(s, c1, c2);
			zeros -= 2;
		}
		
	}

	// Normalization

	// The assertions are arbitrary....
	assert(m_normal.w >= 0 && m_normal.w <= kPi);
	assert(m_normal.gain < 1000000000000);
	assert(m_normal.gain > -1000000000000);

	cascade->Scale( m_normal.gain / std::abs( cascade->Response_radian( m_normal.w ) ) );
}

void Layout::Realize_orig( Cascade *cascade )
{
	// Calculate number of second order sections required.
	{
		int s1=(CountPoles()+1)/2;
		int s2=(CountZeros()+1)/2;
		assert( s1==s2 ); // I am not sure if it works otherwise
		cascade->SetStageCount( s1>s2?s1:s2 );
	}

	cascade->Reset();

	int n;

	// Poles

	n=0;
	for( int i=0;i<CountPoles();i++ )
	{
		Complex c=Pole(i);

#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "Realize pole at %f, %f\n", c.real(), c.imag());
			DebugUtil::trace(buf);
		}
#endif
		if( std::abs(c.imag())<1e-6 )
			c=Complex( c.real(), 0 );
		if( c.imag()==0 )
			BuildA( cascade, c.real(), 0, &n );
		else if( c.imag()>0 )
			BuildA( cascade, 2*c.real(), -std::norm(c), &n );
	}

	// Zeros

	n=0;
	for( int i=0;i<CountZeros();i++ )
	{
		Complex c=Zero(i);
#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "Realize zero at %f, %f\n", c.real(), c.imag());
			DebugUtil::trace(buf);
		}
#endif
		if( ::std::abs(c.imag())<1e-6 )
			c=Complex( c.real(), 0 );
		if( c.imag()==0 ) {
			//DebugUtil::trace("Realize img==0\n");
			BuildB( cascade, -c.real(), 1, 0, &n );
		}
		else if( c.imag()>0 ) {
			//DebugUtil::trace("Realize img>0\n");
			BuildB( cascade, std::norm(c), -2*c.real(), 1, &n );
		}
		else {
			//DebugUtil::trace("Realize img<0\n");
		}
	}

	// Normalization

	assert(m_normal.w >= 0 && m_normal.w <= 2.0);
	assert(m_normal.gain < 1000000000000);
	assert(m_normal.gain > -1000000000000);

	cascade->Scale( m_normal.gain / std::abs( cascade->Response_radian( m_normal.w ) ) );
};

//------------------------------------------------------------------------------

void Layout::BuildA( Cascade *cascade, CalcT x1, CalcT x2, int *na )
{
	if( x2!=0 )
	{
		Cascade::Stage *s=cascade->Stages()+cascade->GetStageCount()-1-*na;
		assert( s->a[1]==0 && s->a[2]==0 );
		s->a[1]=x1;
		s->a[2]=x2;
		(*na)++;
#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "builda2, set a1 to %f a2 %f\n", s->a[1], s->a[2]);
			DebugUtil::trace(buf);
		}
#endif
	}
	else
	{
		// combine
		Cascade::Stage *s=cascade->Stages();
		s->a[2]=-s->a[1]*x1;
		s->a[1]+=x1;
#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "builda1, set a1 to %f a2 %f\n", s->a[1], s->a[2]);
			DebugUtil::trace(buf);
		}
#endif
		if( s->a[2]!=0 )
		{
			int n=cascade->GetStageCount()-1-*na;
			if( n>0 )
			{
				Cascade::Stage *f=cascade->Stages()+n;
				f->a[1]=s->a[1];
				f->a[2]=s->a[2];
				s->a[1]=0;
				s->a[2]=0;
#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "builda1, moved a UP, set a1, a2 to zero\n");
			DebugUtil::trace(buf);
		}
#endif
				(*na)++;
			}
		}
	}
}

//------------------------------------------------------------------------------

void Layout::BuildB( Cascade *cascade, CalcT x0, CalcT x1, CalcT x2, int *nb )
{
	
#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "enter buildb x0=%f x1=%f x2=%f\n", 
				x0, x1, x2);
			DebugUtil::trace(buf);
		}
#endif
	if( x2!=0 )
	{
		Cascade::Stage *s=cascade->Stages()+cascade->GetStageCount()-1-*nb;
		s->b[0]=x0;
		s->b[1]=x1;
		s->b[2]=x2;
		(*nb)++;

#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "buildb, set b0 to %f b1 to %f b2 %f\n", s->b[0], s->b[1], s->b[2]);
			DebugUtil::trace(buf);
		}
#endif
	}
	else
	{
	

		// combine
		// (b0 + z b1)(x0 + z x1) = (b0 x0 + (b1 x0+b0 x1) z + b1 x1 z^2)
		Cascade::Stage *s=cascade->Stages();

#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "enter buildb2  b0=%f b1=%f b2=%f \n", 
				s->b[0], s->b[1], s->b[2]
				);
			DebugUtil::trace(buf);
		}
#endif

		s->b[2]=s->b[1]*x1;
		s->b[1]=s->b[1]*x0+s->b[0]*x1;
		s->b[0]*=x0;

#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "buildb 2, set b0 to %f b1 to %f b2 %f\n", s->b[0], s->b[1], s->b[2]);
			DebugUtil::trace(buf);
		}
#endif
		if( s->b[2]!=0 )
		{
			int n=cascade->GetStageCount()-1-*nb;
			if( n>0 )
			{
				Cascade::Stage *f=cascade->Stages()+n;
				f->b[0]=s->b[0];
				f->b[1]=s->b[1];
				f->b[2]=s->b[2];
				s->b[0]=1;
				s->b[1]=0;
				s->b[2]=0;
				(*nb)++;
#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "buildb 3, set b to 1, bnext b0 to %f b1 to %f b2 %f\n", f->b[0], f->b[1], f->b[2]);
			DebugUtil::trace(buf);
		}
#endif
			}
		}
	}
}

//******************************************************************************

const Complex Dsp::infinity(std::numeric_limits<CalcT>::infinity());
//const Complex Dsp::positiveInfinity( 2 );

//******************************************************************************
//
// Transformations
//
//******************************************************************************

void LowPass::Transform( const Spec &spec, Layout *result, const Layout &layout )
{
	Transform( spec, &result->Poles(), layout.Poles() );
	Transform( spec, &result->Zeros(), layout.Zeros() );
	
	Normalization n0=layout.GetNormalization();
	Normalization &n=result->GetNormalization();
	n=n0;
}

//------------------------------------------------------------------------------

void LowPass::Transform( const Spec &spec, Roots *result, const Roots &roots )
{
	CalcT w=2*kPi*spec.cutoffFreq/spec.sampleRate;
	// prewarp
	CalcT k=tan(w*0.5); 
	result->SetCount( roots.GetCount() );

	assert (k > 0);
#ifdef _LOG
	{
			char buf[512];
			sprintf(buf, "trans fc=%f sr = %f w = %f k = %f\n",
				spec.cutoffFreq, spec.sampleRate, w, k);
			DebugUtil::trace(buf);
	}
#endif

	for( int i=0;i<roots.GetCount();i++ )
	{
		const Complex &r=roots.GetNth(i);
		Complex &c=result->GetNth(i);

		if( r==infinity )
		{
			c=Complex( -1, 0 );
		}
		else
		{
			// frequency transform
			c=r*k;
			// bilinear low-pass xform
			c=(1.+c)/(1.-c);
		}
#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "trans[%d] from %f,%f to %f,%f\n", i, 
				r.real(), r.imag(),
				c.real(), c.imag()
				);
			DebugUtil::trace(buf);
		}
#endif
	}
}

//------------------------------------------------------------------------------

void HighPass::Transform( const Spec &spec, Layout *result, const Layout &layout )
{
	Transform( spec, &result->Poles(), layout.Poles() );
	Transform( spec, &result->Zeros(), layout.Zeros() );

	Normalization n0=layout.GetNormalization();
	Normalization &n=result->GetNormalization();
	n.w=kPi-n0.w;
	n.gain=n0.gain;
}

//------------------------------------------------------------------------------

void HighPass::Transform( const Spec &spec, Roots *result, const Roots &roots )
{




	CalcT w=2*kPi*spec.cutoffFreq/spec.sampleRate;
	CalcT k=1./tan(w*0.5); // prewarp
	result->SetCount( roots.GetCount() );


#ifdef _LOG
	{
			char buf[512];
			sprintf(buf, "HP trans fc=%f width=%f sr = %f w = %f k = %f\n",
				spec.cutoffFreq,
				spec.normWidth,
				spec.sampleRate, w, k);
			DebugUtil::trace(buf);
	}
#endif


	for( int i=0;i<roots.GetCount();i++ )
	{
		const Complex &r=roots.GetNth(i);
		Complex &c=result->GetNth(i);

		if( r==infinity )
		{
			c=Complex( 1, 0 );
		}
		else
		{
			// frequency transform
			c=r*k;
			// bilinear high-pass xform
			c=-(1.+c)/(1.-c);
		}
#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "trans[%d] from %f,%f to %f,%f\n", i, 
				r.real(), r.imag(),
				c.real(), c.imag()
				);
			DebugUtil::trace(buf);
		}
#endif
	}
}

//------------------------------------------------------------------------------

void BandPass::Transform( const Spec &spec, Layout *result, const Layout &layout )
{
	//DebugUtil::trace("----- BandPass::Transform (poles then zeros) -----\n"); 
	Transform( spec, &result->Poles(), layout.Poles() );
	Transform( spec, &result->Zeros(), layout.Zeros() );

	Normalization n0=layout.GetNormalization();
	Normalization &n=result->GetNormalization();

	if( n0.w==0 ) // hack
	{
		CalcT angularWidth=2*kPi*spec.normWidth;
		CalcT wc2=2*kPi*spec.centerFreq/spec.sampleRate-(angularWidth/2);
		CalcT wc =wc2+angularWidth;
		if( wc2<1e-8 )
			wc2=1e-8;
		if( wc >kPi-1e-8 )
			wc =kPi-1e-8;

		wc+=n0.w;
		wc2+=n0.w;
		n.w=2*atan(sqrt(tan(wc*0.5)*tan(wc2*0.5)));
	}
	else
	{
		// yes this is a giant hack to make shelves work
		n.w=((spec.centerFreq/spec.sampleRate)<0.25)?kPi:0;
	}
	n.gain=n0.gain;
}

//------------------------------------------------------------------------------

void BandPass::Transform( const Spec &spec, Roots *result, const Roots &roots )
{
	CalcT angularWidth=2*kPi*spec.normWidth;
	m_wc2=2*kPi*spec.centerFreq/spec.sampleRate-(angularWidth/2);
	m_wc =m_wc2+angularWidth;

	// Probably filter spec is whack
	assert(m_wc2 >= 1e-8);
	assert(m_wc <= kPi-1e-8 );

	if( m_wc2<1e-8 )
		m_wc2=1e-8;
	if( m_wc >kPi-1e-8 )
		m_wc =kPi-1e-8;

	int n=roots.GetCount();
	result->SetCount( n*2 );


#ifdef _LOG
	{
			char buf[512];
			sprintf(buf, "BP trans fc=%f width=%f sr = %f wc2 = %f wc = %f\n",
				spec.cutoffFreq,
				spec.normWidth,
				spec.sampleRate, m_wc2, m_wc);
			DebugUtil::trace(buf);
	}
#endif

	for( int i=0;i<n;i++ )
	{
		int j=2*i;
		Complex c=roots.GetNth(i);
		if( c==infinity )
		{
			result->GetNth(j)=Complex( -1, 0 );
			result->GetNth(j+1)=Complex( 1, 0 );
		}
		else
		{
			// bilinear transform
			c=(1.+c)/(1.-c); 
			result->GetNth(j)=BandPassTransform( j, c );
			result->GetNth(j+1)=BandPassTransform( j+1, c );
		}
	#ifdef _LOG
		{
			char buf[512];
			sprintf(buf, "trans[%d] from %f,%f to %f,%f + %f,%f\n", i, 
				c.real(), c.imag(),
				result->GetNth(j).real(), result->GetNth(j).imag(),
				result->GetNth(j+1).real(), result->GetNth(j+1).imag()
				);
			DebugUtil::trace(buf);
		}
#endif
	}
}

//------------------------------------------------------------------------------

Complex BandPass::BandPassTransform( int i, const Complex &c )
{
	CalcT a=  cos((m_wc+m_wc2)*0.5)/
			  cos((m_wc-m_wc2)*0.5);
	CalcT b=1/tan((m_wc-m_wc2)*0.5);
	Complex c2(0);
	c2=addmul( c2, 4*(b*b*(a*a-1)+1), c );
	c2+=8*(b*b*(a*a-1)-1);
	c2*=c;
	c2+=4*(b*b*(a*a-1)+1);
	c2=std::sqrt( c2 );
	if ((i & 1) == 0)
		c2=-c2;
	c2=addmul( c2, 2*a*b, c );
	c2+=2*a*b;
	Complex c3(0);
	c3=addmul( c3, 2*(b-1), c );
	c3+=2*(1+b);
	return c2/c3;
}

//------------------------------------------------------------------------------

void BandStop::Transform( const Spec &spec, Layout *result, const Layout &layout )
{
	Transform( spec, &result->Poles(), layout.Poles() );
	Transform( spec, &result->Zeros(), layout.Zeros() );

	Normalization n0=layout.GetNormalization();
	Normalization &n=result->GetNormalization();

	n.w=((spec.centerFreq/spec.sampleRate)<0.25)?kPi:0;
	n.gain=n0.gain;
}

void BandStop::Transform( const Spec &spec, Roots *result, const Roots &roots )
{
	CalcT angularWidth=2*kPi*spec.normWidth;
	m_wc2=2*kPi*spec.centerFreq/spec.sampleRate-(angularWidth/2);
	m_wc =m_wc2+angularWidth;
	if( m_wc2<1e-8 )
		m_wc2=1e-8;
	if( m_wc >kPi-1e-8 )
		m_wc =kPi-1e-8;

	int n=roots.GetCount();
	result->SetCount( n*2 );
	for( int i=0;i<n;i++ )
	{
		Complex c=roots.GetNth(i);
		if( c==infinity )
		{
			c=Complex( -1, 0 );
		}
		else
		{
			// bilinear transform
			c=(1.+c)/(1.-c); 
		}
		int j=2*i;
		result->GetNth(j)=BandStopTransform( j, c );
		result->GetNth(j+1)=BandStopTransform( j+1, c );
	}
}

//------------------------------------------------------------------------------

void BandStop::DesignZeros( const Spec &spec, Layout *layout )
{
	int n=spec.order;
	Roots *roots=&layout->Zeros();
	roots->SetCount( n*2 );
	for( int i=0;i<n*2;i++ )
	{
		roots->GetNth(i)=BandStopTransform( i, Complex( -1 ) );
	}
}

//------------------------------------------------------------------------------

Complex BandStop::BandStopTransform( int i, const Complex &c )
{
	CalcT a=cos((m_wc+m_wc2)*.5) /
			cos((m_wc-m_wc2)*.5);
	CalcT b=tan((m_wc-m_wc2)*.5);
	Complex c2(0);
	c2=addmul( c2, 4*(b*b+a*a-1), c );
	c2+=8*(b*b-a*a+1);
	c2*=c;
	c2+=4*(a*a+b*b-1);
	c2=std::sqrt( c2 );
	c2*=((i&1)==0)?.5:-.5;
	c2+=a;
	c2=addmul( c2, -a, c );
	Complex c3( b+1 );
	c3=addmul( c3, b-1, c );
	return c2/c3;
}

//******************************************************************************
//
// Prototype
//
//******************************************************************************

//******************************************************************************
//
// Butterworth
//
//******************************************************************************

void Butter::Design( const Spec &spec )
{
	int n=spec.order;

	SetPoles( n );
	SetZeros( n );
	for( int i=0;i<n;i++ )
	{
		Pole(i)=std::polar( tan(kPi_2*0.5), kPi_2+(2*i+1)*kPi/(2*n) );
		Zero(i)=infinity;
	}

	m_normal.w=0;
	m_normal.gain=1;
}

//------------------------------------------------------------------------------

void ButterShelf::Design( const Spec &spec )
{
	int n=spec.order;

	CalcT g=std::pow(10.,spec.gainDb/20.);
	SetPoles( n );
	SetZeros( n );
	for( int i=0;i<n;i++ )
	{
		int m=i+1;

		{
			CalcT a=kPi*(0.5-(2*m-1)/(2.*n));
			CalcT r=-1./::pow( g, 1./(2*n) );
			Pole(i)=Complex(r*cos(a),r*sin(a));
		}

		{
			CalcT a=kPi*(0.5-(2*m-1)/(2.*n));
			CalcT r=-1.*::pow( g, 1./(2*n) );
			Zero(i)=Complex( r*cos(a), r*sin(a) );
		}
	}

	m_normal.w=kPi;
	m_normal.gain=1;
}

//******************************************************************************
//
// Chebyshev Type I
//
//******************************************************************************

// "Chebyshev PoleFilterSpace Properties"
// http://cnx.org/content/m16906/latest/
void ChebyI::Design( const Spec &spec )
{
	int n=spec.order;

	CalcT eps=std::sqrt( 1/std::exp( -spec.passRippleDb*0.1*kLn10 )-1 );
	CalcT v0=asinh(1/eps)/n;

	SetPoles( n );
	SetZeros( n );
	for( int i=0;i<n;i++ )
	{
		int k=2*i+1-n;
		CalcT a=-sinh(v0)*cos(k*kPi/(2*n));
		CalcT b= cosh(v0)*sin(k*kPi/(2*n));
		Pole(i)=Complex( a, b );
		Zero(i)=infinity;
	}

	m_normal.w=0;
	m_normal.gain=(spec.order&1)?1:pow( 10, -spec.passRippleDb/20.0 );
}

//------------------------------------------------------------------------------

// Chebyshev Type I low pass shelf prototype
// From "High-Order Digital Parametric Equalizer Design", Sophocles J. Orfanidis
// http://www.ece.rutgers.edu/~orfanidi/ece521/hpeq.pdf
void ChebyIShelf::Design( const Spec &spec )
{
	int n=spec.order;

	CalcT gainDb=-spec.gainDb;
	CalcT rippleDb=spec.passRippleDb;
	if( rippleDb>=abs(gainDb) )
		rippleDb=abs(gainDb);
	if( gainDb<0 )
		rippleDb=-rippleDb;
	CalcT G=std::pow( 10., gainDb/20.0 );
	CalcT Gb=std::pow( 10., (gainDb-rippleDb)/20.0 );
	CalcT G0=1;
	CalcT g0=pow(G0,1./n);
	CalcT eps;
	if( Gb!=G0 )
		eps=sqrt((G*G-Gb*Gb)/(Gb*Gb-G0*G0));
	else
		eps=G-1; // This is surely wrong
	CalcT b=pow(G/eps+Gb*sqrt(1+1/(eps*eps)), 1./n);
	CalcT u=log(b/g0);
	CalcT v=log(pow(1./eps+sqrt(1+1/(eps*eps)),1./n));

	SetPoles( n );
	SetZeros( n );
	for( int i=0;i<n;i++ )
	{
		int m=i+1;
		CalcT a=kPi*(2*m-1)/(2*n);
		CalcT sn=sin(a);
		CalcT cs=cos(a);
		Pole(i)	= Complex( -sn*sinh(u), cs*cosh(u) );
		Zero(i)	= Complex( -sn*sinh(v), cs*cosh(v) );
	}

	m_normal.w=kPi;
	m_normal.gain=1;
}

//******************************************************************************
//
// Chebyshev Type II
//
//******************************************************************************

// "Chebyshev PoleFilterSpace Properties"
// http://cnx.org/content/m16906/latest/
void ChebyII::Design( const Spec &spec )
{
	int n=spec.order;

	CalcT eps=::std::sqrt( 1/ (::exp( spec.stopBandDb*0.1*kLn10 )-1 ) );
	CalcT v0=asinh(1/eps)/n;

	SetPoles( n );
	SetZeros( n );
	for( int i=0;i<n;i++ )
	{
		int k=2*i+1-n;
		CalcT a=-sinh(v0)*cos(k*kPi/(2*n));
		CalcT b= cosh(v0)*sin(k*kPi/(2*n));
		Pole(i)=Complex( a/(a*a+b*b), b/(a*a+b*b) );

		b=1/cos((2*k+1)*kPi/(2*n));
		Zero(i)=Complex( 0, b );
	}

	m_normal.w=0;
	m_normal.gain=1;
}

//------------------------------------------------------------------------------

// Chebyshev Type II low pass shelf prototype
// From "High-Order Digital Parametric Equalizer Design", Sophocles J. Orfanidis
// http://www.ece.rutgers.edu/~orfanidi/ece521/hpeq.pdf
void ChebyIIShelf::Design( const Spec &spec )
{
	int n=spec.order;

	CalcT gainDb=-spec.gainDb;
	CalcT rippleDb=spec.passRippleDb;
	gainDb-=rippleDb;
	if( rippleDb>=abs(gainDb) )
		rippleDb=abs(gainDb);
	if( gainDb<0 )
		rippleDb=-rippleDb;
	CalcT G=std::pow( 10., gainDb/20.0 );
	CalcT Gb=std::pow( 10., (rippleDb)/20.0 );
	CalcT G0=1;
	CalcT g=pow(G,1./n);
	CalcT eps;
	if( Gb!=G0 )
		eps=sqrt((G*G-Gb*Gb)/(Gb*Gb-G0*G0));
	else
		eps=G-1; // This is surely wrong
	CalcT b=pow(G0*eps+Gb*sqrt(1+eps*eps), 1./n);
	CalcT u=log(b/g);
	CalcT v=log(pow(eps+sqrt(1+eps*eps),1./n));

	SetPoles( n );
	SetZeros( n );
	for( int i=0;i<n;i++ )
	{
		int m=i+1;
		CalcT a=kPi*(2*m-1)/(2*n);
		CalcT sn=sin(a);
		CalcT cs=cos(a);
		Pole(i)	= 1./Complex( sn*sinh(u), cs*cosh(u) );
		Zero(i)	= 1./Complex( sn*sinh(v), cs*cosh(v) );
	}

	m_normal.w=kPi;
	m_normal.gain=(spec.order&1)?pow( 10, -spec.passRippleDb/20.0 ):1;
}

//******************************************************************************
//
// Elliptic
//
//******************************************************************************

void Elliptic::Design( const Spec &spec )
{
	int n=spec.order;

	CalcT e2=pow(10.,spec.passRippleDb/10)-1;
	CalcT xi=spec.rollOff+1;
	m_Kprime = ellipticK(sqrt(1-1/(xi*xi)));
	m_K = ellipticK(1/xi);
	int ni = ((n & 1) == 1) ? 0 : 1;
	int i;
	CalcT f[100]; // HACK!!!
	for (i = 1; i <= n/2; i++)
	{
		CalcT u = (2*i-ni)*m_K/n;
		CalcT sn = calcsn(u);
		sn *= 2*kPi/m_K;
		f[i] = m_zeros[i-1] = 1/sn;
	}
	m_zeros[n/2] = 1e30;
	CalcT fb = 1/(2*kPi);
	m_nin = n % 2;
	m_n2 = n/2;
	for (i = 1; i <= m_n2; i++)
	{
		CalcT x = f[m_n2+1-i];
		m_z1[i] = sqrt(1-1/(x*x));
	}
	CalcT ee = e2;//pow(10., rippleDb/20)-1;
	m_e = sqrt(ee);
	CalcT fbb = fb*fb;
	m_m = m_nin+2*m_n2;
	m_em = 2*(m_m/2);
	CalcT tp = 2*kPi;
	calcfz();
	calcqz();
	if (m_m > m_em)
		m_c1[2*m_m] = 0;
	for (i = 0; i <= 2*m_m; i += 2)
		m_a1[m_m-i/2] = m_c1[i] + m_d1[i];
	CalcT a0 = findfact(m_m);
	int r = 0;
	while (r < m_em/2)
	{
		r++;
		m_p[r] /= 10;
		m_q1[r] /= 100;
		CalcT d = 1+m_p[r]+m_q1[r];
		m_b1[r] = (1+m_p[r]/2)*fbb/d;
		m_zf1[r] = fb/pow(d, .25);
		m_zq1[r] = 1/sqrt(abs(2*(1-m_b1[r]/(m_zf1[r]*m_zf1[r]))));
		m_zw1[r] = tp*m_zf1[r];
		m_rootR[r] = -.5*m_zw1[r]/m_zq1[r];
		m_rootR[r+m_em/2] = m_rootR[r];
		m_rootI[r] = .5*sqrt(abs(m_zw1[r]*m_zw1[r]/(m_zq1[r]*m_zq1[r]) - 4*m_zw1[r]*m_zw1[r]));
		m_rootI[r+m_em/2] = -m_rootI[r];
	}
	if (a0 != 0)
	{
		m_rootR[r+1+m_em/2] = -sqrt(fbb/(.1*a0-1))*tp;
		m_rootI[r+1+m_em/2] = 0;
	}

	SetPoles( n );
	SetZeros( n );
	for( int i=0;i<n;i++ )
	{
		Pole(i)=Complex( m_rootR[i+1], m_rootI[i+1] );
		Zero(i)=Complex( 0, m_zeros[i/2]*((i&1)?-1:1) );
	}

	m_normal.w=0;
	m_normal.gain=(spec.order&1)?1:pow( 10, -spec.passRippleDb/20.0 );
}

//------------------------------------------------------------------------------

// generate the product of (z+s1[i]) for i = 1 .. sn and store it in b1[]
// (i.e. f[z] = b1[0] + b1[1] z + b1[2] z^2 + ... b1[sn] z^sn)
void Elliptic::prodpoly( int sn )
{
    m_b1[0] = m_s1[1];
    m_b1[1] = 1;
    int i, j;
    for (j = 2; j <= sn; j++)
	{
		m_a1[0] = m_s1[j]*m_b1[0];
		for (i = 1; i <= j-1; i++)
			m_a1[i] = m_b1[i-1]+m_s1[j]*m_b1[i];
		for (i = 0; i != j; i++)
			m_b1[i] = m_a1[i];
		m_b1[j] = 1;
    }
}

//------------------------------------------------------------------------------

// determine f(z)^2
void Elliptic::calcfz2( int i )
{
    int ji = 0;
    int jf = 0;
    if (i < m_em+2)
	{
		ji = 0;
		jf = i;
    }
    if (i > m_em)
	{
		ji = i-m_em;
		jf = m_em;
    }
    m_c1[i] = 0;
    int j;
    for(j = ji; j <= jf; j += 2)
		m_c1[i] += m_a1[j]*(m_a1[i-j]*pow(10., m_m-i/2));
}

//------------------------------------------------------------------------------

// calculate f(z)
void Elliptic::calcfz( void )
{
    int i = 1;
    if( m_nin == 1 )
		m_s1[i++] = 1;
    for (; i <= m_nin+m_n2; i++)
		m_s1[i] = m_s1[i+m_n2] = m_z1[i-m_nin];
    prodpoly(m_nin+2*m_n2);
    for (i = 0; i <= m_em; i += 2)
		m_a1[i] = m_e*m_b1[i];
    for (i = 0; i <= 2*m_em; i += 2)
		calcfz2(i);
}

//------------------------------------------------------------------------------

// determine q(z)
void Elliptic::calcqz( void )
{
    int i;
    for (i = 1; i <= m_nin; i++)
		m_s1[i] = -10;
    for (; i <= m_nin+m_n2; i++)
		m_s1[i] = -10*m_z1[i-m_nin]*m_z1[i-m_nin];
    for (; i <= m_nin+2*m_n2; i++)
	m_s1[i] = m_s1[i-m_n2];
    prodpoly(m_m);
    int dd = ((m_nin & 1) == 1) ? -1 : 1;
    for (i = 0; i <= 2*m_m; i += 2)
		m_d1[i] = dd*m_b1[i/2];
}

//------------------------------------------------------------------------------

// compute factors
CalcT Elliptic::findfact(int t)
{
    int i;
    CalcT a = 0;
    for (i = 1; i <= t; i++)
		m_a1[i] /= m_a1[0];
    m_a1[0] = m_b1[0] = m_c1[0] = 1;
    int i1 = 0;
    for(;;)
	{
		if (t <= 2)
			break;
		CalcT p0 = 0, q0 = 0;
		i1++;
		for(;;)
		{
			m_b1[1] = m_a1[1] - p0;
			m_c1[1] = m_b1[1] - p0;
			for (i = 2; i <= t; i++)
				m_b1[i] = m_a1[i]-p0*m_b1[i-1]-q0*m_b1[i-2];
			for (i = 2; i < t; i++)
				m_c1[i] = m_b1[i]-p0*m_c1[i-1]-q0*m_c1[i-2];
			int x1 = t-1;
			int x2 = t-2;
			int x3 = t-3;
			CalcT x4 = m_c1[x2]*m_c1[x2]+m_c1[x3]*(m_b1[x1]-m_c1[x1]);
			if (x4 == 0)
				x4 = 1e-3;
			CalcT ddp = (m_b1[x1]*m_c1[x2]-m_b1[t]*m_c1[x3])/x4;
			p0 += ddp;
			CalcT dq = (m_b1[t]*m_c1[x2]-m_b1[x1]*(m_c1[x1]-m_b1[x1]))/x4;
			q0 += dq;
			if (abs(ddp+dq) < 1e-6)
				break;
		}
		m_p[i1] = p0;
		m_q1[i1] = q0;
		m_a1[1] = m_a1[1]-p0;
		t -= 2;
		for (i = 2; i <= t; i++)
			m_a1[i] -= p0*m_a1[i-1]+q0*m_a1[i-2];
		if (t <= 2)
			break;
	}

	if (t == 2)
	{
		i1++;
		m_p[i1] = m_a1[1];
		m_q1[i1] = m_a1[2];
    }
    if (t == 1)
		a = -m_a1[1];

    return a;
}

//------------------------------------------------------------------------------

CalcT Elliptic::calcsn(CalcT u)
{
    CalcT sn = 0;
    int j;
    // q = modular constant
    CalcT q = exp(-kPi*m_Kprime/m_K);
    CalcT v = kPi*.5*u/m_K;
    for (j = 0; ; j++)
	{
		CalcT w = pow(q, j+.5);
		sn += w*sin((2*j+1)*v)/(1-w*w);
		if (w < 1e-7)
			break;
    }
    return sn;
}

//------------------------------------------------------------------------------

CalcT Elliptic::ellipticK(CalcT k)
{
    CalcT a[50];
    CalcT theta[50];
    a[0] = atan(k/sqrt(1-k*k));
    theta[0] = kPi*.5;
    int i = 0;
    for(;;)
	{
		CalcT x = 2/(1+sin(a[i]))-1;
		CalcT y = sin(a[i])*sin(theta[i]);
		a[i+1] = atan(sqrt(1-x*x)/x);
		theta[i+1] = .5*(theta[i]+atan(y/sqrt(1-y*y)));
		CalcT e = 1-a[i+1]*2/kPi;
		i++;
		if (e < 1e-7)
			break;
		if (i == 49)
			break;
    }
    int j;
    CalcT p = 1;
    for (j = 1; j <= i; j++)
		p *= 1+cos(a[j]);
    CalcT x = kPi*.25 + theta[i]/2;
    return log(tan(x))*p;
}

//******************************************************************************
//
// RootFinder
//

void RootFinder::solve( int degree, Roots *roots, bool bPolish, bool bSort )
{
	assert( degree<=m_maxdegree );

	const CalcT EPS=1.0e-14;
	int i,its;
	Complex x,b,c;
	int m=degree;
	// copy of coefficients
	for( int j=0;j<=m;j++ )
		m_ad[j]=m_a[j];
	// for each root
	roots->SetCount( degree );
	for( int j=m-1;j>=0;j-- )
	{
		// initial guess at 0
		x=0.0;
		laguerre( j+1, m_ad, x, its );
		if( abs(imag(x)) <= 2.0*EPS*abs(real(x)) )
			x=Complex( real(x), 0.0 );
		roots->GetNth(j)=x;
		b=m_ad[j+1];
		// deflate
		for (int jj=j;jj>=0;jj--)
		{
			c=m_ad[jj];
			m_ad[jj]=b;
			b=x*b+c;
		}
	}
	// polish
	if( bPolish )
		for( int j=0;j<m;j++ )
			laguerre( degree, m_a, roots->GetNth(j), its );
	// sort
	if( bSort )
		for( int j=1;j<m;j++ )
		{
			x=roots->GetNth(j);
			for( i=j-1;i>=0;i-- )
			{
				if( real(roots->GetNth(i)) <= real(x) )
					break;
				roots->GetNth(i+1)=roots->GetNth(i);
			}
			roots->GetNth(i+1)=x;
		}
}

//------------------------------------------------------------------------------

void RootFinder::laguerre( int degree, Complex a[], Complex &x, int &its)
{
	const int MR=8,MT=10,MAXIT=MT*MR;
	const CalcT EPS=std::numeric_limits<CalcT>::epsilon();
	static const CalcT frac[MR+1]=
		{0.0,0.5,0.25,0.75,0.13,0.38,0.62,0.88,1.0};
	Complex dx,x1,b,d,f,g,h,sq,gp,gm,g2;
	int m=degree;
	for (int iter=1;iter<=MAXIT;iter++) {
		its=iter;
		b=a[m];
		CalcT err=abs(b);
		d=f=0.0;
		CalcT abx=abs(x);
		for (int j=m-1;j>=0;j--) {
			f=x*f+d;
			d=x*d+b;
			b=x*b+a[j];
			err=abs(b)+abx*err;
		}
		err *= EPS;
		if (abs(b) <= err) return;
		g=d/b;
		g2=g*g;
		h=g2-2.0*f/b;
		sq=sqrt(CalcT(m-1)*(CalcT(m)*h-g2));
		gp=g+sq;
		gm=g-sq;
		CalcT abp=abs(gp);
		CalcT abm=abs(gm);
		if (abp < abm) gp=gm;
		dx=std::max(abp,abm) > 0.0 ? CalcT(m)/gp : std::polar(1+abx,CalcT(iter));
		x1=x-dx;
		if (x == x1) return;
		if (iter % MT != 0) x=x1;
		else x -= frac[iter/MT]*dx;
	}

	throw;
}

//------------------------------------------------------------------------------

Complex RootFinder::eval( int degree, const Complex &x )
{
	Complex y;
	if( x!=0. )
	{
		for( int i=0;i<=degree;i++ )
			y+=m_a[i]*pow(x,double(i));
	}
	else
	{
		y=m_a[0];
	}

	return y;
}
//******************************************************************************

// returns factorial(n) (n!)
static double fact( double n )
{
	if( n==0 )
		return 1;
	else
		return n*fact(n-1);
}

// returns the k-th zero based coefficient of the reverse bessel polynomial of degree n
static double reversebessel( int k, int n )
{
	return fact(2*n-k)/((fact(n-k)*fact(k))*pow(2.,n-k));
}

// returns the k-th zero based coefficient of the reverse bessel polynomial of degree n
#if 0	// unused, for now
static double bessel( int k, int n )
{
	return fact(n+k)/((fact(n-k)*fact(k))*pow(2.,k));
}
#endif
//------------------------------------------------------------------------------

void Bessel::Design( const Spec &spec )
{
	int n=spec.order;

	CalcT k=1./sqrt((2*n-1)*log(2.));

	RootFinderSpace<100> rf;
	for( int i=0;i<=n;i++ )
		rf.coef()[i]=Complex( reversebessel( i, n ), 0 );
	rf.solve( n, &Poles() );

	SetZeros( n );
	for( int i=0;i<n;i++ )
	{
		Pole(i)*=k;
		Zero(i)=infinity;
	}

	m_normal.w=0;
	m_normal.gain=1;
}

//------------------------------------------------------------------------------

void BesselShelf::Design( const Spec &spec )
{
#if 0
	int n=spec.order;

	CalcT g=std::pow(10.,spec.gainDb/20.);

	int degree=n;
	double a[100];
	bessel( degree, a ); // degree+1 coefficients in a[]

	CalcT k=1./sqrt((2*n-1)*log(2.));

	VecComplex ca;
	VecComplex co;
	ca.resize(degree+1);
	for( int i=0;i<=degree;i++ )
		ca[i]=Complex( a[i], 0 );
	co.resize(degree);
	RootFinderSpace<100> rf;
	rf.solve( degree, ca, co );
	int count=co.size();

	CalcT r=pow( g, 1./n );

	SetPoles( count );
	for( int i=0;i<n;i++ )
	{
		Zero(i)=Complex( co[i].real(), co[i].imag() );
	}

	a[0]+=g*a[0];
	ca[0]=Complex( a[0], 0 );
	rf.solve( degree, ca, co );
	count=co.size();

	SetZeros( count );
	for( int i=0;i<n;i++ )
	{
		Zero(i)=Complex( co[i].real(), co[i].imag() );
	}

	m_normal.w=kPi;
	m_normal.gain=1;
#endif
}

//******************************************************************************

void Legendere::Design( const Spec &spec )
{
}

//******************************************************************************
//
//	Examples
//
//******************************************************************************

#if 0

inline void Plot( int freq, CalcT value )
{
}

inline void FilteringExample( void )
{
	// each frame holds two samples
	const int frames=2000;
	const int sampleRate=44100;
	float stereoData[2*frames];

	{
		// Create a five pole, two channel high-pass Butterworth filter.
		Dsp::ButterHighPass<5, 2> f;

		// Set the cutoff to 18,000 Hz.
		f.Setup( 18000./sampleRate );

		// Apply the filter to the data in place using Direct Form II.
		f.ProcessII( frames, stereoData );
	}

	{
		// Create a two channel low-pass Biquad filter.
		Dsp::BiquadLowPass<2> f;

		// Set the cutoff to 440Hz, with Q=0.25 using approximations
		// to trigonometric functions for fast parameter changes.
		f.SetupFast( 440./sampleRate, 0.25 );

		// Apply the filter to the data in place using Direct Form I.
		f.ProcessI( frames, stereoData );
	}

	// Deinterleave the data, process it using
	// separate objects, and re-interleave it.
	{
		float leftChannel[frames];
		float rightChannel[frames];

		// De-interleave the data.
		deinterleave( frames, leftChannel, rightChannel, stereoData );

		// Create two four pole pair (8 poles total) Chebyshev
		// type I band-pass single channel filters.
		Dsp::Cheby1BandPass<4, 1> f[2];

		// Set the center frequency to 10,000 Hz with a width
		// of 800Hz, no more than 1dB of ripple in the pass-band.
		f[0].Setup( 10000.0/sampleRate, 800./sampleRate, 1 );
		f[1].Setup( 10000.0/sampleRate, 800./sampleRate, 1 );

		// Process each channel using Direct Form II.
		f[0].ProcessII( frames, leftChannel );
		f[1].ProcessII( frames, rightChannel  );

		// Re-interleave the data.
		interleave( frames, stereoData, leftChannel, rightChannel );
	}

	// Apply separate filters to each channel of stereo data.
	{
		// Create two 2 pole pair Chebyshev type I band-stop single channel filters.
		Dsp::Cheby2BandStop<2, 1> f[2];

		// Set the center frequency to 10,000 Hz with a width
		// of 800Hz, with 24dB attenuation in the stop-band.
		f[0].Setup( 10000.0/sampleRate, 800./sampleRate, 24 );
		f[1].Setup( 10000.0/sampleRate, 800./sampleRate, 24 );

		// Process each channel using Direct Form I. skip is set to 1
		// because we want Process() to jump by 1 sample after each output sample
		f[0].ProcessI( frames, stereoData, 1 );
		f[1].ProcessI( frames, stereoData+1, 1 ); // right channel starts at stereoData+1
	}

	// Use one filter to process each channel of stereo data.
	{
		// Create a 2 pole Butterworth high-pass single channel filter
		Dsp::ButterHighPass<2,1> f;

		// Set the cutoff frequency to 10,000Hz.
		f.Setup( 10000.0/sampleRate );

		// Process the left channel using Direct Form I using skip=1.
		f.ProcessI( frames, stereoData, 1 );

		// Clear the filter's history buffer to get
		// it ready for the right channel.
		f.Clear();

		// Process the right channel using Direct Form I.
		f.ProcessI( frames, stereoData+1, 1 );
	}

	// Use one filter to process two single channel buffers having
	// different underlying types, with Direct Form I used for the first
	// buffer and Direct Form II used for the second. Admittedly this
	// is quite a contrived example.
	{
		float buf1[frames];
		double buf2[frames];

		// Create a 1 pole Butterworth high-pass single channel filter
		Dsp::ButterHighPass<1,1> f;

		// Set the cutoff frequency to 10,000Hz.
		f.Setup( 10000.0/sampleRate );

		// Process buf1 using Direct Form I.
		f.ProcessI( frames, buf1 );

		// Clear the history buffer to avoid
		// contamination, since its a different input signal.
		f.Clear();

		// Process buf2 using Direct Form II
		f.ProcessII( frames, buf2 );

		// Ahhh, the miracle of templates
	}

	// Plot the magnitude and phase response of a filter.
	{
		// Create a 1 pole Butterworth low-pass single channel filter
		Dsp::ButterLowPass<1,1> f;

		// Set the cutoff to 8,000 Hz.
		f.Setup( 8000./sampleRate );

		// Plot the magnitude response at each frequency.
		for( int freq=20;freq<sampleRate/2;freq++ )
		{
			Dsp::Complex c=f.Response( CalcT(freq)/sampleRate );

			// Plot magnitude response.
			Plot( freq, std::abs(c) );

			// Plot the phase response.
			Plot( freq, std::arg(c) );
		}
	}
}

#endif

//******************************************************************************
//
// Explicit template instantiations
//
//******************************************************************************

// These routines only work on floating point types anyway
// so I put the instantiations here. Feel free to add any that you need.

template void CascadeFilter::ProcessI<float>( size_t frames, float *dest, int skip );
template void CascadeFilter::ProcessII<float>( size_t frames, float *dest, int skip );
#ifdef DSP_SSE3_AVAIL
template void ProcessISSEStageStereo<float>( size_t frames, float *dest, Cascade::Stage *stage, CascadeFilter::Hist *h, int skip );
#endif

template void CascadeFilter::ProcessI<double>( size_t frames, double *dest, int skip );
template void CascadeFilter::ProcessII<double>( size_t frames, double *dest, int skip );
#ifdef DSP_SSE3_AVAIL
template void ProcessISSEStageStereo<double>( size_t frames, double *dest, Cascade::Stage *stage, CascadeFilter::Hist *h, int skip );
#endif

//******************************************************************************
/*
	BIBLIOGRAPHY

"Chebyshev PoleFilterSpace Properties"
http://cnx.org/content/m16906/latest/

"High-Order Digital Parametric Equalizer Design"
Sophocles J. Orfanidis
http://www.ece.rutgers.edu/~orfanidi/ece521/hpeq.pdf

*/
//******************************************************************************
/*

To do:

- rewrite of filter specifications
- documentation of filter specifications
- bibliography and references for each formula
- optimize prototypes
- fix chebyI and chebyII shelf formulas for ripple specification 

Changes:

- Added RootFinder, for finding complex roots of polynomials
- Added Bessel low pass analog prototype
- Added BesselLowPass, BesselHighPass, BesselBandPass, and BesselBandStop filters
*/
