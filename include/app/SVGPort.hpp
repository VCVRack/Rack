#pragma once
#include "app/common.hpp"
#include "app/PortWidget.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SVGWidget.hpp"
#include "app/CircularShadow.hpp"


namespace rack {
namespace app {


struct SVGPort : PortWidget {
	widget::FramebufferWidget *fb;
	widget::SVGWidget *sw;
	CircularShadow *shadow;

	SVGPort();
	void setSVG(std::shared_ptr<SVG> svg);
};


} // namespace app
} // namespace rack
