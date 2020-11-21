#include <ui/OptionButton.hpp>


namespace rack {
namespace ui {


OptionButton::OptionButton() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void OptionButton::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;
	if (quantity && !quantity->isMin())
		state = BND_ACTIVE;

	bndOptionButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, text.c_str());
}


void OptionButton::onDragDrop(const event::DragDrop& e) {
	if (e.origin == this) {
		if (quantity)
			quantity->toggle();

		event::Action eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack
