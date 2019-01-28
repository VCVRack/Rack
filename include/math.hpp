#pragma once
#include "common.hpp"
#include <algorithm> // for std::min, max


namespace rack {
namespace math {


////////////////////
// basic integer functions
////////////////////

/** Returns true if x is odd */
inline bool isEven(int x) {
	return x % 2 == 0;
}

/** Returns true if x is odd */
inline bool isOdd(int x) {
	return x % 2 != 0;
}

/** Limits `x` between `a` and `b`
Assumes a <= b
*/
inline int clamp(int x, int a, int b) {
	return std::min(std::max(x, a), b);
}

/** Limits `x` between `a` and `b`
If a > b, switches the two values
*/
inline int clampSafe(int x, int a, int b) {
	return clamp(x, std::min(a, b), std::max(a, b));
}

/** Euclidean modulus. Always returns 0 <= mod < b.
b must be positive.
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
b must be positive.
*/
inline int eucDiv(int a, int b) {
	int div = a / b;
	int mod = a % b;
	if (mod < 0) {
		div -= 1;
	}
	return div;
}

inline void eucDivMod(int a, int b, int *div, int *mod) {
	*div = a / b;
	*mod = a % b;
	if (*mod < 0) {
		*div -= 1;
		*mod += b;
	}
}

/** Returns floor(log_2(n)), or 0 if n == 1.
*/
inline int log2(int n) {
	int i = 0;
	while (n >>= 1) {
		i++;
	}
	return i;
}

/** Returns whether `n` is a power of 2 */
inline bool isPow2(int n) {
	return n > 0 && (n & (n - 1)) == 0;
}

////////////////////
// basic float functions
////////////////////

/** Limits `x` between `a` and `b`
Assumes a <= b
*/
inline float clamp(float x, float a, float b) {
	return std::min(std::max(x, a), b);
}

/** Limits `x` between `a` and `b`
If a > b, switches the two values
*/
inline float clampSafe(float x, float a, float b) {
	return clamp(x, std::min(a, b), std::max(a, b));
}

/** Returns 1 for positive numbers, -1 for negative numbers, and 0 for zero
See https://en.wikipedia.org/wiki/Sign_function
*/
inline float sgn(float x) {
	return x > 0.f ? 1.f : x < 0.f ? -1.f : 0.f;
}

/** Converts -0.f to 0.f. Leaves all other values unchanged. */
inline float normalizeZero(float x) {
	return x + 0.f;
}

/** Euclidean modulus. Always returns 0 <= mod < b.
See https://en.wikipedia.org/wiki/Euclidean_division
*/
inline float eucMod(float a, float base) {
	float mod = std::fmod(a, base);
	return (mod >= 0.f) ? mod : mod + base;
}

inline bool isNear(float a, float b, float epsilon = 1e-6f) {
	return std::abs(a - b) <= epsilon;
}

/** If the magnitude of x if less than epsilon, return 0 */
inline float chop(float x, float epsilon = 1e-6f) {
	return isNear(x, 0.f, epsilon) ? 0.f : x;
}

inline float rescale(float x, float a, float b, float yMin, float yMax) {
	return yMin + (x - a) / (b - a) * (yMax - yMin);
}

inline float crossfade(float a, float b, float p) {
	return a + (b - a) * p;
}

/** Linearly interpolate an array `p` with index `x`
Assumes that the array at `p` is of length at least floor(x)+1.
*/
inline float interpolateLinear(const float *p, float x) {
	int xi = x;
	float xf = x - xi;
	return crossfade(p[xi], p[xi+1], xf);
}

/** Complex multiply c = a * b
Arguments may be the same pointers
i.e. cmultf(&ar, &ai, ar, ai, br, bi)
*/
inline void complexMult(float *cr, float *ci, float ar, float ai, float br, float bi) {
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

	/** Negates the vector
	Equivalent to a reflection across the y=-x line.
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
	float norm() const {
		return std::hypotf(x, y);
	}
	float square() const {
		return x * x + y * y;
	}
	/** Rotates counterclockwise in radians */
	Vec rotate(float angle) {
		float sin = std::sin(angle);
		float cos = std::cos(angle);
		return Vec(x * cos - y * sin, x * sin + y * cos);
	}
	/** Swaps the coordinates
	Equivalent to a reflection across the y=x line.
	*/
	Vec flip() const {
		return Vec(y, x);
	}
	Vec min(Vec b) const {
		return Vec(std::min(x, b.x), std::min(y, b.y));
	}
	Vec max(Vec b) const {
		return Vec(std::max(x, b.x), std::max(y, b.y));
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
	/** Constructs a Rect from the upper-left position `a` and lower-right pos `b` */
	static Rect fromMinMax(Vec a, Vec b) {
		return Rect(a, b.minus(a));
	}

	/** Returns whether this Rect contains an entire point, inclusive on the top/left, non-inclusive on the bottom/right */
	bool isContaining(Vec v) const {
		return pos.x <= v.x && v.x < pos.x + size.x
			&& pos.y <= v.y && v.y < pos.y + size.y;
	}
	/** Returns whether this Rect contains an entire Rect */
	bool isContaining(Rect r) const {
		return pos.x <= r.pos.x && r.pos.x + r.size.x <= pos.x + size.x
			&& pos.y <= r.pos.y && r.pos.y + r.size.y <= pos.y + size.y;
	}
	/** Returns whether this Rect overlaps with another Rect */
	bool isIntersecting(Rect r) const {
		return (pos.x + size.x > r.pos.x && r.pos.x + r.size.x > pos.x)
			&& (pos.y + size.y > r.pos.y && r.pos.y + r.size.y > pos.y);
	}
	bool isEqual(Rect r) const {
		return pos.isEqual(r.pos) && size.isEqual(r.size);
	}
	float getLeft() const {
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
	/** Clamps the edges of the rectangle to fit within a bound */
	Rect clamp(Rect bound) const {
		Rect r;
		r.pos.x = math::clampSafe(pos.x, bound.pos.x, bound.pos.x + bound.size.x);
		r.pos.y = math::clampSafe(pos.y, bound.pos.y, bound.pos.y + bound.size.y);
		r.size.x = math::clamp(pos.x + size.x, bound.pos.x, bound.pos.x + bound.size.x) - r.pos.x;
		r.size.y = math::clamp(pos.y + size.y, bound.pos.y, bound.pos.y + bound.size.y) - r.pos.y;
		return r;
	}
	/** Nudges the position to fix inside a bounding box */
	Rect nudge(Rect bound) const {
		Rect r;
		r.size = size;
		r.pos.x = math::clampSafe(pos.x, bound.pos.x, bound.pos.x + bound.size.x - size.x);
		r.pos.y = math::clampSafe(pos.y, bound.pos.y, bound.pos.y + bound.size.y - size.y);
		return r;
	}
	/** Expands this Rect to contain `b` */
	Rect expand(Rect b) const {
		Rect r;
		r.pos.x = std::min(pos.x, b.pos.x);
		r.pos.y = std::min(pos.y, b.pos.y);
		r.size.x = std::max(pos.x + size.x, b.pos.x + b.size.x) - r.pos.x;
		r.size.y = std::max(pos.y + size.y, b.pos.y + b.size.y) - r.pos.y;
		return r;
	}
	/** Returns the intersection of `this` and `b` */
	Rect intersect(Rect b) const {
		Rect r;
		r.pos.x = std::max(pos.x, b.pos.x);
		r.pos.y = std::max(pos.y, b.pos.y);
		r.size.x = std::min(pos.x + size.x, b.pos.x + b.size.x) - r.pos.x;
		r.size.y = std::min(pos.y + size.y, b.pos.y + b.size.y) - r.pos.y;
		return r;
	}
	/** Returns a Rect with its position set to zero */
	Rect zeroPos() const {
		return Rect(Vec(), size);
	}
	/** Expands each corner
	Use a negative delta to shrink.
	*/
	Rect grow(Vec delta) const {
		Rect r;
		r.pos = pos.minus(delta);
		r.size = size.plus(delta.mult(2.f));
		return r;
	}

	DEPRECATED bool contains(Vec v) const {return isContaining(v);}
	DEPRECATED bool contains(Rect r) const {return isContaining(r);}
	DEPRECATED bool intersects(Rect r) const {return isIntersecting(r);}
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


/** Useful for debugging Vecs and Rects, e.g.
	printf("%f %f %f %f", RECT_ARGS(r));
*/
#define VEC_ARGS(v) (v).x, (v).y
#define RECT_ARGS(r) (r).pos.x, (r).pos.y, (r).size.x, (r).size.y


} // namespace math
} // namespace rack
