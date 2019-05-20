#pragma once
#include "vector.hpp"
#include "sse_mathfun.h"
#include "math.hpp"
#include <cmath>


namespace rack {
namespace simd {


// Standard math functions from std::

/* Import std:: math functions into the simd namespace so you can use `sin(T)` etc in templated functions and get both the scalar and vector versions.

Example:

	template <typename T>
	T sin_plus_cos(T x) {
		using namespace simd;
		return sin(x) + cos(x);
	}
*/

using std::fmax;

inline float4 fmax(float4 x, float4 b) {
	return float4(_mm_max_ps(x.v, b.v));
}

using std::fmin;

inline float4 fmin(float4 x, float4 b) {
	return float4(_mm_min_ps(x.v, b.v));
}

using std::sqrt;

inline float4 sqrt(float4 x) {
	return float4(_mm_sqrt_ps(x.v));
}

using std::log;

inline float4 log(float4 x) {
	return float4(sse_mathfun_log_ps(x.v));
}

using std::log10;

inline float4 log10(float4 x) {
	return float4(sse_mathfun_log_ps(x.v)) / std::log(10.f);
}

using std::log2;

inline float4 log2(float4 x) {
	return float4(sse_mathfun_log_ps(x.v)) / std::log(2.f);
}

using std::exp;

inline float4 exp(float4 x) {
	return float4(sse_mathfun_exp_ps(x.v));
}

using std::sin;

inline float4 sin(float4 x) {
	return float4(sse_mathfun_sin_ps(x.v));
}

using std::cos;

inline float4 cos(float4 x) {
	return float4(sse_mathfun_cos_ps(x.v));
}

using std::floor;

inline float4 floor(float4 a) {
	return float4(sse_mathfun_floor_ps(a.v));
}

using std::ceil;

inline float4 ceil(float4 a) {
	return float4(sse_mathfun_ceil_ps(a.v));
}

using std::round;

inline float4 round(float4 a) {
	return float4(sse_mathfun_round_ps(a.v));
}

using std::fmod;

inline float4 fmod(float4 a, float4 b) {
	return float4(sse_mathfun_fmod_ps(a.v, b.v));
}

using std::fabs;

inline float4 fabs(float4 a) {
	return float4(sse_mathfun_fabs_ps(a.v));
}

using std::trunc;

inline float4 trunc(float4 a) {
	return float4(sse_mathfun_trunc_ps(a.v));
}

using std::pow;

inline float4 pow(float4 a, float4 b) {
	return exp(b * log(a));
}

inline float4 pow(float a, float4 b) {
	return exp(b * std::log(a));
}

// Nonstandard functions

inline float ifelse(bool cond, float a, float b) {
	return cond ? a : b;
}

/** Given a mask, returns a if mask is 0xffffffff per element, b if mask is 0x00000000 */
inline float4 ifelse(float4 mask, float4 a, float4 b) {
	return (a & mask) | andnot(mask, b);
}

/** Returns the approximate reciprocal square root.
Much faster than `1/sqrt(x)`.
*/
inline float4 rsqrt(float4 x) {
	return float4(_mm_rsqrt_ps(x.v));
}

/** Returns the approximate reciprocal.
Much faster than `1/x`.
*/
inline float4 rcp(float4 x) {
	return float4(_mm_rcp_ps(x.v));
}

// From math.hpp

using math::clamp;

inline float4 clamp(float4 x, float4 a, float4 b) {
	return fmin(fmax(x, a), b);
}

using math::rescale;

inline float4 rescale(float4 x, float4 xMin, float4 xMax, float4 yMin, float4 yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}

using math::sgn;

inline float4 sgn(float4 x) {
	float4 signbit = x & -0.f;
	float4 nonzero = (x != 0.f);
	return signbit | (nonzero & 1.f);
}


} // namespace simd
} // namespace rack
