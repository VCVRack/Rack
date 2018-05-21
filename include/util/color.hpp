#pragma once


#include "util/common.hpp"
#include "nanovg.h"


namespace rack {


// TODO Make these non-inline in Rack v1


inline NVGcolor colorClip(NVGcolor a) {
	for (int i = 0; i < 4; i++)
		a.rgba[i] = clamp(a.rgba[i], 0.f, 1.f);
	return a;
}

inline NVGcolor colorMinus(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] -= b.rgba[i];
	return a;
}

inline NVGcolor colorPlus(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] += b.rgba[i];
	return a;
}

inline NVGcolor colorMult(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] *= b.rgba[i];
	return a;
}

inline NVGcolor colorMult(NVGcolor a, float x) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] *= x;
	return a;
}

/** Screen blending with alpha compositing */
inline NVGcolor colorScreen(NVGcolor a, NVGcolor b) {
	if (a.a == 0.0)
		return b;
	if (b.a == 0.0)
		return a;

	a = colorMult(a, a.a);
	b = colorMult(b, b.a);
	NVGcolor c = colorMinus(colorPlus(a, b), colorMult(a, b));
	c.a = a.a + b.a - a.a * b.a;
	c = colorMult(c, 1.f / c.a);
	c = colorClip(c);
	return c;
}

inline NVGcolor colorAlpha(NVGcolor a, float alpha) {
	a.a *= alpha;
	return a;
}

NVGcolor colorFromHexString(std::string s);
std::string colorToHexString(NVGcolor c);


} // namespace rack
