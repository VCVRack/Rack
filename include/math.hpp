#pragma once
#include <algorithm> // for std::min, max
#include "common.hpp"


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
inline int clampBetween(int x, int a, int b) {
	return clamp(x, std::min(a, b), std::max(a, b));
}

/** Euclidean modulus. Always returns 0 <= mod < b.
b must be positive.
*/
inline int eucMod(int a, int b) {
	int mod = a % b;
	return (mod >= 0) ? mod : mod + b;
}

/** Euclidean division.
b must be positive.
*/
inline int eucDiv(int a, int b) {
	int mod = a % b;
	int div = a / b;
	return (mod >= 0) ? div : div - 1;
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
inline float clampBetween(float x, float a, float b) {
	return clamp(x, std::min(a, b), std::max(a, b));
}

/** Returns 1 for positive numbers, -1 for negative numbers, and 0 for zero */
inline float sgn(float x) {
	return x > 0.f ? 1.f : x < 0.f ? -1.f : 0.f;
}

inline float eucMod(float a, float base) {
	float mod = std::fmod(a, base);
	return (mod >= 0.0f) ? mod : mod + base;
}

inline bool isNear(float a, float b, float epsilon = 1.0e-6f) {
	return std::abs(a - b) <= epsilon;
}

/** If the magnitude of x if less than epsilon, return 0 */
inline float chop(float x, float epsilon = 1.0e-6f) {
	return isNear(x, 0.f, epsilon) ? 0.f : x;
}

inline float rescale(float x, float a, float b, float yMin, float yMax) {
	return yMin + (x - a) / (b - a) * (yMax - yMin);
}

inline float crossfade(float a, float b, float frac) {
	return a + frac * (b - a);
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
inline void cmult(float *cr, float *ci, float ar, float ai, float br, float bi) {
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

	Vec neg() {
		return Vec(-x, -y);
	}
	Vec plus(Vec b) {
		return Vec(x + b.x, y + b.y);
	}
	Vec minus(Vec b) {
		return Vec(x - b.x, y - b.y);
	}
	Vec mult(float s) {
		return Vec(x * s, y * s);
	}
	Vec mult(Vec b) {
		return Vec(x * b.x, y * b.y);
	}
	Vec div(float s) {
		return Vec(x / s, y / s);
	}
	Vec div(Vec b) {
		return Vec(x / b.x, y / b.y);
	}
	float dot(Vec b) {
		return x * b.x + y * b.y;
	}
	float norm() {
		return std::hypotf(x, y);
	}
	Vec flip() {
		return Vec(y, x);
	}
	Vec min(Vec b) {
		return Vec(std::min(x, b.x), std::min(y, b.y));
	}
	Vec max(Vec b) {
		return Vec(std::max(x, b.x), std::max(y, b.y));
	}
	Vec round() {
		return Vec(std::round(x), std::round(y));
	}
	Vec floor() {
		return Vec(std::floor(x), std::floor(y));
	}
	Vec ceil() {
		return Vec(std::ceil(x), std::ceil(y));
	}
	bool isEqual(Vec b) {
		return x == b.x && y == b.y;
	}
	bool isZero() {
		return x == 0.0f && y == 0.0f;
	}
	bool isFinite() {
		return std::isfinite(x) && std::isfinite(y);
	}
	Vec clamp(Rect bound);
	Vec clampBetween(Rect bound);
	DEPRECATED Vec clamp2(Rect bound);
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
	bool contains(Vec v) {
		return pos.x <= v.x && v.x < pos.x + size.x
			&& pos.y <= v.y && v.y < pos.y + size.y;
	}
	/** Returns whether this Rect contains an entire Rect */
	bool contains(Rect r) {
		return pos.x <= r.pos.x && r.pos.x + r.size.x <= pos.x + size.x
			&& pos.y <= r.pos.y && r.pos.y + r.size.y <= pos.y + size.y;
	}
	/** Returns whether this Rect overlaps with another Rect */
	bool intersects(Rect r) {
		return (pos.x + size.x > r.pos.x && r.pos.x + r.size.x > pos.x)
			&& (pos.y + size.y > r.pos.y && r.pos.y + r.size.y > pos.y);
	}
	bool isEqual(Rect r) {
		return pos.isEqual(r.pos) && size.isEqual(r.size);
	}
	Vec getCenter() {
		return pos.plus(size.mult(0.5f));
	}
	Vec getTopLeft() {
		return pos;
	}
	Vec getTopRight() {
		return pos.plus(Vec(size.x, 0.f));
	}
	Vec getBottomLeft() {
		return pos.plus(Vec(0.f, size.y));
	}
	Vec getBottomRight() {
		return pos.plus(size);
	}
	/** Clamps the edges of the rectangle to fit within a bound */
	Rect clamp(Rect bound) {
		Rect r;
		r.pos.x = clampBetween(pos.x, bound.pos.x, bound.pos.x + bound.size.x);
		r.pos.y = clampBetween(pos.y, bound.pos.y, bound.pos.y + bound.size.y);
		r.size.x = rack::math::clamp(pos.x + size.x, bound.pos.x, bound.pos.x + bound.size.x) - r.pos.x;
		r.size.y = rack::math::clamp(pos.y + size.y, bound.pos.y, bound.pos.y + bound.size.y) - r.pos.y;
		return r;
	}
	/** Nudges the position to fix inside a bounding box */
	Rect nudge(Rect bound) {
		Rect r;
		r.size = size;
		r.pos.x = clampBetween(pos.x, bound.pos.x, bound.pos.x + bound.size.x - size.x);
		r.pos.y = clampBetween(pos.y, bound.pos.y, bound.pos.y + bound.size.y - size.y);
		return r;
	}
	/** Expands this Rect to contain `other` */
	Rect expand(Rect other) {
		Rect r;
		r.pos.x = std::min(pos.x, other.pos.x);
		r.pos.y = std::min(pos.y, other.pos.y);
		r.size.x = std::max(pos.x + size.x, other.pos.x + other.size.x) - r.pos.x;
		r.size.y = std::max(pos.y + size.y, other.pos.y + other.size.y) - r.pos.y;
		return r;
	}
	/** Returns a Rect with its position set to zero */
	Rect zeroPos() {
		return Rect(Vec(), size);
	}
	Rect grow(Vec delta) {
		Rect r;
		r.pos = pos.minus(delta);
		r.size = size.plus(delta.mult(2.f));
		return r;
	}
	Rect shrink(Vec delta) {
		Rect r;
		r.pos = pos.plus(delta);
		r.size = size.minus(delta.mult(2.f));
		return r;
	}
};


inline Vec Vec::clamp(Rect bound) {
	return Vec(
		rack::math::clamp(x, bound.pos.x, bound.pos.x + bound.size.x),
		rack::math::clamp(y, bound.pos.y, bound.pos.y + bound.size.y));
}

inline Vec Vec::clampBetween(Rect bound) {
	return Vec(
		rack::math::clampBetween(x, bound.pos.x, bound.pos.x + bound.size.x),
		rack::math::clampBetween(y, bound.pos.y, bound.pos.y + bound.size.y));
}

inline Vec Vec::clamp2(Rect bound) {return clampBetween(bound);}


} // namespace math
} // namespace rack
