#include "app.hpp"


namespace rack {


void CircularShadow::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg, -blur, -blur, box.size.x + 2*blur, box.size.y + 2*blur);
	nvgFillColor(vg, nvgRGBAf(0.0, 0.0, 0.0, 0.25));
	Vec c = box.size.div(2.0);
	float radius = c.x;
	NVGcolor icol = nvgRGBAf(0.0, 0.0, 0.0, 0.25);
	NVGcolor ocol = nvgRGBAf(0.0, 0.0, 0.0, 0.0);
	NVGpaint paint = nvgRadialGradient(vg, c.x, c.y, radius - blur/2, radius + blur/2, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgFill(vg);
}


} // namespace rack
