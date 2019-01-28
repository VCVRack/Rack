#pragma once
#include "app/common.hpp"
#include "widgets/TransparentWidget.hpp"


namespace rack {


struct RackRail : TransparentWidget {
	void draw(const DrawContext &ctx) override;
};


} // namespace rack
