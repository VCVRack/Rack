#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"
#include "ui/Quantity.hpp"


namespace rack {


struct Button : OpaqueWidget {
	std::string text;
	BNDwidgetState state = BND_DEFAULT;
	/** Optional, owned. Tracks the pressed state of the button.*/
	Quantity *quantity = NULL;

	Button() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	~Button() {
		if (quantity)
			delete quantity;
	}

	void draw(NVGcontext *vg) override {
		bndToolButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
	}

	void onEnter(const event::Enter &e) override {
		state = BND_HOVER;
	}

	void onLeave(const event::Leave &e) override {
		state = BND_DEFAULT;
	}

	void onDragStart(const event::DragStart &e) override {
		state = BND_ACTIVE;
		if (quantity)
			quantity->setMax();
	}

	void onDragEnd(const event::DragEnd &e) override {
		state = BND_HOVER;
		if (quantity)
			quantity->setMin();
	}

	void onDragDrop(const event::DragDrop &e) override {
		if (e.origin == this) {
			event::Action eAction;
			onAction(eAction);
		}
	}
};


} // namespace rack
