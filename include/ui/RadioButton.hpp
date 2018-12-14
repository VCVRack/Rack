#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {


struct RadioButton : OpaqueWidget {
	BNDwidgetState state = BND_DEFAULT;
	Quantity *quantity = NULL;

	RadioButton() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	~RadioButton() {
		if (quantity)
			delete quantity;
	}

	void draw(NVGcontext *vg) override {
		std::string label;
		if (quantity)
			label = quantity->getLabel();
		bndRadioButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, label.c_str());
	}

	void onEnter(event::Enter &e) override {
		if (state != BND_ACTIVE)
			state = BND_HOVER;
	}

	void onLeave(event::Leave &e) override {
		if (state != BND_ACTIVE)
			state = BND_DEFAULT;
	}

	void onDragDrop(event::DragDrop &e) override {
		if (e.origin == this) {
			if (state == BND_ACTIVE) {
				state = BND_HOVER;
				if (quantity)
					quantity->setMin();
			}
			else {
				state = BND_ACTIVE;
				if (quantity)
					quantity->setMax();
			}

			event::Action eAction;
			onAction(eAction);
		}
	}
};


} // namespace rack
