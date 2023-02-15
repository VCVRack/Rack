#include <color.hpp>
#include <math.hpp>


namespace rack {
namespace color {


bool isEqual(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 4; i++) {
		if (a.rgba[i] != b.rgba[i])
			return false;
	}
	return true;
}

NVGcolor clamp(NVGcolor a) {
	for (int i = 0; i < 4; i++)
		a.rgba[i] = math::clamp(a.rgba[i], 0.f, 1.f);
	return a;
}

NVGcolor minus(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] -= b.rgba[i];
	return a;
}

NVGcolor plus(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] += b.rgba[i];
	return a;
}

NVGcolor mult(NVGcolor a, NVGcolor b) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] *= b.rgba[i];
	return a;
}

NVGcolor mult(NVGcolor a, float x) {
	for (int i = 0; i < 3; i++)
		a.rgba[i] *= x;
	return a;
}

NVGcolor lerp(NVGcolor a, NVGcolor b, float t) {
	NVGcolor c;
	for (int i = 0; i < 4; i++)
		c.rgba[i] = a.rgba[i] * (1 - t) + b.rgba[i] * t;
	return c;
}

/** Screen blending with alpha compositing */
NVGcolor screen(NVGcolor a, NVGcolor b) {
	if (a.a == 0.f)
		return b;
	if (b.a == 0.f)
		return a;

	a = mult(a, a.a);
	b = mult(b, b.a);
	NVGcolor c = minus(plus(a, b), mult(a, b));
	c.a = a.a + b.a - a.a * b.a;
	c = mult(c, 1.f / c.a);
	c = clamp(c);
	return c;
}

NVGcolor alpha(NVGcolor a, float alpha) {
	a.a *= alpha;
	return a;
}

NVGcolor fromHexString(std::string s) {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 255;
	// If only three hex pairs are given, this will leave `a` unset.
	sscanf(s.c_str(), "#%2hhx%2hhx%2hhx%2hhx", &r, &g, &b, &a);
	return nvgRGBA(r, g, b, a);
}

std::string toHexString(NVGcolor c) {
	uint8_t r = std::round(c.r * 255);
	uint8_t g = std::round(c.g * 255);
	uint8_t b = std::round(c.b * 255);
	uint8_t a = std::round(c.a * 255);
	if (a == 255)
		return string::f("#%02x%02x%02x", r, g, b);
	else
		return string::f("#%02x%02x%02x%02x", r, g, b, a);
}


} // namespace network
} // namespace rack
