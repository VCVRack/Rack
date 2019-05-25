#pragma once
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <ui/common.hpp>
#include <ui/Button.hpp>


namespace rack {
namespace ui {


struct IconButton : Button {
	widget::FramebufferWidget *fw;
	widget::SvgWidget *sw;

	IconButton();
	void setSvg(std::shared_ptr<Svg> svg);
	DEPRECATED void setSVG(std::shared_ptr<Svg> svg) {setSvg(svg);}
};


} // namespace ui
} // namespace rack
