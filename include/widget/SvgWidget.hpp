#pragma once
#include "widget/Widget.hpp"
#include "svg.hpp"


namespace rack {
namespace widget {


/** Draws an Svg */
struct SvgWidget : Widget {
	std::shared_ptr<Svg> svg;

	/** Sets the box size to the svg image size */
	void wrap();

	/** Sets and wraps the SVG */
	void setSvg(std::shared_ptr<Svg> svg);
	DEPRECATED void setSVG(std::shared_ptr<Svg> svg) {setSvg(svg);}

	void draw(const DrawArgs &args) override;
};


DEPRECATED typedef SvgWidget SVGWidget;


} // namespace widget
} // namespace rack
