#include "app.hpp"


namespace rack {


void Light::draw(NVGcontext *vg) {
	NVGcolor bgColor = nvgRGBf(0.0, 0.0, 0.0);
	float radius = box.size.x / 2.0;
	float oradius = radius + 30.0;

	// Solid
	nvgBeginPath(vg);
	nvgCircle(vg, radius, radius, radius);
	nvgFillColor(vg, bgColor);
	nvgFill(vg);

	// Border
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, nvgTransRGBAf(bgColor, 0.5));
	nvgStroke(vg);

	// Inner glow
	nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
	nvgFillColor(vg, color);
	nvgFill(vg);

	// Outer glow
	nvgBeginPath(vg);
	nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);
	NVGpaint paint;
	NVGcolor icol = color;
	icol.a *= 0.1;
	NVGcolor ocol = color;
	ocol.a = 0.0;
	paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgFill(vg);
}


} // namespace rack
