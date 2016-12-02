#pragma once

#include <stdint.h>
#include <math.h>


namespace rack {

////////////////////
// Math
////////////////////

inline float clampf(float x, float min, float max) {
	return fmaxf(min, fminf(max, x));
}

inline float mapf(float x, float xMin, float xMax, float yMin, float yMax) {
	return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
}

inline float crossf(float a, float b, float frac) {
	return (1.0 - frac) * a + frac * b;
}

inline int mini(int a, int b) {
	return a < b ? a : b;
}

inline int maxi(int a, int b) {
	return a > b ? a : b;
}

// Euclidean modulus, always returns 0 <= mod < base for positive base
// Assumes this architecture's division is non-Euclidean
inline int eucMod(int a, int base) {
	int mod = a % base;
	return mod < 0 ? mod + base : mod;
}

inline float getf(const float *p, float v = 0.0) {
	return p ? *p : v;
}

inline void setf(float *p, float v) {
	if (p)
		*p = v;
}

// Linearly interpolate an array `p` with index `x`
inline float interpf(float *p, float x) {
	int i = x;
	x -= i;
	return crossf(p[i], p[i+1], x);
}

////////////////////
// RNG
////////////////////

uint64_t randomi64(void);

// Return a uniform random number on [0.0, 1.0)
inline float randomf(void) {
	return (float)randomi64() / UINT64_MAX;
}

////////////////////
// 2D float vector
////////////////////

struct Vec {
	float x, y;

	Vec() : x(0.0), y(0.0) {}
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
	Vec div(float s) {
		return Vec(x / s, y / s);
	}
	float dot(Vec b) {
		return x * b.x + y * b.y;
	}
	float norm() {
		return hypotf(x, y);
	}
	Vec min(Vec b) {
		return Vec(fminf(x, b.x), fminf(y, b.y));
	}
	Vec max(Vec b) {
		return Vec(fmaxf(x, b.x), fmaxf(y, b.y));
	}
	Vec round() {
		return Vec(roundf(x), roundf(y));
	}
};


struct Rect {
	Vec pos;
	Vec size;

	Rect() {}
	Rect(Vec pos, Vec size) : pos(pos), size(size) {}

	bool contains(Vec v) {
		return pos.x <= v.x && v.x < pos.x + size.x
			&& pos.y <= v.y && v.y < pos.y + size.y;
	}
	bool intersects(Rect r) {
		return (pos.x + size.x > r.pos.x && r.pos.x + r.size.x > pos.x)
			&& (pos.y + size.y > r.pos.y && r.pos.y + r.size.y > pos.y);
	}
	Vec getCenter() {
		return pos.plus(size.mult(0.5));
	}
	Vec getTopRight() {
		return pos.plus(Vec(size.x, 0.0));
	}
	Vec getBottomLeft() {
		return pos.plus(Vec(0.0, size.y));
	}
	Vec getBottomRight() {
		return pos.plus(size);
	}
};


} // namespace rack
