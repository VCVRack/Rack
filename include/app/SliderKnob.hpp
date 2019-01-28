#pragma once
#include "app/common.hpp"
#include "app/Knob.hpp"


namespace rack {


struct SliderKnob : Knob {
	// Bypass Knob's circular hitbox detection
	void onHover(const event::Hover &e) override {
		ParamWidget::onHover(e);
	}
	void onButton(const event::Button &e) override {
		ParamWidget::onButton(e);
	}
};


} // namespace rack
