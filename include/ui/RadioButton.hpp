#pragma once
#include "ui/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "ui/Quantity.hpp"


namespace rack {


struct RadioButton : OpaqueWidget {
	BNDwidgetState state = BND_DEFAULT;
	Quantity *quantity = NULL;

	RadioButton();
	~RadioButton();
	void draw(const DrawContext &ctx) override;
	void onEnter(const event::Enter &e) override;
	void onLeave(const event::Leave &e) override;
	void onDragDrop(const event::DragDrop &e) override;
};


} // namespace rack
