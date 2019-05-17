#pragma once
#include "widget/OpaqueWidget.hpp"
#include "Quantity.hpp"
#include "ui/common.hpp"
#include "app.hpp"


namespace rack {
namespace ui {


struct Slider : widget::OpaqueWidget {
	/** Not owned. */
	Quantity *quantity = NULL;

	Slider();
	void draw(const DrawArgs &args) override;
	void onDragStart(const event::DragStart &e) override;
	void onDragMove(const event::DragMove &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void onDoubleClick(const event::DoubleClick &e) override;
};


} // namespace ui
} // namespace rack
