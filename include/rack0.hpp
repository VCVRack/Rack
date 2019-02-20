#pragma once
#include "rack.hpp"


namespace rack {

////////////////////
// common
////////////////////

/** Deprecated lowercase macro */
#define defer(...) DEFER(__VA_ARGS__)

////////////////////
// math
////////////////////

using namespace math;

DEPRECATED inline int min(int a, int b) {return std::min(a, b);}
DEPRECATED inline int max(int a, int b) {return std::max(a, b);}
DEPRECATED inline int eucmod(int a, int base) {return eucMod(a, base);}
DEPRECATED inline bool ispow2(int n) {return isPow2(n);}
DEPRECATED inline int clamp2(int x, int a, int b) {return clampSafe(x, a, b);}
DEPRECATED inline float min(float a, float b) {return std::min(a, b);}
DEPRECATED inline float max(float a, float b) {return std::max(a, b);}
DEPRECATED inline float eucmod(float a, float base) {return eucMod(a, base);}
DEPRECATED inline float clamp2(float x, float a, float b) {return clampSafe(x, a, b);}

DEPRECATED inline int mini(int a, int b) {return std::min(a, b);}
DEPRECATED inline int maxi(int a, int b) {return std::max(a, b);}
DEPRECATED inline int clampi(int x, int min, int max) {return math::clamp(x, min, max);}
DEPRECATED inline int absi(int a) {return std::fabs(a);}
DEPRECATED inline int eucmodi(int a, int base) {return eucMod(a, base);}
DEPRECATED inline int log2i(int n) {return math::log2(n);}
DEPRECATED inline bool ispow2i(int n) {return isPow2(n);}
DEPRECATED inline float absf(float x) {return std::fabs(x);}
DEPRECATED inline float sgnf(float x) {return sgn(x);}
DEPRECATED inline float eucmodf(float a, float base) {return eucMod(a, base);}
DEPRECATED inline bool nearf(float a, float b, float epsilon = 1.0e-6f) {return math::isNear(a, b, epsilon);}
DEPRECATED inline float clampf(float x, float min, float max) {return math::clamp(x, min, max);}
DEPRECATED inline float clamp2f(float x, float min, float max) {return clampSafe(x, min, max);}
DEPRECATED inline float chopf(float x, float eps) {return chop(x, eps);}
DEPRECATED inline float rescalef(float x, float a, float b, float yMin, float yMax) {return math::rescale(x, a, b, yMin, yMax);}
DEPRECATED inline float crossf(float a, float b, float frac) {return crossfade(a, b, frac);}
DEPRECATED inline float interpf(const float *p, float x) {return interpolateLinear(p, x);}
DEPRECATED inline void cmultf(float *cr, float *ci, float ar, float ai, float br, float bi) {return complexMult(cr, ci, ar, ai, br, bi);}

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

DEPRECATED inline std::string assetGlobal(std::string filename) {return asset::system(filename);}
DEPRECATED inline std::string assetLocal(std::string filename) {return asset::user(filename);}
DEPRECATED inline std::string assetPlugin(Plugin *plugin, std::string filename) {return asset::plugin(plugin, filename);}

////////////////////
// color
////////////////////

DEPRECATED inline NVGcolor colorClip(NVGcolor a) {return color::clamp(a);}
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

DEPRECATED static const NVGcolor COLOR_BLACK_TRANSPARENT = componentlibrary::SCHEME_BLACK_TRANSPARENT;
DEPRECATED static const NVGcolor COLOR_BLACK = componentlibrary::SCHEME_BLACK;
DEPRECATED static const NVGcolor COLOR_WHITE = componentlibrary::SCHEME_WHITE;
DEPRECATED static const NVGcolor COLOR_RED = componentlibrary::SCHEME_RED;
DEPRECATED static const NVGcolor COLOR_ORANGE = componentlibrary::SCHEME_ORANGE;
DEPRECATED static const NVGcolor COLOR_YELLOW = componentlibrary::SCHEME_YELLOW;
DEPRECATED static const NVGcolor COLOR_GREEN = componentlibrary::SCHEME_GREEN;
DEPRECATED static const NVGcolor COLOR_CYAN = componentlibrary::SCHEME_CYAN;
DEPRECATED static const NVGcolor COLOR_BLUE = componentlibrary::SCHEME_BLUE;
DEPRECATED static const NVGcolor COLOR_PURPLE = componentlibrary::SCHEME_PURPLE;
DEPRECATED static const NVGcolor COLOR_LIGHT_PANEL = componentlibrary::SCHEME_LIGHT_GRAY;
DEPRECATED static const NVGcolor COLOR_DARK_PANEL = componentlibrary::SCHEME_DARK_GRAY;

////////////////////
// helpers
////////////////////

/** Use createWidget() instead */
template <class TScrew>
DEPRECATED TScrew *createScrew(math::Vec pos) {
	return createWidget<TScrew>(pos);
}

/** Use createParam(pos, module, paramId) and set the Param properties in your Module constructor */
template <class TParamWidget>
DEPRECATED TParamWidget *createParam(math::Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue) {
	TParamWidget *o = createParam<TParamWidget>(pos, module, paramId);
	if (module) {
		module->params[paramId].config(minValue, maxValue, defaultValue);
	}
	return o;
}

/** Use createInput() and createOutput() without the `type` variable */
template <class TPortWidget>
DEPRECATED TPortWidget *createPort(math::Vec pos, PortWidget::Type type, Module *module, int inputId) {
	TPortWidget *o = new TPortWidget;
	o->box.pos = pos;
	o->type = type;
	o->module = module;
	o->portId = inputId;
	return o;
}

////////////////////
// engine
////////////////////

DEPRECATED inline float engineGetSampleRate() {
	return APP->engine->getSampleRate();
}

DEPRECATED inline float engineGetSampleTime() {
	return APP->engine->getSampleTime();
}

////////////////////
// dsp
////////////////////

using namespace dsp;

DEPRECATED inline float gainToDb(float gain) {
	return dsp::amplitudeToDb(gain);
}

DEPRECATED inline float dbToGain(float db) {
	return dsp::dbToAmplitude(db);
}


} // namespace rack
