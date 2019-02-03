#include "app/LightWidget.hpp"
#include "color.hpp"


namespace rack {
namespace app {


void LightWidget::draw(const widget::DrawContext &ctx) {
	drawLight(ctx);
	drawHalo(ctx);
}

void LightWidget::drawLight(const widget::DrawContext &ctx) {
	float radius = box.size.x / 2.0;

	nvgBeginPath(ctx.vg);
	nvgCircle(ctx.vg, radius, radius, radius);

	// Background
	if (bgColor.a > 0.0) {
		nvgFillColor(ctx.vg, bgColor);
		nvgFill(ctx.vg);
	}

	// Foreground
	if (color.a > 0.0) {
		nvgFillColor(ctx.vg, color);
		nvgFill(ctx.vg);
	}

	// Border
	if (borderColor.a > 0.0) {
		nvgStrokeWidth(ctx.vg, 0.5);
		nvgStrokeColor(ctx.vg, borderColor);
		nvgStroke(ctx.vg);
	}
}

void LightWidget::drawHalo(const widget::DrawContext &ctx) {
	float radius = box.size.x / 2.0;
	float oradius = 4.0 * radius;

	nvgBeginPath(ctx.vg);
	nvgRect(ctx.vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);

	NVGpaint paint;
	NVGcolor icol = color::mult(color, 0.07);
	NVGcolor ocol = nvgRGB(0, 0, 0);
	paint = nvgRadialGradient(ctx.vg, radius, radius, radius, oradius, icol, ocol);
	nvgFillPaint(ctx.vg, paint);
	nvgGlobalCompositeOperation(ctx.vg, NVG_LIGHTER);
	nvgFill(ctx.vg);
}


} // namespace app
} // namespace rack
