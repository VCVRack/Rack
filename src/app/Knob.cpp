#include "app/Knob.hpp"


namespace rack {


void Knob::onDragStart(event::DragStart &e) {
	context()->window->cursorLock();
	e.target = this;
}

void Knob::onDragEnd(event::DragEnd &e) {
	context()->window->cursorUnlock();
}

void Knob::onDragMove(event::DragMove &e) {
	if (quantity) {
		float range;
		if (quantity->isBounded()) {
			range = quantity->getRange();
		}
		else {
			// Continuous encoders scale as if their limits are +/-1
			range = 2.f;
		}
		float delta = KNOB_SENSITIVITY * -e.mouseDelta.y * speed * range;

		// Drag slower if Mod is held
		if (context()->window->isModPressed())
			delta /= 16.f;
		quantity->moveValue(delta);

		event::Change eChange;
		onChange(eChange);
	}
}


} // namespace rack
