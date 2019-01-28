#pragma once
#include "app/common.hpp"
#include "widgets/TransparentWidget.hpp"


namespace rack {


struct CircularShadow : TransparentWidget {
	float blurRadius;
	float opacity;
	CircularShadow();
	void draw(const DrawContext &ctx) override;
};


} // namespace rack
