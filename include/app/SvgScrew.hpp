#pragma once
#include "common.hpp"
#include "widget/Widget.hpp"
#include "widget/FramebufferWidget.hpp"
#include "widget/SvgWidget.hpp"


namespace rack {
namespace app {


/** If you don't add these to your ModuleWidget, they will fall out of the rack... */
struct SvgScrew : widget::Widget {
	widget::FramebufferWidget *fb;
	widget::SvgWidget *sw;

	SvgScrew();
	void setSvg(std::shared_ptr<Svg> svg);
};


DEPRECATED typedef SvgScrew SVGScrew;


} // namespace app
} // namespace rack
