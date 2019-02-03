#pragma once
#include "app/common.hpp"
#include "app/PortWidget.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SvgWidget.hpp"
#include "app/CircularShadow.hpp"


namespace rack {
namespace app {


struct SvgPort : PortWidget {
	widget::FramebufferWidget *fb;
	widget::SvgWidget *sw;
	CircularShadow *shadow;

	SvgPort();
	void setSvg(std::shared_ptr<Svg> svg);
	DEPRECATED void setSVG(std::shared_ptr<Svg> svg) {setSvg(svg);}
};


DEPRECATED typedef SvgPort SVGPort;


} // namespace app
} // namespace rack
