#include "app/CircularShadow.hpp"


namespace rack {


CircularShadow::CircularShadow() {
	blurRadius = 0;
	opacity = 0.15;
}

void CircularShadow::draw(const DrawContext &ctx) {
	if (opacity <= 0.0)
		return;

	nvgBeginPath(ctx.vg);
	nvgRect(ctx.vg, -blurRadius, -blurRadius, box.size.x + 2*blurRadius, box.size.y + 2*blurRadius);
	math::Vec center = box.size.div(2.0);
	float radius = center.x;
	NVGcolor icol = nvgRGBAf(0.0, 0.0, 0.0, opacity);
	NVGcolor ocol = nvgRGBAf(0.0, 0.0, 0.0, 0.0);
	NVGpaint paint = nvgRadialGradient(ctx.vg, center.x, center.y, radius - blurRadius, radius, icol, ocol);
	nvgFillPaint(ctx.vg, paint);
	nvgFill(ctx.vg);
}


} // namespace rack
