#pragma once
#include <cstring>
#include "common.hpp"


namespace rack {


/** Abstraction of aligned types for SIMD computation
*/
namespace simd {


/** Generic class for vector types.

This class is designed to be used just like you use scalars, with extra features for handling bitwise logic, conditions, loading, and storing.

Example:

	float a[4], b[4];
	float_4 a = float_4::load(in);
	float_4 b = 2.f * a / (1 - a);
	b *= sin(2 * M_PI * a);
	b.store(out);
*/
template <typename TYPE, int SIZE>
struct Vector;


/** Wrapper for `__m128` representing an aligned vector of 4 single-precision float values.
*/
template <>
struct Vector<float, 4> {
	using type = float;
	constexpr static int size = 4;

	union {
		__m128 v;
		/** Accessing this array of scalars is slow and defeats the purpose of vectorizing.
		*/
		float s[4];
	};

	/** Constructs an uninitialized vector. */
	Vector() = default;

	/** Constructs a vector from a native `__m128` type. */
	Vector(__m128 v) : v(v) {}

	/** Constructs a vector with all elements set to `x`. */
	Vector(float x) {
		v = _mm_set1_ps(x);
	}

	/** Constructs a vector from four scalars. */
	Vector(float x1, float x2, float x3, float x4) {
		v = _mm_setr_ps(x1, x2, x3, x4);
	}

	/** Returns a vector with all 0 bits. */
	static Vector zero() {
		return Vector(_mm_setzero_ps());
	}

	/** Returns a vector with all 1 bits. */
	static Vector mask() {
		return Vector(_mm_castsi128_ps(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128())));
	}

	/** Reads an array of 4 values.
	On little-endian machines (e.g. x86_64), the order is reversed, so `x[0]` corresponds to `vector.s[3]`.
	*/
	static Vector load(const float* x) {
		/*
		My benchmarks show that _mm_loadu_ps() performs equally as fast as _mm_load_ps() when data is actually aligned.
		This post seems to agree. https://stackoverflow.com/a/20265193/272642
		I therefore use _mm_loadu_ps() for generality, so you can load unaligned arrays using the same function (although load aligned arrays if you can for best performance).
		*/
		return Vector(_mm_loadu_ps(x));
	}

	/** Writes an array of 4 values.
	On little-endian machines (e.g. x86_64), the order is reversed, so `x[0]` corresponds to `vector.s[3]`.
	*/
	void store(float* x) {
		_mm_storeu_ps(x, v);
	}

	/** Accessing vector elements individually is slow and defeats the purpose of vectorizing.
	However, this operator is convenient when writing simple serial code in a non-bottlenecked section.
	*/
	float& operator[](int i) {
		return s[i];
	}
	const float& operator[](int i) const {
		return s[i];
	}

	// Conversions
	Vector(Vector<int32_t, 4> a);
	// Casts
	static Vector cast(Vector<int32_t, 4> a);
};


template <>
struct Vector<int32_t, 4> {
	using type = int32_t;
	constexpr static int size = 4;

	union {
		__m128i v;
		int32_t s[4];
	};

