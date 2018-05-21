#include "util/color.hpp"


namespace rack {


NVGcolor colorFromHexString(std::string s) {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 255;
	sscanf(s.c_str(), "#%2hhx%2hhx%2hhx%2hhx", &r, &g, &b, &a);
	return nvgRGBA(r, g, b, a);
}

std::string colorToHexString(NVGcolor c) {
	uint8_t r = roundf(c.r * 255);
	uint8_t g = roundf(c.g * 255);
	uint8_t b = roundf(c.b * 255);
	uint8_t a = roundf(c.a * 255);
	if (a == 255)
		return stringf("#%02x%02x%02x", r, g, b);
	else
		return stringf("#%02x%02x%02x%02x", r, g, b, a);
}


} // namespace rack
