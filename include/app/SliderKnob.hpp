#pragma once
#include <app/common.hpp>
#include <app/Knob.hpp>


namespace rack {
namespace app {


struct SliderKnob : Knob {
	SliderKnob();

	void onHover(const event::Hover& e) override;
	void onButton(const event::Button& e) override;
};


} // namespace app
} // namespace rack
