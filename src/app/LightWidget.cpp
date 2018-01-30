#include "app.hpp"


namespace rack {


void LightWidget::draw(NVGcontext *vg) {
	color.r = clampf(color.r, 0.0, 1.0);
	color.g = clampf(color.g, 0.0, 1.0);
	color.b = clampf(color.b, 0.0, 1.0);
	color.a = clampf(color.a, 0.0, 1.0);

	drawLight(vg);
	drawHalo(vg);
}

void LightWidget::drawLight(NVGcontext *vg) {
	float radius = box.size.x / 2.0;

	nvgBeginPath(vg);
	nvgCircle(vg, radius, radius, radius);

	// Background
	nvgFillColor(vg, bgColor);
	nvgFill(vg);

	// Border
	nvgStrokeWidth(vg, 1.0);
	NVGcolor borderColor = bgColor;
	borderColor.a *= 0.5;
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);

	// Inner glow
	nvgFillColor(vg, color);
	nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
	nvgFill(vg);
}

void LightWidget::drawHalo(NVGcontext *vg) {
	float radius = box.size.x / 2.0;
	float oradius = radius + 15.0;

	nvgBeginPath(vg);
	nvgRect(vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);

	NVGpaint paint;
	NVGcolor icol = color;
	icol.a *= 0.10;
	NVGcolor ocol = color;
	ocol.a = 0.0;
	paint = nvgRadialGradient(vg, radius, radius, radius, oradius, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
	nvgFill(vg);
}


} // namespace rack
