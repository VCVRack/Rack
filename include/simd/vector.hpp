#pragma once
#include <cstring>
#include <x86intrin.h>
#include <type_traits>


namespace rack {


/** Abstraction of byte-aligned values for SIMD CPU acceleration. */
namespace simd {


/** Casts the literal bits of FROM to TO without type conversion.
API copied from C++20.

Usage example:

	printf("%08x\n", bit_cast<int>(1.f)); // Prints 3f800000
*/
template <typename TO, typename FROM>
TO bit_cast(const FROM &x) {
	static_assert(sizeof(FROM) == sizeof(TO), "types must have equal size");
	// Should be optimized to two `mov` instructions
	TO y;
	std::memcpy(&y, &x, sizeof(x));
	return y;
}


/** Generic class for vector types.

This class is designed to be used just like you use scalars, with extra features for handling bitwise logic, conditions, loading, and storing.

Usage example:

	float a[4], b[4];
	float_4 a = float_4::load(in);
	float_4 b = 2.f * a / (1 - a);
	b *= sin(2 * M_PI * a);
	b.store(out);
*/
template <typename T, int N>
struct Vector;


/** Wrapper for `__m128` representing an aligned vector of 4 single-precision float values.
*/
template <>
struct Vector<float, 4> {
	__m128 v;

	/** Constructs an uninitialized vector. */
	Vector<float, 4>() {}

	/** Constructs a vector from a native `__m128` type. */
	Vector<float, 4>(__m128 v) : v(v) {}

	/** Constructs a vector with all elements set to `x`. */
	Vector<float, 4>(float x) {
		v = _mm_set_ps1(x);
	}

	/** Constructs a vector from four values. */
	Vector<float, 4>(float x1, float x2, float x3, float x4) {
		v = _mm_set_ps(x1, x2, x3, x4);
	}

	/** Returns a vector initialized to zero. */
	static Vector<float, 4> zero() {
		return Vector<float, 4>(_mm_setzero_ps());
	}

	/** Reads an array of 4 values. */
	static Vector<float, 4> load(const float *x) {
		return Vector<float, 4>(_mm_loadu_ps(x));
	}

	/** Writes an array of 4 values. */
	void store(float *x) {
		_mm_storeu_ps(x, v);
	}
};


// Typedefs


typedef Vector<float, 4> float_4;
// typedef Vector<double, 2> double_2;
// typedef Vector<int32_t, 4> int32_4;


// Operator overloads


/** `a @ b` */
#define DECLARE_FLOAT_4_OPERATOR_INFIX(operator, func) \
	inline float_4 operator(const float_4 &a, const float_4 &b) { \
		return float_4(func(a.v, b.v)); \
	}

/** `a @= b` */
#define DECLARE_FLOAT_4_OPERATOR_INCREMENT(operator, opfunc) \
	inline float_4 &operator(float_4 &a, const float_4 &b) { \
		a = opfunc(a, b); \
		return a; \
	}

DECLARE_FLOAT_4_OPERATOR_INFIX(operator+, _mm_add_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator-, _mm_sub_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator*, _mm_mul_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator/, _mm_div_ps)

/* Use these to apply logic, bit masks, and conditions to elements.
Boolean operators on vectors give 0x00000000 for false and 0xffffffff for true, for each vector element.

Examples:

Subtract 1 from value if greater than or equal to 1.

	x -= (x >= 1.f) & 1.f;
*/
DECLARE_FLOAT_4_OPERATOR_INFIX(operator^, _mm_xor_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator&, _mm_and_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator|, _mm_or_ps)

DECLARE_FLOAT_4_OPERATOR_INCREMENT(operator+=, operator+);
DECLARE_FLOAT_4_OPERATOR_INCREMENT(operator-=, operator-);
DECLARE_FLOAT_4_OPERATOR_INCREMENT(operator*=, operator*);
DECLARE_FLOAT_4_OPERATOR_INCREMENT(operator/=, operator/);
DECLARE_FLOAT_4_OPERATOR_INCREMENT(operator^=, operator^);
DECLARE_FLOAT_4_OPERATOR_INCREMENT(operator&=, operator&);
DECLARE_FLOAT_4_OPERATOR_INCREMENT(operator|=, operator|);

DECLARE_FLOAT_4_OPERATOR_INFIX(operator==, _mm_cmpeq_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator>=, _mm_cmpge_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator>, _mm_cmpgt_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator<=, _mm_cmple_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator<, _mm_cmplt_ps)
DECLARE_FLOAT_4_OPERATOR_INFIX(operator!=, _mm_cmpneq_ps)

/** `+a` */
inline float_4 operator+(const float_4 &a) {
	return a;
}

/** `-a` */
inline float_4 operator-(const float_4 &a) {
	return 0.f - a;
}

/** `++a` */
inline float_4 &operator++(float_4 &a) {
	a += 1.f;
	return a;
}

/** `--a` */
inline float_4 &operator--(float_4 &a) {
	a -= 1.f;
	return a;
}

/** `a++` */
inline float_4 operator++(float_4 &a, int) {
	float_4 b = a;
	++a;
	return b;
}

/** `a--` */
inline float_4 operator--(float_4 &a, int) {
	float_4 b = a;
	--a;
	return b;
}

/** `~a` */
inline float_4 operator~(const float_4 &a) {
	float_4 mask = float_4::zero();
	mask = (mask == mask);
	return a ^ mask;
}


// Instructions not available as operators


/** `~a & b` */
inline float_4 andnot(const float_4 &a, const float_4 &b) {
	return float_4(_mm_andnot_ps(a.v, b.v));
}


} // namespace simd
} // namespace rack
