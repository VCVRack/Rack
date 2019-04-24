#pragma once
#include "ui/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "Quantity.hpp"


namespace rack {
namespace ui {


struct RadioButton : widget::OpaqueWidget {
	BNDwidgetState state = BND_DEFAULT;
	/** Not owned. */
	Quantity *quantity = NULL;

	RadioButton();
	void draw(const DrawArgs &args) override;
	void onEnter(const event::Enter &e) override;
	void onLeave(const event::Leave &e) override;
	void onDragDrop(const event::DragDrop &e) override;
};


} // namespace ui
} // namespace rack
