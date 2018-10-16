#pragma once
#include "app/common.hpp"


namespace rack {


struct Toolbar : OpaqueWidget {
	Slider *wireOpacitySlider;
	Slider *wireTensionSlider;
	Slider *zoomSlider;
	RadioButton *cpuUsageButton;

	Toolbar();
	void draw(NVGcontext *vg) override;
};


} // namespace rack
