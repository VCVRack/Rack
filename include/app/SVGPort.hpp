#pragma once
#include "app/common.hpp"
#include "app/PortWidget.hpp"
#include "widgets/FramebufferWidget.hpp"
#include "widgets/SVGWidget.hpp"
#include "app/CircularShadow.hpp"


namespace rack {


struct SVGPort : PortWidget {
	FramebufferWidget *fb;
	SVGWidget *sw;
	CircularShadow *shadow;

	SVGPort();
	void setSVG(std::shared_ptr<SVG> svg);
};


} // namespace rack
