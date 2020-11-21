#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>
#include <Quantity.hpp>


namespace rack {
namespace ui {


struct OptionButton : widget::OpaqueWidget {
	std::string text;
	/** Not owned. Tracks the pressed state of the button.*/
	Quantity* quantity = NULL;

	OptionButton();
	void draw(const DrawArgs& args) override;
	void onDragDrop(const event::DragDrop& e) override;
};


} // namespace ui
} // namespace rack
