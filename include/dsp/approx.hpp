#pragma once
#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/*
Glossary:
https://en.wikipedia.org/wiki/Taylor_series
https://en.wikipedia.org/wiki/Chebyshev_polynomials
https://en.wikipedia.org/wiki/Pad%C3%A9_approximant
https://en.wikipedia.org/wiki/Horner%27s_method
https://en.wikipedia.org/wiki/Estrin%27s_scheme
https://en.wikipedia.org/wiki/CORDIC
*/


/** Evaluates a polynomial with coefficients `a[n]` at `x`.
Uses naive direct evaluation.
*/
template <typename T, size_t N>
T polyDirect(const T (&a)[N], T x) {
	T y = 0;
	T xn = 1;
	for (size_t n = 0; n < N; n++) {
		y += a[n] * xn;
		xn *= x;
	}
	return y;
}

/** Evaluates a polynomial with coefficients `a[n]` at `x`.
Uses Horner's method.
https://en.wikipedia.org/wiki/Horner%27s_method
*/
template <typename T, size_t N>
T polyHorner(const T (&a)[N], T x) {
	if (N == 0)
		return 0;

	T y = a[N - 1];
	for (size_t n = 1; n < N; n++) {
		y = a[N - 1 - n] + y * x;
	}
	return y;
}

/** Evaluates a polynomial with coefficients `a[n]` at `x`.
Uses Estrin's method.
https://en.wikipedia.org/wiki/Estrin%27s_scheme
*/
template <typename T, size_t N>
T polyEstrin(const T (&a)[N], T x) {
	if (N == 0)
		return 0;
	if (N == 1)
		return a[0];

	const size_t M = (N + 1) / 2;
	T b[M];
	for (size_t i = 0; i < M; i++) {
		b[i] = a[2 * i];
		if (2 * i + 1 < N)
			b[i] += a[2 * i + 1] * x;
	}
	return polyEstrin(b, x * x);
}


/** Returns `2^floor(x)`.
If xf is given, sets it to the fractional part of x.
This is useful in the computation `2^x = 2^floor(x) * 2^frac(x)`.
*/
template <typename T>
T exp2Floor(T x, T* xf);

template <>
inline float exp2Floor(float x, float* xf) {
	x += 127;
	// x should be positive now, so this always truncates towards -inf.
	int32_t xi = x;
	if (xf)
		*xf = x - xi;
	// Set mantissa of float
	union {
		float yi;
		int32_t yii;
	};
	yii = xi << 23;
	return yi;
}

template <>
inline simd::float_4 exp2Floor(simd::float_4 x, simd::float_4* xf) {
	x += 127;
	simd::int32_4 xi = x;
	if (xf)
		*xf = x - simd::float_4(xi);
	simd::int32_4 yii = xi << 23;
	return simd::float_4::cast(yii);
}

/** Deprecated alias of exp2Floor() */
template <typename T>
T approxExp2Floor(T x, T* xf) {
	return exp2Floor(x, xf);
}


/** Returns 2^x with at most 6e-06 relative error.

Polynomial coefficients are chosen to minimize relative error while maintaining continuity and giving exact values at integer values of `x`.
Thanks to Andy Simper for coefficients.
*/
template <typename T>
T exp2_taylor5(T x) {
	T xf;
	T yi = exp2Floor(x, &xf);

	const T a[] = {
		1.0,
		0.69315169353961,
		0.2401595990753,
		0.055817908652,
		0.008991698010,
		0.001879100722,
	};
	T yf = polyHorner(a, xf);
	return yi * yf;
}

/** Deprecated alias of exp2_taylor5() */
template <typename T>
T approxExp2_taylor5(T x) {
	return exp2_taylor5(x);
}


} // namespace dsp
} // namespace rack
