#pragma once
#include <nanovg.h>

#include <common.hpp>
#include <string.hpp>


namespace rack {
/** Utilities for `NVGcolor` */
namespace color {


static const NVGcolor BLACK_TRANSPARENT = nvgRGBA(0x00, 0x00, 0x00, 0x00);
static const NVGcolor WHITE_TRANSPARENT = nvgRGBA(0xff, 0xff, 0xff, 0x00);

// All corners of the RGB cube and nothing else
static const NVGcolor BLACK = nvgRGB(0x00, 0x00, 0x00);
static const NVGcolor RED = nvgRGB(0xff, 0x00, 0x00);
static const NVGcolor GREEN = nvgRGB(0x00, 0xff, 0x00);
static const NVGcolor BLUE = nvgRGB(0x00, 0x00, 0xff);
static const NVGcolor CYAN = nvgRGB(0x00, 0xff, 0xff);
static const NVGcolor MAGENTA = nvgRGB(0xff, 0x00, 0xff);
static const NVGcolor YELLOW = nvgRGB(0xff, 0xff, 0x00);
static const NVGcolor WHITE = nvgRGB(0xff, 0xff, 0xff);


bool isEqual(NVGcolor a, NVGcolor b);
/** Limits color components between 0 and 1. */
NVGcolor clamp(NVGcolor a);
/** Subtracts color components elementwise. */
NVGcolor minus(NVGcolor a, NVGcolor b);
/** Adds color components elementwise. */
NVGcolor plus(NVGcolor a, NVGcolor b);
/** Multiplies color components elementwise. */
NVGcolor mult(NVGcolor a, NVGcolor b);
NVGcolor mult(NVGcolor a, float x);
/** Interpolates RGBA color values. */
NVGcolor lerp(NVGcolor a, NVGcolor b, float t);
/** Screen blending with alpha compositing */
NVGcolor screen(NVGcolor a, NVGcolor b);
/** Multiplies alpha value. */
NVGcolor alpha(NVGcolor a, float alpha);
/** Converts from color hex string of the form "#RRGGBB" or "#RRGGBBAA".
Returns WHITE on error.
*/
NVGcolor fromHexString(std::string s);
std::string toHexString(NVGcolor c);


} // namespace color
} // namespace rack
