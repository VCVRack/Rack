#pragma once
#include "ui/common.hpp"
#include "ui/QuantityWidget.hpp"


namespace rack {


struct ProgressBar : QuantityWidget {
	ProgressBar() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	void draw(NVGcontext *vg) override {
		float progress = rescale(value, minValue, maxValue, 0.0, 1.0);
		bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL, BND_DEFAULT, progress, getText().c_str(), NULL);
	}
};


} // namespace rack
