#include <emmintrin.h>
#include "sse_mathfun.h"


namespace rack {
namespace dsp {


template <int N>
struct f32;


/** Wrapper for `__m128` representing a vector of 4 single-precision float values. */
template <>
struct f32<4> {
	__m128 v;

	f32<4>() {}
	f32<4>(__m128 v) : v(v) {}
	f32<4>(float x) {
		v = _mm_set_ps1(x);
	}
	/** Reads an array of 4 values. */
	f32<4>(const float *x) {
		v = _mm_loadu_ps(x);
	}
	/** Writes an array of 4 values. */
	void store(float *x) {
		_mm_storeu_ps(x, v);
	}
};


typedef f32<4> f32_4;


// Operator overloads

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

#define DECLARE_F32_4_OPERATOR_INCREMENT(operator, func) \
	inline f32_4 &operator(f32_4 &a, const f32_4 &b) { \
		a.v = func(a.v, b.v); \
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

DECLARE_F32_4_OPERATOR_INCREMENT(operator+=, _mm_add_ps);
DECLARE_F32_4_OPERATOR_INCREMENT(operator-=, _mm_sub_ps);
DECLARE_F32_4_OPERATOR_INCREMENT(operator*=, _mm_mul_ps);
DECLARE_F32_4_OPERATOR_INCREMENT(operator/=, _mm_div_ps);

// TODO Perhaps return a future i32 type for these, or add casting between multiple simd types
DECLARE_F32_4_OPERATOR_INFIX(operator==, _mm_cmpeq_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator>=, _mm_cmpge_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator>, _mm_cmpgt_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator<=, _mm_cmple_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator<, _mm_cmplt_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator!=, _mm_cmpneq_ps)


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
	return f32_4(log_ps(x.v));
}

inline f32_4 exp(f32_4 x) {
	return f32_4(exp_ps(x.v));
}

inline f32_4 sin(f32_4 x) {
	return f32_4(sin_ps(x.v));
}

inline f32_4 cos(f32_4 x) {
	return f32_4(cos_ps(x.v));
}


} // namespace dsp
} // namespace rack
