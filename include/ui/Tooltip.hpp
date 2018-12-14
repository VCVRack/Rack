#pragma once
#include "widgets/Widget.hpp"
#include "ui/common.hpp"


namespace rack {


struct Tooltip : VirtualWidget {
	std::string text;

	void draw(NVGcontext *vg) override {
		// Wrap size to contents
		box.size.x = bndLabelWidth(vg, -1, text.c_str()) + 10.0;
		box.size.y = bndLabelHeight(vg, -1, text.c_str(), INFINITY);

		bndTooltipBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
		bndMenuLabel(vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
		Widget::draw(vg);
	}
};


} // namespace rack
