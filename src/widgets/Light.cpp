#include "scene.hpp"


namespace rack {


void Light::draw(NVGcontext *vg) {
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);
	float radius = box.size.x / 2.0;

	nvgBeginPath(vg);
	nvgCircle(vg, radius, radius, radius);
	nvgFillColor(vg, color);
	nvgFill(vg);

	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, colorOutline);
	nvgStroke(vg);

	nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
	NVGpaint paint;
	NVGcolor icol = color;
	icol.a = 0.1;
	NVGcolor ocol = color;
	ocol.a = 0.0;
	float oradius = radius + 30.0;
	paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgBeginPath(vg);
	nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
	nvgFill(vg);
}


} // namespace rack
