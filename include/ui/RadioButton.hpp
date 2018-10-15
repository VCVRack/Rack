#pragma once
#include "common.hpp"


namespace rack {


struct RadioButton : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	RadioButton() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	void draw(NVGcontext *vg) override {
		bndRadioButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, value == 0.0 ? state : BND_ACTIVE, -1, label.c_str());
	}

	void onEnter(event::Enter &e) override {
		state = BND_HOVER;
	}

	void onLeave(event::Leave &e) override {
		state = BND_DEFAULT;
	}

	void onDragDrop(event::DragDrop &e) override {
		if (e.origin == this) {
			if (value)
				setValue(0.0);
			else
				setValue(1.0);

			event::Action eAction;
			onAction(eAction);
		}
	}
};


} // namespace rack
