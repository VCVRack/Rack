#pragma once
#include "widget/OpaqueWidget.hpp"
#include "Quantity.hpp"
#include "ui/common.hpp"
#include "app.hpp"


namespace rack {
namespace ui {


struct Slider : widget::OpaqueWidget {
	BNDwidgetState state = BND_DEFAULT;
	/** Not owned. */
	Quantity *quantity = NULL;

	Slider();
	void draw(const DrawArgs &args) override;
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragMove(const widget::DragMoveEvent &e) override;
	void onDragEnd(const widget::DragEndEvent &e) override;
	void onDoubleClick(const widget::DoubleClickEvent &e) override;
};


} // namespace ui
} // namespace rack
