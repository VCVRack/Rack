#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>
#include <Quantity.hpp>


namespace rack {
namespace ui {


struct Button : widget::OpaqueWidget {
	std::string text;
	/** Not owned. Tracks the pressed state of the button.*/
	Quantity* quantity = NULL;

	Button();
	void draw(const DrawArgs& args) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragEnd(const event::DragEnd& e) override;
	void onDragDrop(const event::DragDrop& e) override;
};


} // namespace ui
} // namespace rack
