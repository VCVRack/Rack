#include <x86intrin.h>


#define ALIGN(n) __attribute__((aligned(n)))


namespace simd {


template <int N>
struct f32;


/** Wrapper for `__m128` representing a vector of 4 single-precision float values. */
template <>
struct f32<4> {
	__m128 v;

	f32<4>() {}
	f32<4>(__m128 v) : v(v) {}
	template <typename T>
	f32<4>(T x) {
		v = _mm_set_ps1((float) x);
	}
	/** Reads an array of 4 values */
	f32<4>(const float *x) {
		v = _mm_loadu_ps(x);
	}
	/** Writes an array of 4 values */
	void store(float *x) {
		_mm_storeu_ps(x, v);
	}
};


// Operator overloads

#define DECLARE_F32_4_OPERATOR_INFIX(operator, func) \
	inline f32<4> operator(f32<4> a, f32<4> b) { \
		return f32<4>(func(a.v, b.v)); \
	} \
	template <typename T> \
	f32<4> operator(T a, f32<4> b) { \
		return operator(f32<4>(a), b); \
	} \
	template <typename T> \
	f32<4> operator(f32<4> a, T b) { \
		return operator(a, f32<4>(b)); \
	}

DECLARE_F32_4_OPERATOR_INFIX(operator+, _mm_add_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator-, _mm_sub_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator*, _mm_mul_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator/, _mm_div_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator==, _mm_cmpeq_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator>=, _mm_cmpge_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator>, _mm_cmpgt_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator<=, _mm_cmple_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator<, _mm_cmplt_ps)
DECLARE_F32_4_OPERATOR_INFIX(operator!=, _mm_cmpneq_ps)

#define DECLARE_F32_4_OPERATOR_INCREMENT(operator, func) \
	inline f32<4> &operator(f32<4> &a, f32<4> b) { \
		a.v = func(a.v, b.v); \
		return a; \
	} \
	template <typename T> \
	f32<4> &operator(f32<4> &a, T b) { \
		return operator(a, f32<4>(b)); \
	}

DECLARE_F32_4_OPERATOR_INCREMENT(operator+=, _mm_add_ps);
DECLARE_F32_4_OPERATOR_INCREMENT(operator-=, _mm_sub_ps);
DECLARE_F32_4_OPERATOR_INCREMENT(operator*=, _mm_mul_ps);
DECLARE_F32_4_OPERATOR_INCREMENT(operator/=, _mm_div_ps);


inline f32<4> rsqrt(f32<4> a) {
	return f32<4>(_mm_rsqrt_ps(a.v));
}

inline f32<4> rcp(f32<4> a) {
	return f32<4>(_mm_rcp_ps(a.v));
}


} // namespace simd


namespace std {

inline simd::f32<4> max(simd::f32<4> a, simd::f32<4> b) {
	return simd::f32<4>(_mm_max_ps(a.v, b.v));
}

inline simd::f32<4> min(simd::f32<4> a, simd::f32<4> b) {
	return simd::f32<4>(_mm_min_ps(a.v, b.v));
}

inline simd::f32<4> sqrt(simd::f32<4> a) {
	return simd::f32<4>(_mm_sqrt_ps(a.v));
}

} // namespace std

