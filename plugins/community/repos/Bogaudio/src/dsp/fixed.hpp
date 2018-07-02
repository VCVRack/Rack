#pragma once

#include <stdint.h>
#include <math.h>

namespace bogaudio {
namespace dsp {

template<typename I, const int SCALE>
struct Fixed {
	static constexpr I scale = ((I)1) << SCALE;
	static constexpr double inverseScale = 1.0 / (double)scale;
	typedef union { I v; } value_t;
	value_t _v;

	Fixed() { _v.v = 0; }
	Fixed(const value_t& o) { _v.v = o.v; }
	Fixed(const int32_t& o) { _v.v = o * scale; }
	Fixed(const int64_t& o) { _v.v = o * scale; }
	Fixed(const double& o) { _v.v = round(o * scale); }
	Fixed(const float& o) { _v.v = roundf(o * scale); }

	inline operator int32_t() const { return _v.v / scale; }
	inline operator int64_t() const { return _v.v / scale; }
	inline operator double() const { return inverseScale * (double)_v.v; }
	inline operator float() const { return inverseScale * (float)_v.v; }

	inline Fixed& operator+=(const Fixed& o) { _v.v += o._v.v; return *this; }
	inline Fixed operator+(const Fixed& o) { return Fixed(*this) += o; }
	inline Fixed& operator-=(const Fixed& o) { _v.v -= o._v.v; return *this; }
	inline Fixed operator-(const Fixed& o) { return Fixed(*this) -= o; }

	inline Fixed& operator+=(double o) { return *this += Fixed(o); }
	inline Fixed operator+(double o) { return Fixed(*this) += o; }
	inline Fixed& operator-=(double o) { return *this -= Fixed(o); }
	inline Fixed operator-(double o) { return Fixed(*this) -= o; }

	inline Fixed& operator+=(float o) { return *this += Fixed(o); }
	inline Fixed operator+(float o) { return Fixed(*this) += o; }
	inline Fixed& operator-=(float o) { return *this -= Fixed(o); }
	inline Fixed operator-(float o) { return Fixed(*this) -= o; }

	inline Fixed& operator+=(int o) { return *this += Fixed(o); }
	inline Fixed operator+(int o) { return Fixed(*this) += o; }
	inline Fixed& operator-=(int o) { return *this -= Fixed(o); }
	inline Fixed operator-(int o) { return Fixed(*this) -= o; }
};

typedef Fixed<int32_t, 16> fixed_16_16;
typedef Fixed<int64_t, 32> fixed_32_32;

} // namespace dsp
} // namespace bogaudio