	Vector() = default;
	Vector(__m128i v) : v(v) {}
	Vector(int32_t x) {
		v = _mm_set1_epi32(x);
	}
	Vector(int32_t x1, int32_t x2, int32_t x3, int32_t x4) {
		v = _mm_setr_epi32(x1, x2, x3, x4);
	}
	static Vector zero() {
		return Vector(_mm_setzero_si128());
	}
	static Vector mask() {
		return Vector(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
	}
	static Vector load(const int32_t* x) {
		// HACK
		// Use _mm_loadu_si128() because GCC doesn't support _mm_loadu_si32()
		return Vector(_mm_loadu_si128((const __m128i*) x));
	}
	void store(int32_t* x) {
		// HACK
		// Use _mm_storeu_si128() because GCC doesn't support _mm_storeu_si32()
		_mm_storeu_si128((__m128i*) x, v);
	}
	int32_t& operator[](int i) {
		return s[i];
	}
	const int32_t& operator[](int i) const {
		return s[i];
	}
	Vector(Vector<float, 4> a);
	static Vector cast(Vector<float, 4> a);
};


// Conversions and casts


inline Vector<float, 4>::Vector(Vector<int32_t, 4> a) {
	v = _mm_cvtepi32_ps(a.v);
}

inline Vector<int32_t, 4>::Vector(Vector<float, 4> a) {
	v = _mm_cvttps_epi32(a.v);
}

inline Vector<float, 4> Vector<float, 4>::cast(Vector<int32_t, 4> a) {
	return Vector(_mm_castsi128_ps(a.v));
}

inline Vector<int32_t, 4> Vector<int32_t, 4>::cast(Vector<float, 4> a) {
	return Vector(_mm_castps_si128(a.v));
}


// Operator overloads


/** `a @ b` */
#define DECLARE_VECTOR_OPERATOR_INFIX(t, s, operator, func) \
	inline Vector<t, s> operator(const Vector<t, s>& a, const Vector<t, s>& b) { \
		return Vector<t, s>(func(a.v, b.v)); \
	}

/** `a @= b` */
#define DECLARE_VECTOR_OPERATOR_INCREMENT(t, s, operator, opfunc) \
	inline Vector<t, s>& operator(Vector<t, s>& a, const Vector<t, s>& b) { \
		return a = opfunc(a, b); \
	}

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator+, _mm_add_ps)
DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator+, _mm_add_epi32)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator-, _mm_sub_ps)
DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator-, _mm_sub_epi32)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator*, _mm_mul_ps)
// DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator*, NOT AVAILABLE IN SSE3)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator/, _mm_div_ps)
// DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator/, NOT AVAILABLE IN SSE3)

/* Use these to apply logic, bit masks, and conditions to elements.
Boolean operators on vectors give 0x00000000 for false and 0xffffffff for true, for each vector element.

Examples:

Subtract 1 from value if greater than or equal to 1.

	x -= (x >= 1.f) & 1.f;
*/
DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator^, _mm_xor_ps)
DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator^, _mm_xor_si128)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator&, _mm_and_ps)
DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator&, _mm_and_si128)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator|, _mm_or_ps)
DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator|, _mm_or_si128)

DECLARE_VECTOR_OPERATOR_INCREMENT(float, 4, operator+=, operator+)
DECLARE_VECTOR_OPERATOR_INCREMENT(int32_t, 4, operator+=, operator+)

DECLARE_VECTOR_OPERATOR_INCREMENT(float, 4, operator-=, operator-)
DECLARE_VECTOR_OPERATOR_INCREMENT(int32_t, 4, operator-=, operator-)

DECLARE_VECTOR_OPERATOR_INCREMENT(float, 4, operator*=, operator*)
// DECLARE_VECTOR_OPERATOR_INCREMENT(int32_t, 4, operator*=, NOT AVAILABLE IN SSE3)

DECLARE_VECTOR_OPERATOR_INCREMENT(float, 4, operator/=, operator/)
// DECLARE_VECTOR_OPERATOR_INCREMENT(int32_t, 4, operator/=, NOT AVAILABLE IN SSE3)

DECLARE_VECTOR_OPERATOR_INCREMENT(float, 4, operator^=, operator^)
DECLARE_VECTOR_OPERATOR_INCREMENT(int32_t, 4, operator^=, operator^)

DECLARE_VECTOR_OPERATOR_INCREMENT(float, 4, operator&=, operator&)
DECLARE_VECTOR_OPERATOR_INCREMENT(int32_t, 4, operator&=, operator&)

