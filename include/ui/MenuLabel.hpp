#pragma once
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"
#include "app.hpp"


namespace rack {


struct MenuLabel : MenuEntry {
	std::string text;

	void draw(NVGcontext *vg) override {
		bndMenuLabel(vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
	}

	void step() override {
		// Add 10 more pixels because Retina measurements are sometimes too small
		const float rightPadding = 10.0;
		// HACK use app()->window->vg from the window.
		box.size.x = bndLabelWidth(app()->window->vg, -1, text.c_str()) + rightPadding;
		Widget::step();
	}
};


} // namespace rack
