#pragma once
#include "widget/OpaqueWidget.hpp"
#include "ui/common.hpp"
#include "ui/Quantity.hpp"


namespace rack {
namespace ui {


struct Button : widget::OpaqueWidget {
	std::string text;
	BNDwidgetState state = BND_DEFAULT;
	/** Optional, owned. Tracks the pressed state of the button.*/
	Quantity *quantity = NULL;

	Button();
	~Button();
	void draw(const DrawArgs &args) override;
	void onEnter(const widget::EnterEvent &e) override;
	void onLeave(const widget::LeaveEvent &e) override;
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragEnd(const widget::DragEndEvent &e) override;
	void onDragDrop(const widget::DragDropEvent &e) override;
};


} // namespace ui
} // namespace rack
