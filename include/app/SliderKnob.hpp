#pragma once
#include "app/common.hpp"
#include "app/Knob.hpp"


namespace rack {
namespace app {


struct SliderKnob : Knob {
	// Bypass Knob's circular hitbox detection
	void onHover(const widget::HoverEvent &e) override {
		ParamWidget::onHover(e);
	}
	void onButton(const widget::ButtonEvent &e) override {
		ParamWidget::onButton(e);
	}
};


} // namespace app
} // namespace rack
