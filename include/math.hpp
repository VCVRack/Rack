#pragma once

#include <math.h>


namespace rack {

/** Limits a value between a minimum and maximum
If min > max for some reason, returns min
*/
inline float clampf(float x, float min, float max) {
	if (x > max)
		x = max;
	if (x < min)
		x = min;
	return x;
}

/** If the magnitude of x if less than eps, return 0 */
inline float chopf(float x, float eps) {
	if (x < eps && x > -eps)
		return 0.0;
	return x;
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

inline float quadraticBipolar(float x) {
	float x2 = x*x;
	return x >= 0.0 ? x2 : -x2;
}

inline float cubic(float x) {
	// optimal with --fast-math
	return x*x*x;
}

inline float quarticBipolar(float x) {
	float x2 = x*x;
	float x4 = x2*x2;
	return x >= 0.0 ? x4 : -x4;
}

inline float quintic(float x) {
	// optimal with --fast-math
	return x*x*x*x*x;
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

/** Linearly interpolate an array `p` with index `x`
Assumes that the array at `p` is of length at least ceil(x)+1.
*/
inline float interpf(const float *p, float x) {
	int xi = x;
	float xf = x - xi;
	return crossf(p[xi], p[xi+1], xf);
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

	/** Returns whether this Rect contains another Rect, inclusive on the top/left, non-inclusive on the bottom/right */
	bool contains(Vec v) {
		return pos.x <= v.x && v.x < pos.x + size.x
			&& pos.y <= v.y && v.y < pos.y + size.y;
	}
	/** Returns whether this Rect overlaps with another Rect */
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
	/** Clamps the position to fix inside a bounding box */
	Rect clamp(Rect bound) {
		Rect r;
		r.size = size;
		r.pos.x = clampf(pos.x, bound.pos.x, bound.pos.x + bound.size.x - size.x);
		r.pos.y = clampf(pos.y, bound.pos.y, bound.pos.y + bound.size.y - size.y);
		return r;
	}
};

////////////////////
// Simple FFT implementation
////////////////////

// Derived from the Italian Wikipedia article for FFT
// https://it.wikipedia.org/wiki/Trasformata_di_Fourier_veloce
// If you need speed, use KissFFT, pffft, etc instead.

inline int log2i(int n) {
	int i = 0;
	while (n >>= 1) {
		i++;
	}
	return i;
}

inline bool isPowerOf2(int n) {
	return n > 0 && (n & (n-1)) == 0;
}

/*
inline int reverse(int N, int n)    //calculating revers number
{
  int j, p = 0;
  for(j = 1; j <= log2i(N); j++) {
    if(n & (1 << (log2i(N) - j)))
      p |= 1 << (j - 1);
  }
  return p;
}

inline void ordina(complex<double>* f1, int N) //using the reverse order in the array
{
  complex<double> f2[MAX];
  for(int i = 0; i < N; i++)
    f2[i] = f1[reverse(N, i)];
  for(int j = 0; j < N; j++)
    f1[j] = f2[j];
}

inline void transform(complex<double>* f, int N)
{
  ordina(f, N);    //first: reverse order
  complex<double> *W;
  W = (complex<double> *)malloc(N / 2 * sizeof(complex<double>));
  W[1] = polar(1., -2. * M_PI / N);
  W[0] = 1;
  for(int i = 2; i < N / 2; i++)
    W[i] = pow(W[1], i);
  int n = 1;
  int a = N / 2;
  for(int j = 0; j < log2i(N); j++) {
    for(int i = 0; i < N; i++) {
      if(!(i & n)) {
        complex<double> temp = f[i];
        complex<double> Temp = W[(i * a) % (n * a)] * f[i + n];
        f[i] = temp + Temp;
        f[i + n] = temp - Temp;
      }
    }
    n *= 2;
    a = a / 2;
  }
}

inline void FFT(complex<double>* f, int N, double d)
{
  transform(f, N);
  for(int i = 0; i < N; i++)
    f[i] *= d; //multiplying by step
}
*/



} // namespace rack
