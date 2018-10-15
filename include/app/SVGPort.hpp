#pragma once
#include "common.hpp"
#include "Port.hpp"


namespace rack {


struct SVGPort : Port, FramebufferWidget {
	SVGWidget *background;
	CircularShadow *shadow;

	SVGPort();
	void setSVG(std::shared_ptr<SVG> svg);
	void draw(NVGcontext *vg) override;
};


} // namespace rack
