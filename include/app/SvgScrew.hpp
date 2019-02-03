#pragma once
#include "common.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SvgWidget.hpp"


namespace rack {
namespace app {


/** If you don't add these to your ModuleWidget, they will fall out of the rack... */
struct SvgScrew : widget::FramebufferWidget {
	widget::SvgWidget *sw;

	SvgScrew();
};


DEPRECATED typedef SvgScrew SVGScrew;


} // namespace app
} // namespace rack
