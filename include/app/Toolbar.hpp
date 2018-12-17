#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"


namespace rack {


struct Toolbar : OpaqueWidget {
	// TODO Move these to future Rack app state
	float wireOpacity = 0.5;
	float wireTension = 0.5;

	Toolbar();
	void draw(NVGcontext *vg) override;
};


} // namespace rack
