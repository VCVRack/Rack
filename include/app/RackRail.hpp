#pragma once
#include "common.hpp"


namespace rack {


struct RackRail : TransparentWidget {
	void draw(NVGcontext *vg) override;
};


} // namespace rack
