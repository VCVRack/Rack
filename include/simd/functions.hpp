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

inline f32_4 fmax(f32_4 x, f32_4 b) {
	return f32_4(_mm_max_ps(x.v, b.v));
}

using std::fmin;

inline f32_4 fmin(f32_4 x, f32_4 b) {
	return f32_4(_mm_min_ps(x.v, b.v));
}

using std::sqrt;

inline f32_4 sqrt(f32_4 x) {
	return f32_4(_mm_sqrt_ps(x.v));
}

using std::log;

inline f32_4 log(f32_4 x) {
	return f32_4(sse_mathfun_log_ps(x.v));
}

using std::exp;

inline f32_4 exp(f32_4 x) {
	return f32_4(sse_mathfun_exp_ps(x.v));
}

using std::sin;

inline f32_4 sin(f32_4 x) {
	return f32_4(sse_mathfun_sin_ps(x.v));
}

using std::cos;

inline f32_4 cos(f32_4 x) {
	return f32_4(sse_mathfun_cos_ps(x.v));
}

using std::floor;

inline f32_4 floor(f32_4 a) {
	return f32_4(sse_mathfun_floor_ps(a.v));
}

using std::ceil;

inline f32_4 ceil(f32_4 a) {
	return f32_4(sse_mathfun_ceil_ps(a.v));
}

using std::round;

inline f32_4 round(f32_4 a) {
	return f32_4(sse_mathfun_round_ps(a.v));
}

using std::fmod;

inline f32_4 fmod(f32_4 a, f32_4 b) {
	return f32_4(sse_mathfun_fmod_ps(a.v, b.v));
}

using std::fabs;

inline f32_4 fabs(f32_4 a) {
	return f32_4(sse_mathfun_fabs_ps(a.v));
}

using std::trunc;

inline f32_4 trunc(f32_4 a) {
	return f32_4(sse_mathfun_trunc_ps(a.v));
}

using std::pow;

inline f32_4 pow(f32_4 a, f32_4 b) {
	return exp(b * log(a));
}

inline f32_4 pow(float a, f32_4 b) {
	return exp(b * std::log(a));
}

// Nonstandard functions

/** Returns the approximate reciprocal square root.
Much faster than `1/sqrt(x)`.
*/
inline f32_4 rsqrt(f32_4 x) {
	return f32_4(_mm_rsqrt_ps(x.v));
}

/** Returns the approximate reciprocal.
Much faster than `1/x`.
*/
inline f32_4 rcp(f32_4 x) {
	return f32_4(_mm_rcp_ps(x.v));
}

// From math.hpp

using math::clamp;

inline f32_4 clamp(f32_4 x, f32_4 a, f32_4 b) {
	return fmin(fmax(x, a), b);
}

using math::rescale;

inline f32_4 rescale(f32_4 x, f32_4 xMin, f32_4 xMax, f32_4 yMin, f32_4 yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}


} // namespace simd
} // namespace rack
