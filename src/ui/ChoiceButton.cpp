#include <ui/ChoiceButton.hpp>
#include <context.hpp>


namespace rack {
namespace ui {


void ChoiceButton::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;
	if (APP->event->getHoveredWidget() == this)
		state = BND_HOVER;
	if (APP->event->getDraggedWidget() == this)
		state = BND_ACTIVE;

	std::string text = this->text;
	if (text.empty() && quantity)
		text = quantity->getLabel();
	bndChoiceButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}


} // namespace ui
} // namespace rack
