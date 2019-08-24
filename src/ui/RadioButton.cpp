#include <ui/RadioButton.hpp>


namespace rack {
namespace ui {


RadioButton::RadioButton() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void RadioButton::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;
	if (APP->event->hoveredWidget == this)
		state = BND_HOVER;

	std::string label;
	if (quantity) {
		label = quantity->getLabel();
		if (quantity->isMax())
			state = BND_ACTIVE;
	}
	bndRadioButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, label.c_str());
}

void RadioButton::onDragDrop(const event::DragDrop& e) {
	if (e.origin == this) {
		if (quantity) {
			if (quantity->isMax())
				quantity->setMin();
			else
				quantity->setMax();
		}

		event::Action eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack
