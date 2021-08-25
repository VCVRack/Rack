#include <ui/RadioButton.hpp>
#include <context.hpp>


namespace rack {
namespace ui {


void RadioButton::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;
	if (APP->event->getHoveredWidget() == this)
		state = BND_HOVER;

	if (quantity) {
		if (quantity->isMax())
			state = BND_ACTIVE;
	}

	std::string text = this->text;
	if (text.empty() && quantity)
		text = quantity->getLabel();
	bndRadioButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}


void RadioButton::onDragStart(const DragStartEvent& e) {
	OpaqueWidget::onDragStart(e);
}


void RadioButton::onDragEnd(const DragEndEvent& e) {
	OpaqueWidget::onDragEnd(e);
}


void RadioButton::onDragDrop(const DragDropEvent& e) {
	if (e.origin == this) {
		if (quantity)
			quantity->toggle();

		ActionEvent eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack
