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

*******************************************************************************/

/**
 * Portions modified by SquinkyLabs, 2010-2018. 
 *	Added ability to generate biquad pairs.
 *	Fixed bugs
 *	Added hilbert filter designer
 */

#ifndef __DSP_FILTER__
#define __DSP_FILTER__


#include <cfloat>
#include <cmath>
#include <complex>

#include <memory.h>
#include <stdlib.h>
#include <assert.h>

#include <AudioMath.h>
//#define _LOG


#ifdef _MSC_VER
/* MSVC generates a flood of warnings about unreferenced functions */
//#pragma warning(disable:4505)
#else
	#include <sys/types.h>
	#include <ctype.h> 
#endif
//#include "DebugUtil.h"
namespace Dsp
{
	//--------------------------------------------------------------------------

	// Compile-time assert to keep us sane

	#define DSP_JOIN(x, y) DSP_JOIN_AGAIN(x, y)
	#define DSP_JOIN_AGAIN(x, y) x ## y

	#define DSP_STATIC_ASSERT(e, msg) \
		typedef char DSP_JOIN(assertion_ ## msg ## _failed_at_line_, __LINE__) [(e) ? 1 : -1]

	template<class Ty, class U>
	struct is_same { enum { value = false }; };
	template<class Ty>
	struct is_same<Ty, Ty> { enum { value = true }; };

	//--------------------------------------------------------------------------
	//
	//	Configuration
	//
	//--------------------------------------------------------------------------

	// Regardless of the type of sample that the filter operates on (e.g.
	// float or double), all calculations are performed using double (or
	// better) for stability and accuracy. This controls the underlying
	// type used for calculations:
	typedef double CalcT;

	typedef int		Int32;	// Must be 32 bits

	// Must be 64 bits
#ifdef _MSC_VER
	typedef __int64 Int64;
#else
	typedef int64_t Int64;
#endif

	// This is used to prevent denormalization.
	const CalcT vsa=1.0 / 4294967295.0; // for CalcT as float

	// These constants are so important, I made my own copy. If you improve
	// the resolution of CalcT be sure to add more significant digits to these.
	const CalcT kPi		=3.1415926535897932384626433832795028841971;
	const CalcT kPi_2	=1.5707963267948966192313216916397514420986;
	const CalcT kLn2    =0.693147180559945309417;
	const CalcT kLn10	=2.30258509299404568402;

	//--------------------------------------------------------------------------

	template<typename Ty>
	inline Ty asinh( Ty x )
	{
		return log( x+::std::sqrt(x*x+1) );
	}

	template<typename Ty>
	inline Ty acosh( Ty x )
	{
		return log( x+::std::sqrt(x*x-1) );
	}

	//--------------------------------------------------------------------------
	//
	//	Complex
	//
	//--------------------------------------------------------------------------

	template<typename Ty, typename To>
	inline std::complex<Ty> addmul( const std::complex<Ty> &c, Ty v, const std::complex<To> &c1 )
	{
		return std::complex<Ty>( c.real()+v*c1.real(), c.imag()+v*c1.imag() );
	}

	template<typename Ty>
	inline std::complex<Ty> recip( const std::complex<Ty> &c )
	{
		Ty n=1.0/std::norm(c);
		return std::complex<Ty>( n*c.real(), n*c.imag() );
	}

	typedef std::complex<CalcT> Complex;

	//--------------------------------------------------------------------------
	//
	// Numerical Analysis
	//
	//--------------------------------------------------------------------------

	// Container for a set of complex polynomial roots.
	// Storage is provided by a derived class.
	struct Roots
	{
		// Get the number of roots available.
		int GetCount( void ) const
		{
			return m_rootCount;
		}

		// Set the number of roots available up to max.
		void SetCount( int n )
		{
			assert( n>=1 && n<=m_rootMax );
			m_rootCount=n;
		}

		// Retrieve zero-based i-th root.
		Complex &GetNth( int i )
		{
			assert( i>=0 && i<m_rootCount );
			return m_root[i];
		}

		// Retrieve zero-based i-th root.
		const Complex &GetNth( int i ) const
		{
			assert( i>=0 && i<m_rootCount );
			return m_root[i];
		}

	protected:
		int			m_rootMax;		// max roots of storage
		Complex *		m_root;			// array of roots
	private:
		int			m_rootCount;	// actual number of roots
	};

	//--------------------------------------------------------------------------

	// Storage for Roots.
	template<int maxdegree>
	struct RootsSpace : Roots
	{
		RootsSpace()
		{
			m_rootMax=sizeof(m_roots)/sizeof(m_roots[0]);
			m_root=m_roots;
		}

	private:
		Complex m_roots[maxdegree];
	};

	//--------------------------------------------------------------------------

	// Finds the complex roots of the given polynomial with
	// complex-valued coefficients using a numerical method.
	struct RootFinder
	{
		// Find roots of polynomial f(x)=a[0]+a[1]*x+a[2]*x^2...+a[degree]*x^degree
		// The input coefficients are set using coef()[].
		// The solutions are placed in roots.
		void solve( int degree, Roots *roots, bool bPolish=false, bool bSort=false );

		// Evaluates the polynomial at x
		Complex eval( int degree, const Complex &x );

		// Direct access to the input coefficient array of size degree+1.
		Complex *coef( void )
		{
			return m_a;
		}

	private:
		// Improves x as a root using Laguerre's method.
		// The input coefficient array has degree+1 elements.
		void laguerre( int degree, Complex a[], Complex &x, int &its );

	protected:
		int m_maxdegree;
		Complex *m_a;		// input coefficients (m_maxdegree+1 elements)
		Complex *m_ad;		// copy of deflating coefficients
	};

	//------------------------------------------------------------------------------

	template<int maxdegree>
	struct RootFinderSpace : virtual RootFinder
	{
		RootFinderSpace()
		{
			m_maxdegree=maxdegree;
			m_a=m_a0;
			m_ad=m_ad0;
		}

	private:
		Complex m_a0[maxdegree+1];
		Complex m_ad0[maxdegree+1];
	};

	//--------------------------------------------------------------------------
	//
	//	Utility Classes
	//
	//--------------------------------------------------------------------------

	// Tracks the peaks in the signal stream using the attack and release parameters
	template<int channels=1>
	class EnvelopeFollower
	{
	public:
		EnvelopeFollower();

		void Setup( double attackMs, double releaseMs, int sampleRate );

		template<typename Ty>
		void Process( size_t count, const Ty *src , int skip=0 );

		CalcT Envelope( void ) const;

		CalcT env[channels];

	protected:
		CalcT a;
		CalcT r;
	};

	//--------------------------------------------------------------------------

	template<int channels>
	EnvelopeFollower<channels>::EnvelopeFollower()
	{
		for( int i=0;i<channels;i++ )
			env[i]=0;
	}

	template<int channels>
	CalcT EnvelopeFollower<channels>::Envelope( void ) const
	{
		return env[0];
	}

	template<int channels>
	void EnvelopeFollower<channels>::Setup( double attackMs, double releaseMs, int sampleRate )
	{
		a = pow( 0.01, 1.0 / ( attackMs * sampleRate * 0.001 ) );
		r = pow( 0.01, 1.0 / ( releaseMs * sampleRate * 0.001 ) );
	}

	template<int channels>
	template<typename Ty>
	void EnvelopeFollower<channels>::Process( size_t count, const Ty *src, int skip )
	{
		skip=channels+skip;
		for( int i=0;i<channels;i++ )
		{
			const Ty *s=src+i;

			CalcT e=env[i];
			for( int n=count;n;n-- )
			{
				double v=::fabs( *s );
				if( v>e )	e = a * ( e - v ) + v;
				else		e = r * ( e - v ) + v;
				s+=skip;
			}
			env[i]=e;
		}
	}

	//--------------------------------------------------------------------------

	// Uses the envelope follower to scale the audio signal into range
	// and prevent clipping. If audio signal is already between 0..1, does
	// nothing. IsClipping() will return true if the last call to Process()
	// contained source data that would have otherwise caused clipping.
	//
	// For musical purposes attack=10ms and release=500ms seems to work well.
	//
	struct AutoLimiter
	{
		void Setup( double attackMs, double releaseMs, int sampleRate );

		bool IsClipping( void ) const;

		template<int skip, class Ty>
		void Process( size_t nSamples, Ty *dest );

	private:
		EnvelopeFollower<1> e;
	};

	//--------------------------------------------------------------------------

	inline void AutoLimiter::Setup( double attackMs, double releaseMs, int sampleRate )
	{
		e.Setup( attackMs, releaseMs, sampleRate );
	}

	inline bool AutoLimiter::IsClipping( void ) const
	{
		return e.Envelope()>1;
	}

	template<int skip, class Ty>
	void AutoLimiter::Process( size_t count, Ty *dest )
	{
		while( count-- )
		{
			Ty v=*dest;
			// don't worry, this should get optimized
			e.Process( 1, &v, skip );
			if( e.Envelope()>1 )
				*dest=Ty(*dest/e.Envelope());
			dest+=skip;
		}
	}

	//--------------------------------------------------------------------------
	//
	// Utility Functions
	//
	//	These may prove useful. Since they support multiple channels
	//	code can measure everything uniformly in terms of number of frames.
	//
	//--------------------------------------------------------------------------

	// Fill a sample buffer with zeroes
	template<typename Ty>
	void zero( int channels, size_t frames, Ty *dest, int destSkip=0 )
	{
		if( destSkip==0 )
		{
			::memset( dest, 0, channels*frames*sizeof(Ty) );
		}
		else
		{
			while( frames-- )
			{
				int n=channels;
				while( n-- )
				{
					*dest++=0;
				}
				dest+=destSkip;
			}
		}
	}

	//--------------------------------------------------------------------------

	// Copy interleaved samples from src to dest (may not overlap).
	// skip specifies the number of samples in between interleaved frames.

	// This will perform a conversion between numerical types
	template<typename Td, typename Ts>
	void copy( int channels, size_t frames, Td *dest, const Ts *src, int destSkip=0, int srcSkip=0 )
	{
		while( frames-- )
		{
			int n=channels;
			while( n-- )
			{
				*dest++=*src++;
			}
			dest+=destSkip;
			src+=srcSkip;
		}
	}

	// Faster version of copy when the source and destination types are the same
	template<typename Ty>
	void copy( int channels, size_t frames, Ty *dest, const Ty *src, int destSkip=0, int srcSkip=0 )
	{
		if( destSkip==0 && srcSkip==0 )
		{
			::memcpy( dest, src, channels * frames * sizeof(src[0]) );
		}
		else
		{
			copy<Ty,Ty>( channels, frames, dest, src, destSkip, srcSkip );
		}
	}

	//--------------------------------------------------------------------------

	// Add each sample in src to dest. Does not perform clipping or overflow testing.
	template<typename Td, typename Ts>
	void mix( int channels, size_t frames, Td *dest, const Ts *src, int destSkip=0, int srcSkip=0 )
	{
		while( frames-- )
		{
			int n=channels;
			while( n-- )
			{
				*dest+++=Td(*src++);
			}
			dest+=destSkip;
			src+=srcSkip;
		}
	}

	//--------------------------------------------------------------------------

	// Multiply each sample by mult. Does not perform clipping or overflow testing.
	template<typename Td, typename Ty>
	void scale( int channels, size_t frames, Td *dest, Ty mult, int destSkip=0 )
	{
		while( frames-- )
		{
			int n=channels;
			while( n-- )
			{
				*dest++=Td(*dest*mult);
			}
			dest+=destSkip;
		}
	}

	//--------------------------------------------------------------------------

	// Half-wave rectify
	template<typename Ty>
	void rectifyhalf( int channels, size_t frames, Ty *dest, int skip=0 )
	{
		while( frames-- )
		{
			int n=channels;
			while( n-- )
			{
				Ty v=*dest;
				if( v<0 )
					v=0;
				*dest++=v;
			}
			dest+=skip;
		}
	}

	//--------------------------------------------------------------------------

	// Full-wave rectify
	template<typename Ty>
	void rectifyfull( int channels, size_t frames, Ty *dest, int skip=0 )
	{
		while( frames-- )
		{
			int n=channels;
			while( n-- )
			{
				*dest++=std::abs(*dest);
			}
			dest+=skip;
		}
	}

	//--------------------------------------------------------------------------

	// Calculate n-order difference
	template<typename Ty>
	void derivative( /*int order,*/ int channels, size_t frames, Ty *dest, int destSkip=0 )
	{
		assert( frames>0 );

		int prev=-(channels+destSkip);
		destSkip=destSkip+2*prev;

		Ty *dest0=dest;

		frames--;
		dest=dest+frames*(channels+destSkip);

		if( frames>1 )
		{
			while( frames-- )
			{
				int n=channels;
				while( n-- )
				{
					*dest++=*dest-dest[prev];
				}
				dest=dest+destSkip;
			}
		}

		int n=channels;
		while( n-- )
		{
			*dest++=0;
		}
	}

	//--------------------------------------------------------------------------

	/*
	template<typename Ty>
	void convolve( int channels, size_t frames, Ty *dest, int kernelSize, const Ty *kernel, int destSkip=0 )
	{
		assert( frames>kernelSize );

		const Ty *kernel0=kernel;

		frames-=kernelSize;
		while( frames-- )
		{
			int n=channels;
			while( n-- )
			{
				kernel=kernel0;
			}
		}
	}
	*/

	//--------------------------------------------------------------------------

	// Interleave separate channels from source pointers to destination
	// (Destination requires channels*frames samples of storage)
	template<typename Td, typename Ts>
	void interleave( int channels, size_t frames, Td *dest, const Ts *src[] )
	{
		assert( channels!=1 );

		switch( channels )
		{
		case 2:
			{
				// unroll further if desired
				const Ts *l=src[0];
				const Ts *r=src[1];
				switch( frames%4 )
				{
				case 3: *dest++=*l++; *dest++=*r++;;
				case 2: *dest++=*l++; *dest++=*r++;;
				case 1: *dest++=*l++; *dest++=*r++;;
				};
				frames/=4;
				while( frames-- )
				{
					*dest++=*l++; *dest++=*r++;
					*dest++=*l++; *dest++=*r++;
					*dest++=*l++; *dest++=*r++;
					*dest++=*l++; *dest++=*r++;
				}
			}
			break;

		default:
			{
				for( int i=0;i<channels;i++ )
					copy( 1, frames, dest+i, src[i], channels-1, 0 );
			}
			break;
		};
	}

	//--------------------------------------------------------------------------

	// Convenience for separate channel pointers.
	template<typename Td, typename Ts>
	void interleave( size_t frames, Td *dest, const Ts *left, const Ts *right )
	{
		const Ts *src[2];
		src[0]=left;
		src[1]=right;
		interleave( 2, frames, dest, src );
	}

	//--------------------------------------------------------------------------

	// Deinterleave channels from interleaved data to separate pointers.
	template<typename Td, typename Ts>
	void deinterleave( int channels, size_t frames, Td *dest[], const Ts *src )
	{
		assert( channels!=1 );

		switch( channels )
		{
		case 2:
			{
				// unroll further if desired
				Td *l=dest[0];
				Td *r=dest[1];
				switch( frames%4 )
				{
				case 3: *l++=*src++; *r++=*src++;
				case 2: *l++=*src++; *r++=*src++;
				case 1: *l++=*src++; *r++=*src++;
				};
				frames/=4;
				while( frames-- )
				{
					*l++=*src++; *r++=*src++;
					*l++=*src++; *r++=*src++;
					*l++=*src++; *r++=*src++;
					*l++=*src++; *r++=*src++;
				}
			}
			break;

		default:
			{
				for( int i=0;i<channels;i++ )
					copy( 1, frames, dest[i], src+i, 0, channels-1 );
			}
			break;
		};
	}

	//--------------------------------------------------------------------------

	// Convenience for separate channel pointers
	template<typename Td, typename Ts>
	void deinterleave( size_t frames, Td *left, Td *right, const Ts *src )
	{
		Td *dest[2];
		dest[0]=left;
		dest[1]=right;
		deinterleave( 2, frames, dest, src );
	}

	//--------------------------------------------------------------------------
	/*
		Units

		w		Angular frequency in radians per sample. 0..pi

	*/
	//--------------------------------------------------------------------------

	// Common structure for all filter specifications.
	struct Spec
	{
		int	order;				// PoleFilterSpace order, >=1
		CalcT sampleRate;		// Sample rate in Hz
		CalcT cutoffFreq;		// Cutoff frequency in Hz
		CalcT passRippleDb;		// Passband ripple in Db
		CalcT stopBandDb;		// Minimum stopband attenuation in Db
		//CalcT cornerFreq1;	// left corner frequency in Hz
		//CalcT cornerFreq2;	// right corner frequency in Hz
		CalcT centerFreq;
		CalcT normWidth;
		CalcT gainDb;			// gain or cut in Db
		CalcT rollOff;			// for elliptics

		Spec( void )
		{
			// This is used as a flag to tell us
			// that the structure is uninitialized.
			order=0;
		}
	};

	//--------------------------------------------------------------------------

	// Information required to normalize the magnitude response of a filter.
	// The Cascade determines the actual magnitude response at w, and applies a
	// scale factor to the coefficients to achieve the specified target gain at w.
	struct Normalization
	{
		CalcT w;		// angular frequency
		CalcT gain;		// target gain
	};

	//--------------------------------------------------------------------------

	// Representation of an Infinite Impulse Response filter modeled
	// as a series of coefficients of second order sections. Derived
	// classes provide storage for the coefficients.
	struct Cascade
	{
		struct Stage;

        // Original source had no virtual destructor.
        // I tried to put one in years ago, and had crashes.
        // Seems ok now.
        virtual ~Cascade()
        {
        }


		// Initializes some important fields.
		Cascade();

		// Return the number of active stages.
		int GetStageCount( void );

		// Set the number of active stages up to max.
		void SetStageCount( int n );

		// Direct access to the stage array.
		Stage *Stages( void );

		// Convenience function for Biquads.
		void SetStage1( CalcT a1, CalcT a2, CalcT b0, CalcT b1, CalcT b2 );
		
		// Reset coefficients in preparation for realization.
		void Reset( void );

		// Add output scale factor to the cascade.
		void Scale( CalcT factor );

		// Determine response at angular frequency.
		// Note that this is the only(?) funciton in the library that works 0.. 2 pi instead of 0..1
		Complex	Response_radian( CalcT w ) const;

		// Determine resonse at normalized freq
		Complex Response_normalized( CalcT f) const { return Response_radian(f * AudioMath::Pi * 2); }

		// Functor for finding the local
		// maximum of the response magnitude.
		struct ResponseFunctor
		{
			ResponseFunctor( Cascade *cascade );
			CalcT operator()( CalcT w );

		private:
			Cascade *m_cascade;
		};

		// The coefficients of one second-order-section.
		struct Stage
		{
			// Reset coefficients.
			void Reset( void )
			{
				a[1]=0; a[2]=0; b[0]=1; b[1]=0; b[2]=0;
			}

			CalcT a[3];
			CalcT b[3];
		};

	protected:
		int		m_stageCount;
		int		m_stageMax;
		Stage *	m_stage;
	};

	//--------------------------------------------------------------------------

	// Storage for Cascade stages.
	template<int maxorder>
	struct CascadeSpace : virtual Cascade
	{
		CascadeSpace()
		{
			m_stageMax=sizeof(m_stages)/sizeof(m_stages[0]);
			m_stage=m_stages;
		}

	private:
		// Each stage is order 2.
		Stage m_stages[(maxorder+1)/2];
	};

	//--------------------------------------------------------------------------

	// Adds the ability to process sample data to a Cascade.
	// Storage for each channel's history information is provided
	// by derived classes.
	struct CascadeFilter : virtual Cascade
	{
		// Clear the history buffer. Used on initialization,
		// and should also be used if the audio source is changed
		// in between filtering.
		void Clear( void );

		// Process data in place using Direct Form I
		// skip is added after each frame.
		// Direct Form I is more suitable when the filter parameters
		// are changed often. However, it is slightly slower.
		template<typename Ty>
		void ProcessI( size_t frames, Ty *dest, int skip=0 );

		// Process data in place using Direct Form II
		// skip is added after each frame.
		// Direct Form II is slightly faster than Direct Form I,
		// but changing filter parameters on stream can result
		// in discontinuities in the output. It is best suited
		// for a filter whose parameters are set only once.
		template<typename Ty>
		void ProcessII( size_t frames, Ty *dest, int skip=0 );

		// Convenience function that just calls ProcessI.
		// Feel free to change the implementation.
		template<typename Ty>
		void Process( size_t frames, Ty *dest, int skip=0 )
		{
			ProcessI( frames, dest, skip );
		}

		// History information for one channel of one stage.
		struct Hist
		{
			CalcT v[4];
		};

	protected:
		int			m_nchan;
		Hist *		m_histp;
	};

	//--------------------------------------------------------------------------
	//
	//	Biquad Second Order IIR Filters
	//
	//--------------------------------------------------------------------------

	// Biquad with stage storage.
	struct Biquad : CascadeSpace<1>
	{
		Biquad();
	};

	//--------------------------------------------------------------------------

	// Low pass
	struct BiquadLp : Biquad
	{
		void Setup			( CalcT normFreq, CalcT q );
		void SetupFast		( CalcT normFreq, CalcT q );
	protected:
		void SetupCommon	( CalcT sn, CalcT cs, CalcT q );
	};

	//--------------------------------------------------------------------------

	// High pass
	struct BiquadHp : Biquad
	{
	public:
		void Setup			( CalcT normFreq, CalcT q );
		void SetupFast		( CalcT normFreq, CalcT q );
	protected:
		void SetupCommon	( CalcT sn, CalcT cs, CalcT q );
	};

	//--------------------------------------------------------------------------

	// Band pass 1
	// Constant skirt gain, peak gain=Q
	struct BiquadBp1 : Biquad
	{
		void Setup			( CalcT normFreq, CalcT q );
		void SetupFast		( CalcT normFreq, CalcT q );
	protected:
		void SetupCommon	( CalcT sn, CalcT cs, CalcT q );
	};

	//--------------------------------------------------------------------------

	// Band pass 2
	// Constant 0dB peak gain
	struct BiquadBp2 : Biquad
	{
		void Setup			( CalcT normFreq, CalcT q );
		void SetupFast		( CalcT normFreq, CalcT q );
	protected:
		void SetupCommon	( CalcT sn, CalcT cs, CalcT q );
	};

	//--------------------------------------------------------------------------

	// Band stop
	struct BiquadBs : Biquad
	{
		void Setup			( CalcT normFreq, CalcT q );
		void SetupFast		( CalcT normFreq, CalcT q );
	protected:
		void SetupCommon	( CalcT sn, CalcT cs, CalcT q );
	};

	//--------------------------------------------------------------------------

	// All pass
	struct BiquadAp : Biquad
	{
		void Setup			( CalcT normFreq, CalcT q );
		void SetupFast		( CalcT normFreq, CalcT q );
	protected:
		void SetupCommon	( CalcT sn, CalcT cs, CalcT q );
	};

	//--------------------------------------------------------------------------

	// Low shelf
	struct BiquadLs : Biquad
	{
		void Setup			( CalcT normFreq, CalcT dB, CalcT shelfSlope=1.0 );
		void SetupFast		( CalcT normFreq, CalcT dB, CalcT shelfSlope=1.0 );
	protected:
		void SetupCommon	( CalcT cs, CalcT A, CalcT sa );
	};

	//--------------------------------------------------------------------------

	// High shelf
	struct BiquadHs : Biquad
	{
		void Setup			( CalcT normFreq, CalcT dB, CalcT shelfSlope=1.0 );
		void SetupFast		( CalcT normFreq, CalcT dB, CalcT shelfSlope=1.0 );
	protected:
		void SetupCommon	( CalcT cs, CalcT A, CalcT sa );
	};

	//--------------------------------------------------------------------------

	// Peak/notch
	struct BiquadEq : Biquad
	{
		void Setup			( CalcT normFreq, CalcT dB, CalcT bandWidth );
		void SetupFast		( CalcT normFreq, CalcT dB, CalcT bandWidth );
	protected:
		void SetupCommon	( CalcT sn, CalcT cs, CalcT alph, CalcT A );
	};

	//--------------------------------------------------------------------------

	// Biquad filter.
	template<int channels>
	struct BiquadFilter : CascadeFilter
	{
		BiquadFilter()
		{
			m_nchan=channels;
			m_histp=m_hist;
			memset( m_hist, 0, sizeof(m_hist) );
		}

	private:
		Hist m_hist[channels];
	};

	//--------------------------------------------------------------------------

	template<int channels>
	struct BiquadLowPass : BiquadLp, BiquadFilter<channels>
	{
	};

	template<int channels>
	struct BiquadHighPass : BiquadHp, BiquadFilter<channels>
	{
	};

	template<int channels>
	struct BiquadBandPass1 : BiquadBp1, BiquadFilter<channels>
	{
	};

	template<int channels>
	struct BiquadBandPass2 : BiquadBp2, BiquadFilter<channels>
	{
	};

	template<int channels>
	struct BiquadBandStop : BiquadBs, BiquadFilter<channels>
	{
	};

	template<int channels>
	struct BiquadAllPass : BiquadAp, BiquadFilter<channels>
	{
	};

	template<int channels>
	struct BiquadLowShelf : BiquadLs, BiquadFilter<channels>
	{
	};

	template<int channels>
	struct BiquadHighShelf: BiquadHs, BiquadFilter<channels>
	{
	};

	template<int channels>
	struct BiquadPeak: BiquadEq, BiquadFilter<channels>
	{
	};

	//--------------------------------------------------------------------------
	//
	// Pole Filters
	//
	//--------------------------------------------------------------------------

	extern const Complex infinity;

	//--------------------------------------------------------------------------

	// Representation of a filter as a set of poles and
	// zeroes, corresponding to complex-valued roots of
	// a rational transfer function. Storage is provided
	// by a derived class.
	struct Layout
	{
		// Return the number of available poles.
		int CountPoles( void ) const
		{
			return m_pole->GetCount();
		}

		// Return the number of available zeros.
		int CountZeros( void ) const
		{
			return m_zero->GetCount();
		}

		// Set the number of available poles up to max.
		void SetPoles( int n )
		{
			m_pole->SetCount( n );
		}

		// Set the number of available zeros up to max.
		void SetZeros( int n )
		{
			m_zero->SetCount( n );
		}

		// Retrieve the zero-based i-th pole.
		Complex &Pole( int i )
		{
			return m_pole->GetNth(i);
		}

		// Retrieve the zero-based i-th zero.
		Complex &Zero( int i )
		{
			return m_zero->GetNth(i);
		}

		// Direct access to the set of all poles.
		Roots &Poles( void )
		{
			return *m_pole;
		}

		const Roots &Poles( void ) const
		{
			return *m_pole;
		}

		// Direct access to the set of all zeros.
		Roots &Zeros( void )
		{
			return *m_zero;
		}

		const Roots &Zeros( void ) const
		{
			return *m_zero;
		}

		// Access normalization parameters.
		Normalization &GetNormalization( void )
		{
			return m_normal;
		}

		const Normalization &GetNormalization( void ) const
		{
			return m_normal;
		}

		// Build a Cascade from poles and zeroes.
		void Realize( Cascade *cascade );
		void Realize_orig( Cascade *cascade );
		void Realize_custom( Cascade *cascade );
		void pairRoots();

	private:
		// Helpers for Realize().
		void BuildA( Cascade *cascade, CalcT x1, CalcT x2, int *na );
		void BuildB( Cascade *cascade, CalcT x0, CalcT x1, CalcT x2, int *nb );

	protected:
		Roots *m_pole;	// The pole roots.
		Roots *m_zero;	// The zero roots.
		Normalization m_normal;
	};

	//--------------------------------------------------------------------------

	// Storage for a Layout.
	template<int maxdegree>
	struct LayoutSpace : virtual Layout
	{
		LayoutSpace()
		{
			m_pole=&m_poles;
			m_zero=&m_zeros;
		};

	private:
		RootsSpace<maxdegree> m_poles;
		RootsSpace<maxdegree> m_zeros;
	};

	//--------------------------------------------------------------------------

	// An abstract analog to digital transformation. This converts the
	// layout of the analog prototype into a digital layout based on
	// the type of transformation (low pass, high pass, band pass, band stop).
	struct Transformation
	{
	};

	// Low pass to low pass.
	struct LowPass : Transformation
	{
		void Transform( const Spec &spec, Layout *result, const Layout &layout );
	protected:
		void Transform( const Spec &spec, Roots *result, const Roots &roots );
	};

	// Low pass to high pass.
	struct HighPass : Transformation
	{
		void Transform( const Spec &spec, Layout *result, const Layout &layout );
	protected:
		void Transform( const Spec &spec, Roots *result, const Roots &roots );
	};

	// Low pass to band pass.
	// The number of poles and zeroes is doubled.
	struct BandPass : Transformation
	{
		void Transform( const Spec &spec, Layout *result, const Layout &layout );
	protected:
		void Transform( const Spec &spec, Roots *result, const Roots &roots );
		Complex BandPassTransform( int i, const Complex &c );
		CalcT m_wc;
		CalcT m_wc2;
	};

	// Low pass to band stop.
	// The number of poles and zeroes is doubled.
	struct BandStop : Transformation
	{
		void Transform( const Spec &spec, Layout *result, const Layout &layout );
		void Transform( const Spec &spec, Roots *result, const Roots &roots );
		void DesignZeros( const Spec &spec, Layout *layout );
	protected:
		Complex BandStopTransform( int i, const Complex &c );
		CalcT m_wc;
		CalcT m_wc2;
	};

	//--------------------------------------------------------------------------

	// Abstract analog filter prototype. The filter is designed with fixed
	// specifications and then transformed to the desired response.
	// The layout is cached for fast parameter changes.
	struct Prototype : virtual Layout
	{
	};

	//--------------------------------------------------------------------------

	// Abstract digital pole filter base
	struct PoleFilter : CascadeFilter, virtual Layout
	{
		virtual void Setup( const Spec &spec )=0;
	};

	//--------------------------------------------------------------------------

	// Component aggregate for a cascade filter that provides storage
	// for coefficients, history buffer, and processing capabilities.
	template<class Proto, class Trans, int maxorder, int channels>
	struct PoleFilterSpace : PoleFilter, LayoutSpace<maxorder>, CascadeSpace<maxorder>
	{
		PoleFilterSpace()
		{
			m_nchan=channels;
			m_histp=m_hist;
			memset( m_hist, 0, sizeof(m_hist) );
		}

		void Setup( const Spec &spec ) override
		{
			m_proto.Design( spec );
			m_trans.Transform( spec, this, m_proto );
			Realize( this );
		}

	private:
		template<class Base, int maxorder1>
		struct PrototypeSpace : Base, LayoutSpace<maxorder1>
		{
		};

		PrototypeSpace<Proto, maxorder> m_proto;
		Trans m_trans;
		Hist m_hist[channels*((maxorder+1)/2)];
	};

	//--------------------------------------------------------------------------
	//
	// Butterworth
	//
	//--------------------------------------------------------------------------

	// Low pass prototype
	struct Butter : Prototype
	{
		void Design( const Spec &spec );
	};

	// Low shelf prototype
	struct ButterShelf : Prototype
	{
		void Design( const Spec &spec );
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct ButterLowPass : PoleFilterSpace<Butter, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			PoleFilterSpace<Butter, LowPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ButterHighPass : PoleFilterSpace<Butter, HighPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			PoleFilterSpace<Butter, HighPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ButterBandPass : PoleFilterSpace<Butter, BandPass, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			PoleFilterSpace<Butter, BandPass, 2*order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ButterBandStop : PoleFilterSpace<Butter, BandStop, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			PoleFilterSpace<Butter, BandStop, 2*order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ButterLowShelf : PoleFilterSpace<ButterShelf, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT gainDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.gainDb=gainDb;
			PoleFilterSpace<ButterShelf, LowPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ButterHighShelf : PoleFilterSpace<ButterShelf, HighPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT gainDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.gainDb=gainDb;
			PoleFilterSpace<ButterShelf, HighPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ButterEq : PoleFilterSpace<ButterShelf, BandPass, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT gainDb )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.gainDb=gainDb;
			PoleFilterSpace<ButterShelf, BandPass, 2*order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------
	//
	// Chebyshev Type I
	//
	//--------------------------------------------------------------------------

	// Low pass prototype
	struct ChebyI : Prototype
	{
		void Design( const Spec &spec );
	};

	// Low shelf prototype
	struct ChebyIShelf : Prototype
	{
		void Design( const Spec &spec );
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct ChebyILowPass : PoleFilterSpace<ChebyI, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			PoleFilterSpace<ChebyI, LowPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIHighPass : PoleFilterSpace<ChebyI, HighPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			PoleFilterSpace<ChebyI, HighPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIBandPass : PoleFilterSpace<ChebyI, BandPass, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			PoleFilterSpace<ChebyI, BandPass, 2*order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIBandStop : PoleFilterSpace<ChebyI, BandStop, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			PoleFilterSpace<ChebyI, BandStop, 2*order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyILowShelf : PoleFilterSpace<ChebyIShelf, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT gainDb, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			spec.gainDb=gainDb;
			PoleFilterSpace<ChebyIShelf, LowPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIHighShelf : PoleFilterSpace<ChebyIShelf, HighPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT gainDb, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			spec.gainDb=gainDb;
			PoleFilterSpace<ChebyIShelf, HighPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIEq : PoleFilterSpace<ChebyIShelf, BandPass, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT gainDb, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			spec.gainDb=gainDb;
			PoleFilterSpace<ChebyIShelf, BandPass, 2*order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------
	//
	// Chebyshev Type II
	//
	//--------------------------------------------------------------------------

	// Low pass prototype
	struct ChebyII : Prototype
	{
		void Design( const Spec &spec );
	};

	// Low shelf prototype
	struct ChebyIIShelf : Prototype
	{
		void Design( const Spec &spec );
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct ChebyIILowPass : PoleFilterSpace<ChebyII, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT stopBandDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.stopBandDb=stopBandDb;
			PoleFilterSpace<ChebyII, LowPass, order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct ChebyIIHighPass : PoleFilterSpace<ChebyII, HighPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT stopBandDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.stopBandDb=stopBandDb;
			PoleFilterSpace<ChebyII, HighPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIIBandPass : PoleFilterSpace<ChebyII, BandPass, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT stopBandDb )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.stopBandDb=stopBandDb;
			PoleFilterSpace<ChebyII, BandPass, 2*order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIIBandStop : PoleFilterSpace<ChebyII, BandStop, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT stopBandDb )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.stopBandDb=stopBandDb;
			PoleFilterSpace<ChebyII, BandStop, 2*order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIILowShelf : PoleFilterSpace<ChebyIIShelf, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT gainDb, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			spec.gainDb=gainDb;
			PoleFilterSpace<ChebyIIShelf, LowPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIIHighShelf : PoleFilterSpace<ChebyIIShelf, HighPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT gainDb, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			spec.gainDb=gainDb;
			PoleFilterSpace<ChebyIIShelf, HighPass, order, channels>::Setup( spec );
		}
	};

	template<int order, int channels>
	struct ChebyIIEq : PoleFilterSpace<ChebyIIShelf, BandPass, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT gainDb, CalcT rippleDb )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.passRippleDb=rippleDb;
			spec.gainDb=gainDb;
			PoleFilterSpace<ChebyIIShelf, BandPass, 2*order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------
	//
	// Elliptic
	//
	//--------------------------------------------------------------------------

	// Low pass prototype
	struct Elliptic : Prototype
	{
		void Design( const Spec &spec );

	protected:
		void	prodpoly		( int sn );
		void	calcfz2			( int i );
		void	calcfz			( void );
		void	calcqz			( void );
		CalcT	findfact		( int t );
		CalcT	calcsn			( CalcT u );
		CalcT	ellipticK		( CalcT k );

	protected:
		template<int n>
		struct CalcArray
		{
			CalcT &operator[](size_t index)
			{
				assert( index>=0 && index<n );
				return m_a[index];
			}
		private:
			CalcT m_a[n];
		};

		CalcT m_p0;
		CalcT m_q;
		CalcT m_K;
		CalcT m_Kprime;
		CalcArray<100> m_zeros;
		CalcArray<100> m_c1;
		CalcArray<100> m_b1;
		CalcArray<100> m_a1;
		CalcArray<100> m_d1;
		CalcArray<100> m_q1;
		CalcArray<100> m_z1;
		CalcArray<100> m_f1;
		CalcArray<100> m_s1;
		CalcArray<100> m_p ;
		CalcArray<100> m_zw1;
		CalcArray<100> m_zf1;
		CalcArray<100> m_zq1;
		CalcArray<100> m_rootR;
		CalcArray<100> m_rootI;
		CalcT m_e;
		int m_nin;
		int m_m;
		int m_n2;
		int m_em;
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct EllipticLowPass : PoleFilterSpace<Elliptic, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT passRippleDb, CalcT rollOff )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.passRippleDb=passRippleDb;
			spec.rollOff=rollOff;
			PoleFilterSpace<Elliptic, LowPass, order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct EllipticHighPass : PoleFilterSpace<Elliptic, HighPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT passRippleDb, CalcT rollOff )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.passRippleDb=passRippleDb;
			spec.rollOff=rollOff;
			PoleFilterSpace<Elliptic, HighPass, order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct EllipticBandPass : PoleFilterSpace<Elliptic, BandPass, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT passRippleDb, CalcT rollOff )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.passRippleDb=passRippleDb;
			spec.rollOff=rollOff;
			PoleFilterSpace<Elliptic, BandPass, 2*order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct EllipticBandStop : PoleFilterSpace<Elliptic, BandStop, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth, CalcT passRippleDb, CalcT rollOff )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			spec.passRippleDb=passRippleDb;
			spec.rollOff=rollOff;
			PoleFilterSpace<Elliptic, BandStop, 2*order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------
	//
	// Bessel
	//
	//--------------------------------------------------------------------------

	// Low pass prototype
	struct Bessel : Prototype
	{
		void Design( const Spec &spec );
	};

	// Low shelf prototype
	struct BesselShelf : Prototype
	{
		void Design( const Spec &spec );
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct BesselLowPass : PoleFilterSpace<Bessel, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			PoleFilterSpace<Bessel, LowPass, order, channels>::Setup( spec );
		}
	};
	
	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct BesselHighPass : PoleFilterSpace<Bessel, HighPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			PoleFilterSpace<Bessel, HighPass, order, channels>::Setup( spec );
		}
	};
	
	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct BesselBandPass : PoleFilterSpace<Bessel, BandPass, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			PoleFilterSpace<Bessel, BandPass, 2*order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct BesselBandStop : PoleFilterSpace<Bessel, BandStop, 2*order, channels>
	{
		void SetupAs( CalcT centerFreq, CalcT normWidth )
		{
			Spec spec;
			spec.order=order;
			spec.centerFreq=centerFreq;
			spec.normWidth=normWidth;
			spec.sampleRate=1;
			PoleFilterSpace<Bessel, BandStop, 2*order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct BesselLowShelf : PoleFilterSpace<BesselShelf, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq, CalcT gainDb )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			spec.gainDb=gainDb;
			PoleFilterSpace<BesselShelf, LowPass, order, channels>::Setup( spec );
		}
	};

	//--------------------------------------------------------------------------
	//
	// Legendere
	//
	//--------------------------------------------------------------------------

	// Low pass prototype
	struct Legendere : Prototype
	{
		void Design( const Spec &spec );
	};

	//--------------------------------------------------------------------------

	template<int order, int channels>
	struct LegendereLowPass : PoleFilterSpace<Legendere, LowPass, order, channels>
	{
		void SetupAs( CalcT cutoffFreq )
		{
			Spec spec;
			spec.order=order;
			spec.cutoffFreq=cutoffFreq;
			spec.sampleRate=1;
			PoleFilterSpace<Legendere, LowPass, order, channels>::Setup( spec );
		}
	};
	
	//--------------------------------------------------------------------------
};

#endif
