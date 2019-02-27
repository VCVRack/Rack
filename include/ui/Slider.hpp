#pragma once
#include "widget/OpaqueWidget.hpp"
#include "ui/Quantity.hpp"
#include "ui/common.hpp"
#include "app.hpp"


namespace rack {
namespace ui {


struct Slider : widget::OpaqueWidget {
	BNDwidgetState state = BND_DEFAULT;
	Quantity *quantity = NULL;

	Slider();
	~Slider();
	void draw(const DrawArgs &args) override;
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragMove(const widget::DragMoveEvent &e) override;
	void onDragEnd(const widget::DragEndEvent &e) override;
	void onDoubleClick(const widget::DoubleClickEvent &e) override;
};


} // namespace ui
} // namespace rack
