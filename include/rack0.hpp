#pragma once

#include "rack.hpp"
#include "componentlibrary.hpp"


namespace rack {

////////////////////
// common
////////////////////

/** Deprecated lowercase macro */
#define defer(...) DEFER(__VA_ARGS__)

////////////////////
// math
////////////////////

DEPRECATED inline int min(int a, int b) {return std::min(a, b);}
DEPRECATED inline int max(int a, int b) {return std::max(a, b);}
DEPRECATED inline int eucmod(int a, int base) {return eucMod(a, base);}
DEPRECATED inline bool ispow2(int n) {return isPow2(n);}
DEPRECATED inline int clamp2(int x, int a, int b) {return clampBetween(x, a, b);}
DEPRECATED inline float min(float a, float b) {return std::min(a, b);}
DEPRECATED inline float max(float a, float b) {return std::max(a, b);}
DEPRECATED inline float eucmod(float a, float base) {return eucMod(a, base);}
DEPRECATED inline float clamp2(float x, float a, float b) {return clampBetween(x, a, b);}

DEPRECATED inline int mini(int a, int b) {return std::min(a, b);}
DEPRECATED inline int maxi(int a, int b) {return std::max(a, b);}
DEPRECATED inline int clampi(int x, int min, int max) {return clamp(x, min, max);}
DEPRECATED inline int absi(int a) {return std::abs(a);}
DEPRECATED inline int eucmodi(int a, int base) {return eucMod(a, base);}
DEPRECATED inline int log2i(int n) {return log2(n);}
DEPRECATED inline bool ispow2i(int n) {return isPow2(n);}
DEPRECATED inline float absf(float x) {return std::abs(x);}
DEPRECATED inline float sgnf(float x) {return sgn(x);}
DEPRECATED inline float eucmodf(float a, float base) {return eucMod(a, base);}
DEPRECATED inline bool nearf(float a, float b, float epsilon = 1.0e-6f) {return isNear(a, b, epsilon);}
DEPRECATED inline float clampf(float x, float min, float max) {return clamp(x, min, max);}
DEPRECATED inline float clamp2f(float x, float min, float max) {return clampBetween(x, min, max);}
DEPRECATED inline float chopf(float x, float eps) {return chop(x, eps);}
DEPRECATED inline float rescalef(float x, float a, float b, float yMin, float yMax) {return rescale(x, a, b, yMin, yMax);}
DEPRECATED inline float crossf(float a, float b, float frac) {return crossfade(a, b, frac);}
DEPRECATED inline float interpf(const float *p, float x) {return interpolateLinear(p, x);}
DEPRECATED inline void cmultf(float *cr, float *ci, float ar, float ai, float br, float bi) {return cmult(cr, ci, ar, ai, br, bi);}

////////////////////
// random
////////////////////

DEPRECATED inline float randomu32() {return random::u32();}
DEPRECATED inline float randomu64() {return random::u64();}
DEPRECATED inline float randomUniform() {return random::uniform();}
DEPRECATED inline float randomNormal() {return random::normal();}
DEPRECATED inline float randomf() {return random::uniform();}

////////////////////
// logger
////////////////////

/** Deprecated lowercase log functions */
#define debug(...) DEBUG(__VA_ARGS__)
#define info(...) INFO(__VA_ARGS__)
#define warn(...) WARN(__VA_ARGS__)
#define fatal(...) FATAL(__VA_ARGS__)

////////////////////
// asset
////////////////////

DEPRECATED inline std::string assetGlobal(std::string filename) {return asset::global(filename);}
DEPRECATED inline std::string assetLocal(std::string filename) {return asset::local(filename);}
DEPRECATED inline std::string assetPlugin(Plugin *plugin, std::string filename) {return asset::plugin(plugin, filename);}

////////////////////
// color
////////////////////

DEPRECATED inline NVGcolor colorClip(NVGcolor a) {return color::clip(a);}
DEPRECATED inline NVGcolor colorMinus(NVGcolor a, NVGcolor b) {return color::minus(a, b);}
DEPRECATED inline NVGcolor colorPlus(NVGcolor a, NVGcolor b) {return color::plus(a, b);}
DEPRECATED inline NVGcolor colorMult(NVGcolor a, NVGcolor b) {return color::mult(a, b);}
DEPRECATED inline NVGcolor colorMult(NVGcolor a, float x) {return color::mult(a, x);}
DEPRECATED inline NVGcolor colorScreen(NVGcolor a, NVGcolor b) {return color::screen(a, b);}
DEPRECATED inline NVGcolor colorAlpha(NVGcolor a, float alpha) {return color::alpha(a, alpha);}
DEPRECATED inline NVGcolor colorFromHexString(std::string s) {return color::fromHexString(s);}
DEPRECATED inline std::string colorToHexString(NVGcolor c) {return color::toHexString(c);}

////////////////////
// componentlibrary
////////////////////

DEPRECATED static const NVGcolor COLOR_BLACK_TRANSPARENT = SCHEME_BLACK_TRANSPARENT;
DEPRECATED static const NVGcolor COLOR_BLACK = SCHEME_BLACK;
DEPRECATED static const NVGcolor COLOR_WHITE = SCHEME_WHITE;
DEPRECATED static const NVGcolor COLOR_RED = SCHEME_RED;
DEPRECATED static const NVGcolor COLOR_ORANGE = SCHEME_ORANGE;
DEPRECATED static const NVGcolor COLOR_YELLOW = SCHEME_YELLOW;
DEPRECATED static const NVGcolor COLOR_GREEN = SCHEME_GREEN;
DEPRECATED static const NVGcolor COLOR_CYAN = SCHEME_CYAN;
DEPRECATED static const NVGcolor COLOR_BLUE = SCHEME_BLUE;
DEPRECATED static const NVGcolor COLOR_PURPLE = SCHEME_PURPLE;
DEPRECATED static const NVGcolor COLOR_LIGHT_PANEL = SCHEME_LIGHT_PANEL;
DEPRECATED static const NVGcolor COLOR_DARK_PANEL = SCHEME_DARK_PANEL;

////////////////////
// helpers
////////////////////

template <class TScrew>
DEPRECATED TScrew *createScrew(Vec pos) {
	return createWidget<TScrew>(pos);
}


} // namespace rack
