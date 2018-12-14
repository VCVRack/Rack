#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/Slider.hpp"
#include "ui/RadioButton.hpp"
#include "app/common.hpp"


namespace rack {


struct Toolbar : OpaqueWidget {
	// TODO Move these to future Rack app state
	float wireOpacity = 0.5;
	float wireTension = 0.5;

	Toolbar();
	void draw(NVGcontext *vg) override;
};


} // namespace rack
