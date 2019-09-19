#include <ui/List.hpp>


namespace rack {
namespace ui {


void List::step() {
	Widget::step();

	// Set positions of children
	box.size.y = 0.0;
	for (widget::Widget* child : children) {
		if (!child->visible)
			continue;
		// Set position of child
		child->box.pos = math::Vec(0.0, box.size.y);
		// Increment height
		box.size.y += child->box.size.y;
		// Resize width of child
		child->box.size.x = box.size.x;
	}
}


} // namespace ui
} // namespace rack