DECLARE_VECTOR_OPERATOR_INCREMENT(float, 4, operator|=, operator|)
DECLARE_VECTOR_OPERATOR_INCREMENT(int32_t, 4, operator|=, operator|)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator==, _mm_cmpeq_ps)
DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator==, _mm_cmpeq_epi32)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator>=, _mm_cmpge_ps)
inline Vector<int32_t, 4> operator>=(const Vector<int32_t, 4>& a, const Vector<int32_t, 4>& b) {
	return Vector<int32_t, 4>(_mm_cmpgt_epi32(a.v, b.v)) ^ Vector<int32_t, 4>::mask();
}

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator>, _mm_cmpgt_ps)
DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator>, _mm_cmpgt_epi32)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator<=, _mm_cmple_ps)
inline Vector<int32_t, 4> operator<=(const Vector<int32_t, 4>& a, const Vector<int32_t, 4>& b) {
	return Vector<int32_t, 4>(_mm_cmplt_epi32(a.v, b.v)) ^ Vector<int32_t, 4>::mask();
}

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator<, _mm_cmplt_ps)
DECLARE_VECTOR_OPERATOR_INFIX(int32_t, 4, operator<, _mm_cmplt_epi32)

DECLARE_VECTOR_OPERATOR_INFIX(float, 4, operator!=, _mm_cmpneq_ps)
inline Vector<int32_t, 4> operator!=(const Vector<int32_t, 4>& a, const Vector<int32_t, 4>& b) {
	return Vector<int32_t, 4>(_mm_cmpeq_epi32(a.v, b.v)) ^ Vector<int32_t, 4>::mask();
}

/** `+a` */
inline Vector<float, 4> operator+(const Vector<float, 4>& a) {
	return a;
}
inline Vector<int32_t, 4> operator+(const Vector<int32_t, 4>& a) {
	return a;
}

/** `-a` */
inline Vector<float, 4> operator-(const Vector<float, 4>& a) {
	return 0.f - a;
}
inline Vector<int32_t, 4> operator-(const Vector<int32_t, 4>& a) {
	return 0 - a;
}

/** `++a` */
inline Vector<float, 4>& operator++(Vector<float, 4>& a) {
	return a += 1.f;
}
inline Vector<int32_t, 4>& operator++(Vector<int32_t, 4>& a) {
	return a += 1;
}

/** `--a` */
inline Vector<float, 4>& operator--(Vector<float, 4>& a) {
	return a -= 1.f;
}
inline Vector<int32_t, 4>& operator--(Vector<int32_t, 4>& a) {
	return a -= 1;
}

/** `a++` */
inline Vector<float, 4> operator++(Vector<float, 4>& a, int) {
	Vector<float, 4> b = a;
	++a;
	return b;
}
inline Vector<int32_t, 4> operator++(Vector<int32_t, 4>& a, int) {
	Vector<int32_t, 4> b = a;
	++a;
	return b;
}

/** `a--` */
inline Vector<float, 4> operator--(Vector<float, 4>& a, int) {
	Vector<float, 4> b = a;
	--a;
	return b;
}
inline Vector<int32_t, 4> operator--(Vector<int32_t, 4>& a, int) {
	Vector<int32_t, 4> b = a;
	--a;
	return b;
}

/** `~a` */
inline Vector<float, 4> operator~(const Vector<float, 4>& a) {
	return a ^ Vector<float, 4>::mask();
}
inline Vector<int32_t, 4> operator~(const Vector<int32_t, 4>& a) {
	return a ^ Vector<int32_t, 4>::mask();
}

/** `a << b` */
inline Vector<int32_t, 4> operator<<(const Vector<int32_t, 4>& a, const int& b) {
	return Vector<int32_t, 4>(_mm_sll_epi32(a.v, _mm_cvtsi32_si128(b)));
}

/** `a >> b` */
inline Vector<int32_t, 4> operator>>(const Vector<int32_t, 4>& a, const int& b) {
	return Vector<int32_t, 4>(_mm_srl_epi32(a.v, _mm_cvtsi32_si128(b)));
}


// Typedefs


using float_4 = Vector<float, 4>;
using int32_4 = Vector<int32_t, 4>;


} // namespace simd
} // namespace rack
