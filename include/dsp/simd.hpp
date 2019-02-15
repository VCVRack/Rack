#include "common.hpp"
#include "sse_mathfun.h"
#include <emmintrin.h>


namespace rack {
namespace dsp {


inline float cast_i32_f32(int i) {
	float f;
	std::memcpy(&f, &i, sizeof(f));
	return f;
}


inline int cast_f32_i32(float f) {
	float i;
	std::memcpy(&i, &f, sizeof(i));
	return i;
}


template <int N>
struct f32;


/** Wrapper for `__m128` representing an aligned vector of 4 single-precision float values.
*/
template <>
struct f32<4> {
	__m128 v;

	/** Constructs an uninitialized vector. */
	f32<4>() {}

	/** Constructs a vector from a native `__m128` type. */
	f32<4>(__m128 v) : v(v) {}

	/** Constructs a vector with all elements set to `x`. */
	f32<4>(float x) {
		v = _mm_set_ps1(x);
	}

	/** Reads an array of 4 values. */
	static f32<4> load(const float *x) {
		return f32<4>(_mm_loadu_ps(x));
	}

	/** Returns a vector initialized to zero. */
	static f32<4> zero() {
		return f32<4>(_mm_setzero_ps());
	}

	/** Writes an array of 4 values. */
	void store(float *x) {
		_mm_storeu_ps(x, v);
	}
};


typedef f32<4> f32_4;


// Operator overloads


/** `a operator b` */
#define DECLARE_F32_4_OPERATOR_INFIX(operator, func) \
	inline f32_4 operator(const f32_4 &a, const f32_4 &b) { \
		return f32_4(func(a.v, b.v)); \
	} \
	template <typename T> \
	f32_4 operator(const T &a, const f32_4 &b) { \
		return operator(f32_4(a), b); \
	} \
	template <typename T> \
	f32_4 operator(const f32_4 &a, const T &b) { \
		return operator(a, f32_4(b)); \
	}

/** `a operator b` */
#define DECLARE_F32_4_OPERATOR_INCREMENT(operator, opfunc) \
	inline f32_4 &operator(f32_4 &a, const f32_4 &b) { \
		a = opfunc(a, b); \
		return a; \
	} \
	template <typename T> \
	f32_4 &operator(f32_4 &a, const T &b) { \
		return operator(a, f32_4(b)); \
	}

DECLARE_F32_4_OPERATOR_INFIX(operator+, _mm_add_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator-, _mm_sub_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator*, _mm_mul_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator/, _mm_div_ps)

/** Boolean operators on vectors give 0x00000000 for false and 0xffffffff for true, for each vector element.
Use these to apply logic, bit masks, and conditions to elements.
Examples:

Subtract 1 from value if greater than or equal to 1.
	x -= (x >= 1.f) & 1.f;
*/
DECLARE_F32_4_OPERATOR_INFIX(operator^, _mm_xor_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator&, _mm_and_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator|, _mm_mul_ps)

DECLARE_F32_4_OPERATOR_INCREMENT(operator+=, operator+);
DECLARE_F32_4_OPERATOR_INCREMENT(operator-=, operator-);
DECLARE_F32_4_OPERATOR_INCREMENT(operator*=, operator*);
DECLARE_F32_4_OPERATOR_INCREMENT(operator/=, operator/);
DECLARE_F32_4_OPERATOR_INCREMENT(operator^=, operator^);
DECLARE_F32_4_OPERATOR_INCREMENT(operator&=, operator&);
DECLARE_F32_4_OPERATOR_INCREMENT(operator|=, operator|);

/** `+a` */
inline f32_4 operator+(const f32_4 &a) {
	return a;
}

/** `-a` */
inline f32_4 operator-(const f32_4 &a) {
	return 0.f - a;
}

/** `++a` */
inline f32_4 &operator++(f32_4 &a) {
	a += 1.f;
	return a;
}

/** `--a` */
inline f32_4 &operator--(f32_4 &a) {
	a -= 1.f;
	return a;
}

/** `a++` */
inline f32_4 operator++(f32_4 &a, int) {
	f32_4 b = a;
	++a;
	return b;
}

/** `a--` */
inline f32_4 operator--(f32_4 &a, int) {
	f32_4 b = a;
	--a;
	return b;
}

/** `~a` */
inline f32_4 operator~(const f32_4 &a) {
	return f32_4(_mm_xor_ps(a.v, _mm_cmpeq_ps(a.v, a.v)));
}

DECLARE_F32_4_OPERATOR_INFIX(operator==, _mm_cmpeq_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator>=, _mm_cmpge_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator>, _mm_cmpgt_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator<=, _mm_cmple_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator<, _mm_cmplt_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator!=, _mm_cmpneq_ps)


// Math functions


inline f32_4 fmax(f32_4 x, f32_4 b) {
	return f32_4(_mm_max_ps(x.v, b.v));
}

inline f32_4 fmin(f32_4 x, f32_4 b) {
	return f32_4(_mm_min_ps(x.v, b.v));
}

inline f32_4 sqrt(f32_4 x) {
	return f32_4(_mm_sqrt_ps(x.v));
}

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

inline f32_4 log(f32_4 x) {
	return f32_4(sse_mathfun_log_ps(x.v));
}

inline f32_4 exp(f32_4 x) {
	return f32_4(sse_mathfun_exp_ps(x.v));
}

inline f32_4 sin(f32_4 x) {
	return f32_4(sse_mathfun_sin_ps(x.v));
}

inline f32_4 cos(f32_4 x) {
	return f32_4(sse_mathfun_cos_ps(x.v));
}

inline f32_4 floor(f32_4 a) {
	return f32_4(sse_mathfun_floor_ps(a.v));
}

inline f32_4 ceil(f32_4 a) {
	return f32_4(sse_mathfun_ceil_ps(a.v));
}

inline f32_4 round(f32_4 a) {
	return f32_4(sse_mathfun_round_ps(a.v));
}

inline f32_4 fmod(f32_4 a, f32_4 b) {
	return f32_4(sse_mathfun_fmod_ps(a.v, b.v));
}

inline f32_4 fabs(f32_4 a) {
	return f32_4(sse_mathfun_fabs_ps(a.v));
}

inline f32_4 trunc(f32_4 a) {
	return f32_4(sse_mathfun_trunc_ps(a.v));
}


} // namespace dsp
} // namespace rack
