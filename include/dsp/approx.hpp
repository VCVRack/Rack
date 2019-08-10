#pragma once
#include <dsp/common.hpp>


namespace rack {
namespace dsp {


/*
In this header, function names are divided into two or more parts, separated by "_".
The functionality is the first part, and the approximation methods are the following parts.

Glossary:
https://en.wikipedia.org/wiki/Taylor_series
https://en.wikipedia.org/wiki/Pad%C3%A9_approximant
https://en.wikipedia.org/wiki/Horner%27s_method
https://en.wikipedia.org/wiki/Estrin%27s_scheme
*/


/** Returns 2^x, assuming that x >= 0.
Maximum 0.00024% error.
Roughly 7x faster than `std::pow(2, x)`.

If negative powers are needed, you may use a lower bound and rescale.

	approxExp2(x + 20) / 1048576
*/
template <typename T>
T approxExp2_taylor5(T x) {
	// Use bit-shifting for integer part of x.
	int xi = x;
	x -= xi;
	T y = 1 << xi;
	// 5th order expansion of 2^x around 0.4752 in Horner form.
	// The center is chosen so that the endpoints of [0, 1] have equal error, creating no discontinuity at integers.
	y *= T(0.9999976457798443) + x * (T(0.6931766804601935) + x * (T(0.2400729486415728) + x * (T(0.05592817518644387) + x * (T(0.008966320633544) + x * T(0.001853512473884202)))));
	return y;
}


} // namespace dsp
} // namespace rack
