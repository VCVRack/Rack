#pragma once
#include "ui/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "ui/Quantity.hpp"


namespace rack {
namespace ui {


struct RadioButton : widget::OpaqueWidget {
	BNDwidgetState state = BND_DEFAULT;
	Quantity *quantity = NULL;

	RadioButton();
	~RadioButton();
	void draw(const DrawArgs &args) override;
	void onEnter(const widget::EnterEvent &e) override;
	void onLeave(const widget::LeaveEvent &e) override;
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragDrop(const widget::DragDropEvent &e) override;
};


} // namespace ui
} // namespace rack
