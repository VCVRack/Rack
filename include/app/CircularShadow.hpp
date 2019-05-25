#pragma once
#include <app/common.hpp>
#include <widget/TransparentWidget.hpp>


namespace rack {
namespace app {


struct CircularShadow : widget::TransparentWidget {
	float blurRadius;
	float opacity;

	CircularShadow();
	void draw(const DrawArgs &args) override;
};


} // namespace app
} // namespace rack
