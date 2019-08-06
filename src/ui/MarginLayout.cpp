#include <ui/MarginLayout.hpp>
#include <vector>


namespace rack {
namespace ui {


void MarginLayout::step() {
	Widget::step();

	math::Rect childBox = box.zeroPos().grow(margin.neg());
	for (Widget* child : children) {
		child->box = childBox;
	}
}


} // namespace ui
} // namespace rack
