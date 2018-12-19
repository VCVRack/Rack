#pragma once
#include "widgets/FramebufferWidget.hpp"
#include "widgets/SVGWidget.hpp"
#include "ui/common.hpp"
#include "ui/Button.hpp"


namespace rack {


struct IconButton : Button {
	FramebufferWidget *fw;
	SVGWidget *sw;

	IconButton() {
		box.size.x = BND_TOOL_WIDTH;

		fw = new FramebufferWidget;
		fw->oversample = 2;
		addChild(fw);

		sw = new SVGWidget;
		sw->box.pos = math::Vec(2, 2);
		fw->addChild(sw);
	}

	void setSVG(std::shared_ptr<SVG> svg) {
		sw->setSVG(svg);
		fw->dirty = true;
	}
};


} // namespace rack
