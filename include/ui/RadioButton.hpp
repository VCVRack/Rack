#pragma once

#include "widgets.hpp"
#include "blendish.h"


namespace rack {


struct RadioButton : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	RadioButton() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	void draw(NVGcontext *vg) override {
		bndRadioButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, value == 0.0 ? state : BND_ACTIVE, -1, label.c_str());
	}

	void on(event::Enter &e) override {
		state = BND_HOVER;
	}

	void on(event::Leave &e) override {
		state = BND_DEFAULT;
	}

	void on(event::DragDrop &e) override {
		if (e.origin == this) {
			if (value)
				setValue(0.0);
			else
				setValue(1.0);

			event::Action eAction;
			handleEvent(eAction);
		}
	}
};


} // namespace rack
