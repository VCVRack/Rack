#pragma once
#include "ui/common.hpp"


namespace rack {


struct ProgressBar : VirtualWidget {
	Quantity *quantity = NULL;

	ProgressBar() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	~ProgressBar() {
		if (quantity)
			delete quantity;
	}

	void draw(NVGcontext *vg) override {
		float progress = quantity ? quantity->getScaledValue() : 0.f;
		std::string text = quantity ? quantity->getString() : "";
		bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL, BND_DEFAULT, progress, text.c_str(), NULL);
	}
};


} // namespace rack
