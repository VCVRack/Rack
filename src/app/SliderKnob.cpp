#include <app/SliderKnob.hpp>


namespace rack {
namespace app {


SliderKnob::SliderKnob() {
	forceLinear = true;
}

void SliderKnob::onHover(const event::Hover& e) {
	// Bypass Knob's circular hitbox detection
	ParamWidget::onHover(e);
}

void SliderKnob::onButton(const event::Button& e) {
	ParamWidget::onButton(e);
}


} // namespace app
} // namespace rack
