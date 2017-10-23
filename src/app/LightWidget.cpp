#include "app.hpp"


namespace rack {


void LightWidget::draw(NVGcontext *vg) {
	float radius = box.size.x / 2.0;
	float oradius = radius + 20.0;

	// Solid
	nvgBeginPath(vg);
	nvgCircle(vg, radius, radius, radius);
	nvgFillColor(vg, bgColor);
	nvgFill(vg);

	// Border
	nvgStrokeWidth(vg, 1.0);
	NVGcolor borderColor = bgColor;
	borderColor.a *= 0.5;
	nvgStrokeColor(vg, borderColor);
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
	icol.a *= 0.15;
	NVGcolor ocol = color;
	ocol.a = 0.0;
	paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgFill(vg);
}


} // namespace rack
