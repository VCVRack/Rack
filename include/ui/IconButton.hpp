#pragma once
#include "widget/FramebufferWidget.hpp"
#include "widget/SVGWidget.hpp"
#include "ui/common.hpp"
#include "ui/Button.hpp"


namespace rack {
namespace ui {


struct IconButton : Button {
	widget::FramebufferWidget *fw;
	widget::SVGWidget *sw;

	IconButton();
	void setSVG(std::shared_ptr<SVG> svg);
};


} // namespace ui
} // namespace rack
