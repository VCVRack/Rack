#include "app/LightWidget.hpp"
#include "color.hpp"


namespace rack {


void LightWidget::draw(const DrawContext &ctx) {
	drawLight(ctx);
	drawHalo(ctx);
}

void LightWidget::drawLight(const DrawContext &ctx) {
	float radius = box.size.x / 2.0;

	nvgBeginPath(ctx.vg);
	nvgCircle(ctx.vg, radius, radius, radius);

	// Background
	nvgFillColor(ctx.vg, bgColor);
	nvgFill(ctx.vg);

	// Foreground
	nvgFillColor(ctx.vg, color);
	nvgFill(ctx.vg);

	// Border
	nvgStrokeWidth(ctx.vg, 0.5);
	nvgStrokeColor(ctx.vg, borderColor);
	nvgStroke(ctx.vg);
}

void LightWidget::drawHalo(const DrawContext &ctx) {
	float radius = box.size.x / 2.0;
	float oradius = radius + 15.0;

	nvgBeginPath(ctx.vg);
	nvgRect(ctx.vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);

	NVGpaint paint;
	NVGcolor icol = color::mult(color, 0.08);
	NVGcolor ocol = nvgRGB(0, 0, 0);
	paint = nvgRadialGradient(ctx.vg, radius, radius, radius, oradius, icol, ocol);
	nvgFillPaint(ctx.vg, paint);
	nvgGlobalCompositeOperation(ctx.vg, NVG_LIGHTER);
	nvgFill(ctx.vg);
}


} // namespace rack
