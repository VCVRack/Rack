#pragma once
#include "widgets/FramebufferWidget.hpp"
#include "widgets/SVGWidget.hpp"
#include "ui/common.hpp"
#include "ui/Button.hpp"


namespace rack {


struct IconButton : Button {
	FramebufferWidget *fw;
	SVGWidget *sw;

	IconButton();
	void setSVG(std::shared_ptr<SVG> svg);
};


} // namespace rack
