#pragma once
#include "common.hpp"
#include "string.hpp"
#include <nanovg.h>


namespace rack {

/** Utilities for `NVGcolor`
*/
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


NVGcolor clamp(NVGcolor a);
NVGcolor minus(NVGcolor a, NVGcolor b);
NVGcolor plus(NVGcolor a, NVGcolor b);
NVGcolor mult(NVGcolor a, NVGcolor b);
NVGcolor mult(NVGcolor a, float x);
/** Screen blending with alpha compositing */
NVGcolor screen(NVGcolor a, NVGcolor b);
NVGcolor alpha(NVGcolor a, float alpha);
NVGcolor fromHexString(std::string s);
std::string toHexString(NVGcolor c);


} // namespace color
} // namespace rack
