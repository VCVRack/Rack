#pragma once
#include <simd/vector.hpp>
#include <simd/sse_mathfun_extension.h>
#include <common.hpp>
#include <math.hpp>


namespace rack {
namespace simd {


// Nonstandard functions

inline float ifelse(bool cond, float a, float b) {
	return cond ? a : b;
}

/** Given a mask, returns a if mask is 0xffffffff per element, b if mask is 0x00000000 */
inline float_4 ifelse(float_4 mask, float_4 a, float_4 b) {
	return (a & mask) | andnot(mask, b);
}

/** Returns the approximate reciprocal square root.
Much faster than `1/sqrt(x)`.
*/
inline float_4 rsqrt(float_4 x) {
	return float_4(_mm_rsqrt_ps(x.v));
}

/** Returns the approximate reciprocal.
Much faster than `1/x`.
*/
inline float_4 rcp(float_4 x) {
	return float_4(_mm_rcp_ps(x.v));
}

/** Given a mask `a`, returns a vector with each element either 0's or 1's depending on the mask bit.
*/
template <typename T>
T movemaskInverse(int a);

template <>
inline float_4 movemaskInverse<float_4>(int x) {
	__m128i msk8421 = _mm_set_epi32(8, 4, 2, 1);
	__m128i x_bc = _mm_set1_epi32(x);
	__m128i t = _mm_and_si128(x_bc, msk8421);
	return float_4(_mm_castsi128_ps(_mm_cmpeq_epi32(x_bc, t)));
}


// Standard math functions from std::

/* Import std:: math functions into the simd namespace so you can use `sin(T)` etc in templated functions and get both the scalar and vector versions.

Example:

	template <typename T>
	T sin_plus_cos(T x) {
		return simd::sin(x) + simd::cos(x);
	}
*/

using std::fmax;

inline float_4 fmax(float_4 x, float_4 b) {
	return float_4(_mm_max_ps(x.v, b.v));
}

using std::fmin;

inline float_4 fmin(float_4 x, float_4 b) {
	return float_4(_mm_min_ps(x.v, b.v));
}

using std::sqrt;

inline float_4 sqrt(float_4 x) {
	return float_4(_mm_sqrt_ps(x.v));
}

using std::log;

inline float_4 log(float_4 x) {
	return float_4(sse_mathfun_log_ps(x.v));
}

using std::log10;

inline float_4 log10(float_4 x) {
	return float_4(sse_mathfun_log_ps(x.v)) / std::log(10.f);
}

using std::log2;

inline float_4 log2(float_4 x) {
	return float_4(sse_mathfun_log_ps(x.v)) / std::log(2.f);
}

using std::exp;

inline float_4 exp(float_4 x) {
	return float_4(sse_mathfun_exp_ps(x.v));
}

using std::sin;

inline float_4 sin(float_4 x) {
	return float_4(sse_mathfun_sin_ps(x.v));
}

using std::cos;

inline float_4 cos(float_4 x) {
	return float_4(sse_mathfun_cos_ps(x.v));
}

using std::tan;

inline float_4 tan(float_4 x) {
	return float_4(sse_mathfun_tan_ps(x.v));
}

using std::atan;

inline float_4 atan(float_4 x) {
	return float_4(sse_mathfun_atan_ps(x.v));
}

using std::atan2;

inline float_4 atan2(float_4 x, float_4 y) {
	return float_4(sse_mathfun_atan2_ps(x.v, y.v));
}

using std::trunc;

inline float_4 trunc(float_4 a) {
	return float_4(_mm_cvtepi32_ps(_mm_cvttps_epi32(a.v)));
}

using std::floor;

inline float_4 floor(float_4 a) {
	float_4 b = trunc(a);
	b -= (b > a) & 1.f;
	return b;
}

using std::ceil;

inline float_4 ceil(float_4 a) {
	float_4 b = trunc(a);
	b += (b < a) & 1.f;
	return b;
}

using std::round;

inline float_4 round(float_4 a) {
	a += ifelse(a < 0, -0.5f, 0.5f);
	float_4 b = trunc(a);
	return b;
}

using std::fmod;

inline float_4 fmod(float_4 a, float_4 b) {
	return a - trunc(a / b) * b;
}

using std::fabs;

inline float_4 fabs(float_4 a) {
	// Sign bit
	int32_4 mask = ~0x80000000;
	return a & float_4::cast(mask);
}

using std::pow;

inline float_4 pow(float_4 a, float_4 b) {
	return exp(b * log(a));
}

inline float_4 pow(float a, float_4 b) {
	return exp(b * std::log(a));
}

template <typename T>
T pow(T a, int b) {
	// Optimal with `-O3 -funsafe-math-optimizations` when b is known at compile-time
	T p = 1;
	for (int i = 1; i <= b; i *= 2) {
		if (i & b)
			p *= a;
		a *= a;
	}
	return p;
}

// From math.hpp

using math::clamp;

inline float_4 clamp(float_4 x, float_4 a, float_4 b) {
	return fmin(fmax(x, a), b);
}

using math::rescale;

inline float_4 rescale(float_4 x, float_4 xMin, float_4 xMax, float_4 yMin, float_4 yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}

using math::crossfade;

inline float_4 crossfade(float_4 a, float_4 b, float_4 p) {
	return a + (b - a) * p;
}

using math::sgn;

inline float_4 sgn(float_4 x) {
	float_4 signbit = x & -0.f;
	float_4 nonzero = (x != 0.f);
	return signbit | (nonzero & 1.f);
}


} // namespace simd
} // namespace rack
