#pragma once
#include "widgets/TransparentWidget.hpp"
#include "app/common.hpp"


namespace rack {


struct RackRail : TransparentWidget {
	void draw(NVGcontext *vg) override;
};


} // namespace rack
