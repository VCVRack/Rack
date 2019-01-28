#pragma once
#include "widgets/Widget.hpp"
#include "svg.hpp"


namespace rack {


/** Draws an SVG */
struct SVGWidget : Widget {
	std::shared_ptr<SVG> svg;

	/** Sets the box size to the svg image size */
	void wrap() {
		if (svg && svg->handle) {
			box.size = math::Vec(svg->handle->width, svg->handle->height);
		}
		else {
			box.size = math::Vec();
		}
	}

	/** Sets and wraps the SVG */
	void setSVG(std::shared_ptr<SVG> svg) {
		this->svg = svg;
		wrap();
	}

	void draw(const DrawContext &ctx) override {
		if (svg && svg->handle) {
			svgDraw(ctx.vg, svg->handle);
		}
	}
};


} // namespace rack
