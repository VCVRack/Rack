#pragma once

#include "widgets.hpp"
#include "blendish.h"


namespace rack {


struct WindowWidget : OpaqueWidget {
	std::string title;

	void draw(NVGcontext *vg) override {
		bndNodeBackground(vg, 0.0, 0.0, box.size.x, box.size.y, BND_DEFAULT, -1, title.c_str(), bndGetTheme()->backgroundColor);
		Widget::draw(vg);
	}

	void on(event::DragMove &e) override {
		box.pos = box.pos.plus(e.mouseDelta);
	}
};


} // namespace rack
