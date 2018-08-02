#pragma once


#include "util/common.hpp"
#include "nanovg.h"


namespace rack {
namespace color {


static const NVGcolor BLACK_TRANSPARENT = nvgRGBA(0x00, 0x00, 0x00, 0x00);
static const NVGcolor BLACK = nvgRGB(0x00, 0x00, 0x00);
static const NVGcolor WHITE = nvgRGB(0xff, 0xff, 0xff);
static const NVGcolor WHITE_TRANSPARENT = nvgRGB(0xff, 0xff, 0xff);
static const NVGcolor RED = nvgRGB(0xff, 0x00, 0x00);
static const NVGcolor GREEN = nvgRGB(0x00, 0xff, 0x00);
static const NVGcolor BLUE = nvgRGB(0x00, 0x00, 0xff);
static const NVGcolor YELLOW = nvgRGB(0xff, 0xff, 0x00);
static const NVGcolor MAGENTA = nvgRGB(0xff, 0x00, 0xff);
static const NVGcolor CYAN = nvgRGB(0x00, 0xff, 0xff);


inline NVGcolor clip(NVGcolor a) {
	for (int i = 0; i < 4; i++)
		a.rgba[i] = math::clamp(a.rgba[i], 0.f, 1.f);
	return a;
}

inline NVGcolor minus(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] -= b.rgba[i];
	return a;
}

inline NVGcolor plus(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] += b.rgba[i];
	return a;
}

inline NVGcolor mult(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] *= b.rgba[i];
	return a;
}

inline NVGcolor mult(NVGcolor a, float x) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] *= x;
	return a;
}

/** Screen blending with alpha compositing */
inline NVGcolor screen(NVGcolor a, NVGcolor b) {
	if (a.a == 0.0)
		return b;
	if (b.a == 0.0)
		return a;

	a = mult(a, a.a);
	b = mult(b, b.a);
	NVGcolor c = minus(plus(a, b), mult(a, b));
	c.a = a.a + b.a - a.a * b.a;
	c = mult(c, 1.f / c.a);
	c = clip(c);
	return c;
}

inline NVGcolor alpha(NVGcolor a, float alpha) {
	a.a *= alpha;
	return a;
}

inline NVGcolor fromHexString(std::string s) {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 255;
	sscanf(s.c_str(), "#%2hhx%2hhx%2hhx%2hhx", &r, &g, &b, &a);
	return nvgRGBA(r, g, b, a);
}

inline std::string toHexString(NVGcolor c) {
	uint8_t r = std::round(c.r * 255);
	uint8_t g = std::round(c.g * 255);
	uint8_t b = std::round(c.b * 255);
	uint8_t a = std::round(c.a * 255);
	if (a == 255)
		return stringf("#%02x%02x%02x", r, g, b);
	else
		return stringf("#%02x%02x%02x%02x", r, g, b, a);
}


} // namespace color
} // namespace rack
