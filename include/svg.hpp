#pragma once
#include <memory>

#include <nanovg.h>
#include <nanosvg.h>

#include <common.hpp>


namespace rack {


struct Svg {
	NSVGimage* handle = NULL;
	/** Don't call this directly. Use `Svg::load()` for caching. */
	void loadFile(const std::string& filename);
	~Svg();

	/** Loads Svg from a cache. */
	static std::shared_ptr<Svg> load(const std::string& filename);
};

DEPRECATED typedef Svg SVG;


void svgDraw(NVGcontext* vg, NSVGimage* svg);


} // namespace rack
