#pragma once
#include <widget/OpaqueWidget.hpp>
#include <Quantity.hpp>
#include <ui/common.hpp>
#include <context.hpp>


namespace rack {
namespace ui {


struct Slider : widget::OpaqueWidget {
	/** Not owned. */
	Quantity* quantity = NULL;

	Slider();
	void draw(const DrawArgs& args) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragMove(const DragMoveEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDoubleClick(const DoubleClickEvent& e) override;
};


} // namespace ui
} // namespace rack
