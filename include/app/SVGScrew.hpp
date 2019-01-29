#pragma once
#include "common.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SVGWidget.hpp"


namespace rack {
namespace app {


/** If you don't add these to your ModuleWidget, they will fall out of the rack... */
struct SVGScrew : widget::FramebufferWidget {
	widget::SVGWidget *sw;

	SVGScrew();
};


} // namespace app
} // namespace rack
