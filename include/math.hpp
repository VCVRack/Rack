#pragma once
#include <complex>
#include <algorithm> // for std::min, max

#include <common.hpp>


namespace rack {
/** Extends `<cmath>` with extra functions and types */
namespace math {


////////////////////
// basic integer functions
////////////////////

/** Returns true if `x` is odd. */
template <typename T>
bool isEven(T x) {
	return x % 2 == 0;
}

/** Returns true if `x` is odd. */
template <typename T>
bool isOdd(T x) {
	return x % 2 != 0;
}

/** Limits `x` between `a` and `b`.
If `b < a`, returns a.
*/
inline int clamp(int x, int a, int b) {
	return std::max(std::min(x, b), a);
}

/** Limits `x` between `a` and `b`.
If `b < a`, switches the two values.
*/
inline int clampSafe(int x, int a, int b) {
	return (a <= b) ? clamp(x, a, b) : clamp(x, b, a);
}

/** Euclidean modulus. Always returns `0 <= mod < b`.
`b` must be positive.
See https://en.wikipedia.org/wiki/Euclidean_division
*/
inline int eucMod(int a, int b) {
	int mod = a % b;
	if (mod < 0) {
		mod += b;
	}
	return mod;
}

/** Euclidean division.
`b` must be positive.
*/
inline int eucDiv(int a, int b) {
	int div = a / b;
	int mod = a % b;
	if (mod < 0) {
		div -= 1;
	}
	return div;
}

inline void eucDivMod(int a, int b, int* div, int* mod) {
	*div = a / b;
	*mod = a % b;
	if (*mod < 0) {
		*div -= 1;
		*mod += b;
	}
}

/** Returns `floor(log_2(n))`, or 0 if `n == 1`. */
inline int log2(int n) {
	int i = 0;
	while (n >>= 1) {
		i++;
	}
	return i;
}

/** Returns whether `n` is a power of 2. */
template <typename T>
bool isPow2(T n) {
	return n > 0 && (n & (n - 1)) == 0;
}

/** Returns 1 for positive numbers, -1 for negative numbers, and 0 for zero.
See https://en.wikipedia.org/wiki/Sign_function.
*/
template <typename T>
T sgn(T x) {
	return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

////////////////////
// basic float functions
////////////////////

/** Limits `x` between `a` and `b`.
If `b < a`, returns a.
*/
inline float clamp(float x, float a = 0.f, float b = 1.f) {
	return std::fmax(std::fmin(x, b), a);
}

/** Limits `x` between `a` and `b`.
If `b < a`, switches the two values.
*/
inline float clampSafe(float x, float a = 0.f, float b = 1.f) {
	return (a <= b) ? clamp(x, a, b) : clamp(x, b, a);
}

/** Converts -0.f to 0.f. Leaves all other values unchanged. */
#if defined __clang__
// Clang doesn't support disabling individual optimizations, just everything.
__attribute__((optnone))
#else
__attribute__((optimize("signed-zeros")))
#endif
inline float normalizeZero(float x) {
	return x + 0.f;
}

/** Euclidean modulus. Always returns `0 <= mod < b`.
See https://en.wikipedia.org/wiki/Euclidean_division.
*/
inline float eucMod(float a, float b) {
	float mod = std::fmod(a, b);
	if (mod < 0.f) {
		mod += b;
	}
	return mod;
}

/** Returns whether `a` is within epsilon distance from `b`. */
inline bool isNear(float a, float b, float epsilon = 1e-6f) {
	return std::fabs(a - b) <= epsilon;
}

/** If the magnitude of `x` if less than epsilon, return 0. */
inline float chop(float x, float epsilon = 1e-6f) {
	return std::fabs(x) <= epsilon ? 0.f : x;
}

/** Rescales `x` from the range `[xMin, xMax]` to `[yMin, yMax]`.
*/
inline float rescale(float x, float xMin, float xMax, float yMin, float yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}

/** Linearly interpolates between `a` and `b`, from `p = 0` to `p = 1`.
*/
inline float crossfade(float a, float b, float p) {
	return a + (b - a) * p;
}

/** Linearly interpolates an array `p` with index `x`.
The array at `p` must be at least length `floor(x) + 2`.
*/
inline float interpolateLinear(const float* p, float x) {
	int xi = x;
	float xf = x - xi;
	return crossfade(p[xi], p[xi + 1], xf);
}

/** Complex multiplication `c = a * b`.
Arguments may be the same pointers.
Example:

	cmultf(ar, ai, br, bi, &ar, &ai);
*/
inline void complexMult(float ar, float ai, float br, float bi, float* cr, float* ci) {
	*cr = ar * br - ai * bi;
	*ci = ar * bi + ai * br;
}

////////////////////
// 2D vector and rectangle
////////////////////

struct Rect;

/** 2-dimensional vector of floats, representing a point on the plane for graphics.
*/
struct Vec {
	float x = 0.f;
	float y = 0.f;

	Vec() {}
	Vec(float xy) : x(xy), y(xy) {}
	Vec(float x, float y) : x(x), y(y) {}

	float& operator[](int i) {
		return (i == 0) ? x : y;
	}
	const float& operator[](int i) const {
		return (i == 0) ? x : y;
	}
	/** Negates the vector.
	Equivalent to a reflection across the `y = -x` line.
	*/
	Vec neg() const {
		return Vec(-x, -y);
	}
	Vec plus(Vec b) const {
		return Vec(x + b.x, y + b.y);
	}
	Vec minus(Vec b) const {
		return Vec(x - b.x, y - b.y);
	}
	Vec mult(float s) const {
		return Vec(x * s, y * s);
	}
	Vec mult(Vec b) const {
		return Vec(x * b.x, y * b.y);
	}
	Vec div(float s) const {
		return Vec(x / s, y / s);
	}
	Vec div(Vec b) const {
		return Vec(x / b.x, y / b.y);
	}
	float dot(Vec b) const {
		return x * b.x + y * b.y;
	}
	float arg() const {
		return std::atan2(y, x);
	}
	float norm() const {
		return std::hypot(x, y);
	}
	Vec normalize() const {
		return div(norm());
	}
	float square() const {
		return x * x + y * y;
	}
	float area() const {
		return x * y;
	}
	/** Rotates counterclockwise in radians. */
	Vec rotate(float angle) {
		float sin = std::sin(angle);
		float cos = std::cos(angle);
		return Vec(x * cos - y * sin, x * sin + y * cos);
	}
	/** Swaps the coordinates.
	Equivalent to a reflection across the `y = x` line.
	*/
	Vec flip() const {
		return Vec(y, x);
	}
	Vec min(Vec b) const {
		return Vec(std::fmin(x, b.x), std::fmin(y, b.y));
	}
	Vec max(Vec b) const {
		return Vec(std::fmax(x, b.x), std::fmax(y, b.y));
	}
	Vec abs() const {
		return Vec(std::fabs(x), std::fabs(y));
	}
	Vec round() const {
		return Vec(std::round(x), std::round(y));
	}
	Vec floor() const {
		return Vec(std::floor(x), std::floor(y));
	}
	Vec ceil() const {
		return Vec(std::ceil(x), std::ceil(y));
	}
	bool equals(Vec b) const {
		return x == b.x && y == b.y;
	}
	bool isZero() const {
		return x == 0.f && y == 0.f;
	}
	bool isFinite() const {
		return std::isfinite(x) && std::isfinite(y);
	}
	Vec clamp(Rect bound) const;
	Vec clampSafe(Rect bound) const;
	Vec crossfade(Vec b, float p) {
		return this->plus(b.minus(*this).mult(p));
	}

	// Method aliases
	bool isEqual(Vec b) const {
		return equals(b);
	}
};


/** 2-dimensional rectangle for graphics.
Mathematically, Rects include points on its left/top edge but *not* its right/bottom edge.
The infinite Rect (equal to the entire plane) is defined using pos=-inf and size=inf.
*/
struct Rect {
	Vec pos;
	Vec size;

	Rect() {}
	Rect(Vec pos, Vec size) : pos(pos), size(size) {}
	Rect(float posX, float posY, float sizeX, float sizeY) : pos(Vec(posX, posY)), size(Vec(sizeX, sizeY)) {}
	/** Constructs a Rect from a top-left and bottom-right vector.
	*/
	static Rect fromMinMax(Vec a, Vec b) {
		return Rect(a, b.minus(a));
	}
	/** Constructs a Rect from any two opposite corners.
	*/
	static Rect fromCorners(Vec a, Vec b) {
		return fromMinMax(a.min(b), a.max(b));
	}
	/** Returns the infinite Rect. */
	static Rect inf() {
		return Rect(Vec(-INFINITY, -INFINITY), Vec(INFINITY, INFINITY));
	}

	/** Returns whether this Rect contains a point, inclusive on the left/top, exclusive on the right/bottom.
	Correctly handles infinite Rects.
	*/
	bool contains(Vec v) const {
		return (pos.x <= v.x) && (size.x == INFINITY || v.x < pos.x + size.x)
		    && (pos.y <= v.y) && (size.y == INFINITY || v.y < pos.y + size.y);
	}
	/** Returns whether this Rect contains (is a superset of) a Rect.
	Correctly handles infinite Rects.
	*/
	bool contains(Rect r) const {
		return (pos.x <= r.pos.x) && (r.pos.x - size.x <= pos.x - r.size.x)
		    && (pos.y <= r.pos.y) && (r.pos.y - size.y <= pos.y - r.size.y);
	}
	/** Returns whether this Rect overlaps with another Rect.
	Correctly handles infinite Rects.
	*/
	bool intersects(Rect r) const {
		return (r.size.x == INFINITY || pos.x < r.pos.x + r.size.x) && (size.x == INFINITY || r.pos.x < pos.x + size.x)
		    && (r.size.y == INFINITY || pos.y < r.pos.y + r.size.y) && (size.y == INFINITY || r.pos.y < pos.y + size.y);
	}
	bool equals(Rect r) const {
		return pos.equals(r.pos) && size.equals(r.size);
	}
	float getLeft() const {
		return pos.x;
	}
	float getRight() const {
		return (size.x == INFINITY) ? INFINITY : (pos.x + size.x);
	}
	float getTop() const {
		return pos.y;
	}
	float getBottom() const {
		return (size.y == INFINITY) ? INFINITY : (pos.y + size.y);
	}
	float getWidth() const {
		return size.x;
	}
	float getHeight() const {
		return size.y;
	}
	/** Returns the center point of the rectangle.
	Returns a NaN coordinate if pos=-inf and size=inf.
	*/
	Vec getCenter() const {
		return pos.plus(size.mult(0.5f));
	}
	Vec getTopLeft() const {
		return pos;
	}
	Vec getTopRight() const {
		return Vec(getRight(), getTop());
	}
	Vec getBottomLeft() const {
		return Vec(getLeft(), getBottom());
	}
	Vec getBottomRight() const {
		return Vec(getRight(), getBottom());
	}
	/** Clamps the edges of the rectangle to fit within a bound. */
	Rect clamp(Rect bound) const {
		Rect r;
		r.pos.x = math::clampSafe(pos.x, bound.pos.x, bound.pos.x + bound.size.x);
		r.pos.y = math::clampSafe(pos.y, bound.pos.y, bound.pos.y + bound.size.y);
		r.size.x = math::clamp(pos.x + size.x, bound.pos.x, bound.pos.x + bound.size.x) - r.pos.x;
		r.size.y = math::clamp(pos.y + size.y, bound.pos.y, bound.pos.y + bound.size.y) - r.pos.y;
		return r;
	}
	/** Nudges the position to fix inside a bounding box. */
	Rect nudge(Rect bound) const {
		Rect r;
		r.size = size;
		r.pos.x = math::clampSafe(pos.x, bound.pos.x, bound.pos.x + bound.size.x - size.x);
		r.pos.y = math::clampSafe(pos.y, bound.pos.y, bound.pos.y + bound.size.y - size.y);
		return r;
	}
	/** Returns the bounding box of the union of `this` and `b`. */
	Rect expand(Rect b) const {
		Rect r;
		r.pos.x = std::fmin(pos.x, b.pos.x);
		r.pos.y = std::fmin(pos.y, b.pos.y);
		r.size.x = std::fmax(pos.x + size.x, b.pos.x + b.size.x) - r.pos.x;
		r.size.y = std::fmax(pos.y + size.y, b.pos.y + b.size.y) - r.pos.y;
		return r;
	}
	/** Returns the intersection of `this` and `b`. */
	Rect intersect(Rect b) const {
		Rect r;
		r.pos.x = std::fmax(pos.x, b.pos.x);
		r.pos.y = std::fmax(pos.y, b.pos.y);
		r.size.x = std::fmin(pos.x + size.x, b.pos.x + b.size.x) - r.pos.x;
		r.size.y = std::fmin(pos.y + size.y, b.pos.y + b.size.y) - r.pos.y;
		return r;
	}
	/** Returns a Rect with its position set to zero. */
	Rect zeroPos() const {
		return Rect(Vec(), size);
	}
	/** Expands each corner. */
	Rect grow(Vec delta) const {
		Rect r;
		r.pos = pos.minus(delta);
		r.size = size.plus(delta.mult(2.f));
		return r;
	}
	/** Contracts each corner. */
	Rect shrink(Vec delta) const {
		Rect r;
		r.pos = pos.plus(delta);
		r.size = size.minus(delta.mult(2.f));
		return r;
	}
	/** Returns `pos + size * p` */
	Vec interpolate(Vec p) {
		return pos.plus(size.mult(p));
	}

	// Method aliases
	bool isContaining(Vec v) const {
		return contains(v);
	}
	bool isIntersecting(Rect r) const {
		return intersects(r);
	}
	bool isEqual(Rect r) const {
		return equals(r);
	}
};


inline Vec Vec::clamp(Rect bound) const {
	return Vec(
		math::clamp(x, bound.pos.x, bound.pos.x + bound.size.x),
		math::clamp(y, bound.pos.y, bound.pos.y + bound.size.y)
	);
}

inline Vec Vec::clampSafe(Rect bound) const {
	return Vec(
		math::clampSafe(x, bound.pos.x, bound.pos.x + bound.size.x),
		math::clampSafe(y, bound.pos.y, bound.pos.y + bound.size.y)
	);
}


// Operator overloads for Vec
inline Vec operator+(const Vec& a) {
	return a;
}
inline Vec operator-(const Vec& a) {
	return a.neg();
}
inline Vec operator+(const Vec& a, const Vec& b) {
	return a.plus(b);
}
inline Vec operator-(const Vec& a, const Vec& b) {
	return a.minus(b);
}
inline Vec operator*(const Vec& a, const Vec& b) {
	return a.mult(b);
}
inline Vec operator*(const Vec& a, const float& b) {
	return a.mult(b);
}
inline Vec operator*(const float& a, const Vec& b) {
	return b.mult(a);
}
inline Vec operator/(const Vec& a, const Vec& b) {
	return a.div(b);
}
inline Vec operator/(const Vec& a, const float& b) {
	return a.div(b);
}
inline Vec operator+=(Vec& a, const Vec& b) {
	return a = a.plus(b);
}
inline Vec operator-=(Vec& a, const Vec& b) {
	return a = a.minus(b);
}
inline Vec operator*=(Vec& a, const Vec& b) {
	return a = a.mult(b);
}
inline Vec operator*=(Vec& a, const float& b) {
	return a = a.mult(b);
}
inline Vec operator/=(Vec& a, const Vec& b) {
	return a = a.div(b);
}
inline Vec operator/=(Vec& a, const float& b) {
	return a = a.div(b);
}
inline bool operator==(const Vec& a, const Vec& b) {
	return a.equals(b);
}
inline bool operator!=(const Vec& a, const Vec& b) {
	return !a.equals(b);
}


// Operator overloads for Rect
inline bool operator==(const Rect& a, const Rect& b) {
	return a.equals(b);
}
inline bool operator!=(const Rect& a, const Rect& b) {
	return !a.equals(b);
}


/** Expands a Vec and Rect into a comma-separated list.
Useful for print debugging.

	printf("(%f %f) (%f %f %f %f)", VEC_ARGS(v), RECT_ARGS(r));

Or passing the values to a C function.

	nvgRect(vg, RECT_ARGS(r));
*/
#define VEC_ARGS(v) (v).x, (v).y
#define RECT_ARGS(r) (r).pos.x, (r).pos.y, (r).size.x, (r).size.y


} // namespace math
} // namespace rack
