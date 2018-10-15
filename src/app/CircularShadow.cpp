#include "app.hpp"


namespace rack {


CircularShadow::CircularShadow() {
	blurRadius = 0;
	opacity = 0.15;
}

void CircularShadow::draw(NVGcontext *vg) {
	if (opacity <= 0.0)
		return;

	nvgBeginPath(vg);
	nvgRect(vg, -blurRadius, -blurRadius, box.size.x + 2*blurRadius, box.size.y + 2*blurRadius);
	Vec center = box.size.div(2.0);
	float radius = center.x;
	NVGcolor icol = nvgRGBAf(0.0, 0.0, 0.0, opacity);
	NVGcolor ocol = nvgRGBAf(0.0, 0.0, 0.0, 0.0);
	NVGpaint paint = nvgRadialGradient(vg, center.x, center.y, radius - blurRadius, radius, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgFill(vg);
}


} // namespace rack
