#pragma once
#include "util/common.hpp"
#include <math.h> // for global namespace functions
#include <cmath> // for std::isfinite, etc
#include <cstdlib> // for std::abs, etc


// Use a few standard math functions without std::
using std::isfinite;
using std::isinf;
using std::isnan;
using std::isnormal;


namespace rack {

////////////////////
// basic integer functions
////////////////////

/** Returns the minimum of `a` and `b` */
inline int min(int a, int b) {
	return (a < b) ? a : b;
}

/** Returns the maximum of `a` and `b` */
inline int max(int a, int b) {
	return (a > b) ? a : b;
}

/** Limits `x` between `a` and `b`
Assumes a <= b
*/
inline int clamp(int x, int a, int b) {
	return min(max(x, a), b);
}

/** Limits `x` between `a` and `b`
If a > b, switches the two values
*/
inline int clamp2(int x, int a, int b) {
	return clamp(x, min(a, b), max(a, b));
}

/** Euclidean modulus, always returns 0 <= mod < base for positive base.
*/
inline int eucmod(int a, int base) {
	int mod = a % base;
	return (mod >= 0) ? mod : mod + base;
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

inline bool ispow2(int n) {
	return n > 0 && (n & (n - 1)) == 0;
}

////////////////////
// basic float functions
////////////////////

/** Returns the minimum of `a` and `b` */
inline float min(float a, float b) {
	return (a < b) ? a : b;
}

/** Returns the maximum of `a` and `b` */
inline float max(float a, float b) {
	return (a > b) ? a : b;
}

/** Limits `x` between `a` and `b`
Assumes a <= b
*/
inline float clamp(float x, float a, float b) {
	return min(max(x, a), b);
}

/** Limits `x` between `a` and `b`
If a > b, switches the two values
*/
inline float clamp2(float x, float a, float b) {
	return clamp(x, min(a, b), max(a, b));
}

/** Returns 1.f for positive numbers and -1.f for negative numbers (including positive/negative zero) */
inline float sgn(float x) {
	return copysignf(1.0f, x);
}

inline float eucmod(float a, float base) {
	float mod = fmodf(a, base);
	return (mod >= 0.0f) ? mod : mod + base;
}

inline bool isNear(float a, float b, float epsilon = 1.0e-6f) {
	return fabsf(a - b) <= epsilon;
}

/** If the magnitude of x if less than eps, return 0 */
inline float chop(float x, float eps) {
	return (-eps < x && x < eps) ? 0.0f : x;
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
	float x, y;

	Vec() : x(0.0f), y(0.0f) {}
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
		return hypotf(x, y);
	}
	Vec min(Vec b) {
		return Vec(rack::min(x, b.x), rack::min(y, b.y));
	}
	Vec max(Vec b) {
		return Vec(rack::max(x, b.x), rack::max(y, b.y));
	}
	Vec round() {
		return Vec(roundf(x), roundf(y));
	}
	Vec floor() {
		return Vec(floorf(x), floorf(y));
	}
	Vec ceil() {
		return Vec(ceilf(x), ceilf(y));
	}
	bool isEqual(Vec b) {
		return x == b.x && y == b.y;
	}
	bool isZero() {
		return x == 0.0f && y == 0.0f;
	}
	bool isFinite() {
		return isfinite(x) && isfinite(y);
	}
	Vec clamp(Rect bound);
	Vec clamp2(Rect bound);
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
	Vec getTopRight() {
		return pos.plus(Vec(size.x, 0.0f));
	}
	Vec getBottomLeft() {
		return pos.plus(Vec(0.0f, size.y));
	}
	Vec getBottomRight() {
		return pos.plus(size);
	}
	/** Clamps the edges of the rectangle to fit within a bound */
	Rect clamp(Rect bound) {
		Rect r;
		r.pos.x = clamp2(pos.x, bound.pos.x, bound.pos.x + bound.size.x);
		r.pos.y = clamp2(pos.y, bound.pos.y, bound.pos.y + bound.size.y);
		r.size.x = rack::clamp(pos.x + size.x, bound.pos.x, bound.pos.x + bound.size.x) - r.pos.x;
		r.size.y = rack::clamp(pos.y + size.y, bound.pos.y, bound.pos.y + bound.size.y) - r.pos.y;
		return r;
	}
	/** Nudges the position to fix inside a bounding box */
	Rect nudge(Rect bound) {
		Rect r;
		r.size = size;
		r.pos.x = clamp2(pos.x, bound.pos.x, bound.pos.x + bound.size.x - size.x);
		r.pos.y = clamp2(pos.y, bound.pos.y, bound.pos.y + bound.size.y - size.y);
		return r;
	}
	/** Expands this Rect to contain `other` */
	Rect expand(Rect other) {
		Rect r;
		r.pos.x = min(pos.x, other.pos.x);
		r.pos.y = min(pos.y, other.pos.y);
		r.size.x = max(pos.x + size.x, other.pos.x + other.size.x) - r.pos.x;
		r.size.y = max(pos.y + size.y, other.pos.y + other.size.y) - r.pos.y;
		return r;
	}
	/** Returns a Rect with its position set to zero */
	Rect zeroPos() {
		Rect r;
		r.size = size;
		return r;
	}
	Rect grow(Vec delta) {
		Rect r;
		r.pos = pos.minus(delta);
		r.size = size.plus(delta.mult(2.f));
		return r;
	}
};


inline Vec Vec::clamp(Rect bound) {
	return Vec(
		rack::clamp(x, bound.pos.x, bound.pos.x + bound.size.x),
		rack::clamp(y, bound.pos.y, bound.pos.y + bound.size.y));
}

inline Vec Vec::clamp2(Rect bound) {
	return Vec(
		rack::clamp2(x, bound.pos.x, bound.pos.x + bound.size.x),
		rack::clamp2(y, bound.pos.y, bound.pos.y + bound.size.y));
}


////////////////////
// Deprecated functions
////////////////////

DEPRECATED inline int mini(int a, int b) {return min(a, b);}
DEPRECATED inline int maxi(int a, int b) {return max(a, b);}
DEPRECATED inline int clampi(int x, int min, int max) {return clamp(x, min, max);}
DEPRECATED inline int absi(int a) {return abs(a);}
DEPRECATED inline int eucmodi(int a, int base) {return eucmod(a, base);}
DEPRECATED inline int log2i(int n) {return log2(n);}
DEPRECATED inline bool ispow2i(int n) {return ispow2(n);}
DEPRECATED inline float absf(float x) {return fabsf(x);}
DEPRECATED inline float sgnf(float x) {return sgn(x);}
DEPRECATED inline float eucmodf(float a, float base) {return eucmod(a, base);}
DEPRECATED inline bool nearf(float a, float b, float epsilon = 1.0e-6f) {return isNear(a, b, epsilon);}
DEPRECATED inline float clampf(float x, float min, float max) {return clamp(x, min, max);}
DEPRECATED inline float clamp2f(float x, float min, float max) {return clamp2(x, min, max);}
DEPRECATED inline float chopf(float x, float eps) {return chop(x, eps);}
DEPRECATED inline float rescalef(float x, float a, float b, float yMin, float yMax) {return rescale(x, a, b, yMin, yMax);}
DEPRECATED inline float crossf(float a, float b, float frac) {return crossfade(a, b, frac);}
DEPRECATED inline float interpf(const float *p, float x) {return interpolateLinear(p, x);}
DEPRECATED inline void cmultf(float *cr, float *ci, float ar, float ai, float br, float bi) {return cmult(cr, ci, ar, ai, br, bi);}


} // namespace rack
