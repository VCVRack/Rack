#pragma once
#include "widgets/Widget.hpp"


namespace rack {


/** Draws an SVG */
struct SVGWidget : virtual Widget {
	std::shared_ptr<SVG> svg;

	/** Sets the box size to the svg image size */
	void wrap() {
		if (svg && svg->handle) {
			box.size = Vec(svg->handle->width, svg->handle->height);
		}
		else {
			box.size = Vec();
		}
	}

	/** Sets and wraps the SVG */
	void setSVG(std::shared_ptr<SVG> svg) {
		this->svg = svg;
		wrap();
	}

	void draw(NVGcontext *vg) override {
		if (svg && svg->handle) {
			svgDraw(vg, svg->handle);
		}
	}
};


} // namespace rack
