#pragma once
#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/*
In this header, function names are divided into two or more parts, separated by "_".
The functionality is the first part, and the approximation methods are the following parts.

Glossary:
https://en.wikipedia.org/wiki/Taylor_series
https://en.wikipedia.org/wiki/Chebyshev_polynomials
https://en.wikipedia.org/wiki/Pad%C3%A9_approximant
https://en.wikipedia.org/wiki/Horner%27s_method
https://en.wikipedia.org/wiki/Estrin%27s_scheme
https://en.wikipedia.org/wiki/CORDIC
*/


/** Returns 2^floor(x), assuming that x >= 0.
If `xf` is non-NULL, it is set to the fractional part of x.
*/
template <typename T>
T approxExp2Floor(T x, T* xf);

template <>
inline simd::float_4 approxExp2Floor(simd::float_4 x, simd::float_4* xf) {
	simd::int32_4 xi = x;
	if (xf)
		*xf = x - simd::float_4(xi);
	// Set float exponent directly
	// https://stackoverflow.com/a/57454528/272642
	simd::int32_4 y = (xi + 127) << 23;
	return simd::float_4::cast(y);
}

template <>
inline float approxExp2Floor(float x, float* xf) {
	int32_t xi = x;
	if (xf)
		*xf = x - xi;
	int32_t y = (xi + 127) << 23;
	return bitCast<float>(y);
}


/** Returns 2^x, assuming that x >= 0.
Maximum 0.00024% error.
For float, roughly 3x faster than `std::pow(2.f, x)`.
For float_4, roughly 2x faster than `simd::pow(2.f, x)`.

If negative powers are needed, you may use a lower bound and rescale.

	approxExp2(x + 20) / 1048576
*/
template <typename T>
T approxExp2_taylor5(T x) {
	// Use bit-shifting for integer part of x.
	T y = approxExp2Floor(x, &x);
	// 5th order expansion of 2^x around 0.4752 in Horner form.
	// The center is chosen so that the endpoints of [0, 1] have equal error, creating no discontinuity at integers.
	y *= T(0.9999976457798443) + x * (T(0.6931766804601935) + x * (T(0.2400729486415728) + x * (T(0.05592817518644387) + x * (T(0.008966320633544) + x * T(0.001853512473884202)))));
	return y;
}


} // namespace dsp
} // namespace rack
