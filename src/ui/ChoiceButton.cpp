#include <ui/ChoiceButton.hpp>
#include <app.hpp>
#include <event.hpp>


namespace rack {
namespace ui {


void ChoiceButton::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;
	if (APP->event->hoveredWidget == this)
		state = BND_HOVER;
	if (APP->event->draggedWidget == this)
		state = BND_ACTIVE;
	bndChoiceButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}


} // namespace ui
} // namespace rack
