#pragma once
#include <common.hpp>
#include <complex>
#include <algorithm> // for std::min, max


namespace rack {


/** Supplemental `<cmath>` functions and types
*/
namespace math {


////////////////////
// basic integer functions
////////////////////

/** Returns true if `x` is odd. */
inline bool isEven(int x) {
	return x % 2 == 0;
}

/** Returns true if `x` is odd. */
inline bool isOdd(int x) {
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
inline bool isPow2(int n) {
	return n > 0 && (n & (n - 1)) == 0;
}

////////////////////
// basic float functions
////////////////////

/** Limits `x` between `a` and `b`.
If `b < a`, returns a.
*/
inline float clamp(float x, float a, float b) {
	return std::fmax(std::fmin(x, b), a);
}

/** Limits `x` between `a` and `b`.
If `b < a`, switches the two values.
*/
inline float clampSafe(float x, float a, float b) {
	return (a <= b) ? clamp(x, a, b) : clamp(x, b, a);
}

/** Returns 1 for positive numbers, -1 for negative numbers, and 0 for zero.
See https://en.wikipedia.org/wiki/Sign_function.
*/
inline float sgn(float x) {
	return x > 0.f ? 1.f : (x < 0.f ? -1.f : 0.f);
}

/** Converts -0.f to 0.f. Leaves all other values unchanged. */
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

inline float rescale(float x, float xMin, float xMax, float yMin, float yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}

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

struct Vec {
	float x = 0.f;
	float y = 0.f;

	Vec() {}
	Vec(float x, float y) : x(x), y(y) {}

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
	bool isEqual(Vec b) const {
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
};


struct Rect {
	Vec pos;
	Vec size;

	Rect() {}
	Rect(Vec pos, Vec size) : pos(pos), size(size) {}
	Rect(float posX, float posY, float sizeX, float sizeY) : pos(math::Vec(posX, posY)), size(math::Vec(sizeX, sizeY)) {}
	/** Constructs a Rect from the upper-left position `a` and lower-right pos `b`. */
	static Rect fromMinMax(Vec a, Vec b) {
		return Rect(a, b.minus(a));
	}

	/** Returns whether this Rect contains an entire point, inclusive on the top/left, non-inclusive on the bottom/right. */
	bool isContaining(Vec v) const {
		return pos.x <= v.x && v.x < pos.x + size.x
		       && pos.y <= v.y && v.y < pos.y + size.y;
	}
	/** Returns whether this Rect contains an entire Rect. */
	bool isContaining(Rect r) const {
		return pos.x <= r.pos.x && r.pos.x + r.size.x <= pos.x + size.x
		       && pos.y <= r.pos.y && r.pos.y + r.size.y <= pos.y + size.y;
	}
	/** Returns whether this Rect overlaps with another Rect. */
	bool isIntersecting(Rect r) const {
		return (pos.x + size.x > r.pos.x && r.pos.x + r.size.x > pos.x)
		       && (pos.y + size.y > r.pos.y && r.pos.y + r.size.y > pos.y);
	}
	bool isEqual(Rect r) const {
		return pos.isEqual(r.pos) && size.isEqual(r.size);
	}
	float getRight() const {
		return pos.x + size.x;
	}
	float getBottom() const {
		return pos.y + size.y;
	}
	Vec getCenter() const {
		return pos.plus(size.mult(0.5f));
	}
	Vec getTopLeft() const {
		return pos;
	}
	Vec getTopRight() const {
		return pos.plus(Vec(size.x, 0.f));
	}
	Vec getBottomLeft() const {
		return pos.plus(Vec(0.f, size.y));
	}
	Vec getBottomRight() const {
		return pos.plus(size);
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
	/** Expands each corner.
	Use a negative delta to shrink.
	*/
	Rect grow(Vec delta) const {
		Rect r;
		r.pos = pos.minus(delta);
		r.size = size.plus(delta.mult(2.f));
		return r;
	}

	DEPRECATED bool contains(Vec v) const {
		return isContaining(v);
	}
	DEPRECATED bool contains(Rect r) const {
		return isContaining(r);
	}
	DEPRECATED bool intersects(Rect r) const {
		return isIntersecting(r);
	}
};


inline Vec Vec::clamp(Rect bound) const {
	return Vec(
	         math::clamp(x, bound.pos.x, bound.pos.x + bound.size.x),
	         math::clamp(y, bound.pos.y, bound.pos.y + bound.size.y));
}

inline Vec Vec::clampSafe(Rect bound) const {
	return Vec(
	         math::clampSafe(x, bound.pos.x, bound.pos.x + bound.size.x),
	         math::clampSafe(y, bound.pos.y, bound.pos.y + bound.size.y));
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
