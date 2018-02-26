#pragma once


#include "util/math.hpp"
#include "nanovg.h"


namespace rack {


inline NVGcolor colorClip(NVGcolor a) {
	for (int i = 0; i < 4; i++)
		a.rgba[i] = clamp(a.rgba[i], 0.f, 1.f);
	return a;
}

inline NVGcolor colorMinus(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 4; i++)
		a.rgba[i] -= b.rgba[i];
	return a;
}

inline NVGcolor colorPlus(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 4; i++)
		a.rgba[i] += b.rgba[i];
	return a;
}

inline NVGcolor colorMult(NVGcolor a, float x) {
	for (int i = 0; i < 4; i++)
		a.rgba[i] *= x;
	return a;
}


} // namespace rack
