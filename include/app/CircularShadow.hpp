#pragma once
#include "app/common.hpp"


namespace rack {


struct CircularShadow : TransparentWidget {
	float blurRadius;
	float opacity;
	CircularShadow();
	void draw(NVGcontext *vg) override;
};


} // namespace rack
