#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>
#include <Quantity.hpp>


namespace rack {
namespace ui {


/** A clickable button with text.
Dispatches Action event when clicked.

If quantity is set, its value is set to 1.0 when pressed, 0.0 when released.

If text is not set, the quantity label is used.
*/
struct Button : widget::OpaqueWidget {
	std::string text;
	/** Not owned. Tracks the pressed state of the button.*/
	Quantity* quantity = NULL;

	Button();
	void draw(const DrawArgs& args) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDragDrop(const DragDropEvent& e) override;
};


} // namespace ui
} // namespace rack
