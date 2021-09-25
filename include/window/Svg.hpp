#pragma once
#include <memory>

#include <nanovg.h>
#include <nanosvg.h>

#include <common.hpp>
#include <math.hpp>


namespace rack {
namespace window {


/** Arbitrary DPI, standardized for Rack. */
static const float SVG_DPI = 75.f;
static const float MM_PER_IN = 25.4f;


/** Converts inch measurements to pixels */
inline float in2px(float in) {
	return in * SVG_DPI;
}

inline math::Vec in2px(math::Vec in) {
	return in.mult(SVG_DPI);
}

/** Converts millimeter measurements to pixels */
inline float mm2px(float mm) {
	return mm * (SVG_DPI / MM_PER_IN);
}

inline math::Vec mm2px(math::Vec mm) {
	return mm.mult(SVG_DPI / MM_PER_IN);
}


/** Scalable Vector Graphics document */
struct Svg {
	NSVGimage* handle = NULL;

	~Svg();
	/** Don't call this directly. Use `Svg::load()` for caching. */
	void loadFile(const std::string& filename);
	/** Loads SVG data from a string. */
	void loadString(const std::string& str);
	/** Returns the SVG page size in pixels. */
	math::Vec getSize();
	int getNumShapes();
	int getNumPaths();
	int getNumPoints();
	void draw(NVGcontext* vg);

	/** Loads Svg from a cache. */
	static std::shared_ptr<Svg> load(const std::string& filename);
};

DEPRECATED typedef Svg SVG;


void svgDraw(NVGcontext* vg, NSVGimage* svg);


} // namespace window
} // namespace rack
