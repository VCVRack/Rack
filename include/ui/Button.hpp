#pragma once

#include "widgets.hpp"
#include "blendish.h"


namespace rack {


struct Button : OpaqueWidget {
	std::string text;
	BNDwidgetState state = BND_DEFAULT;

	Button() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	void draw(NVGcontext *vg) override {
		bndToolButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
		Widget::draw(vg);
	}

	void on(event::Enter &e) override {
		state = BND_HOVER;
	}

	void on(event::Leave &e) override {
		state = BND_DEFAULT;
	}

	void on(event::DragStart &e) override {
		state = BND_ACTIVE;
	}

	void on(event::DragEnd &e) override {
		state = BND_HOVER;
	}

	void on(event::DragDrop &e) override {
		if (e.origin == this) {
			event::Action eAction;
			handleEvent(eAction);
		}
	}
};


} // namespace rack
