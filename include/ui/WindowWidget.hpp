#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {


struct WindowWidget : OpaqueWidget {
	std::string title;

	void draw(NVGcontext *vg) override {
		bndNodeBackground(vg, 0.0, 0.0, box.size.x, box.size.y, BND_DEFAULT, -1, title.c_str(), bndGetTheme()->backgroundColor);
		Widget::draw(vg);
	}

	void onDragMove(event::DragMove &e) override {
		box.pos = box.pos.plus(e.mouseDelta);
	}
};


} // namespace rack
