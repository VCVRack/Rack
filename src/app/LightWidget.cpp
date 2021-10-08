#include <app/LightWidget.hpp>
#include <color.hpp>
#include <settings.hpp>


namespace rack {
namespace app {


void LightWidget::draw(const DrawArgs& args) {
	drawBackground(args);

	// Child widgets
	Widget::draw(args);
}


void LightWidget::drawLayer(const DrawArgs& args, int layer) {
	if (layer == 1) {
		// Use the formula `lightColor * (1 - dest) + dest` for blending
		nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
		drawLight(args);
		drawHalo(args);
	}

	Widget::drawLayer(args, layer);
}


void LightWidget::drawBackground(const DrawArgs& args) {
	float radius = std::min(box.size.x, box.size.y) / 2.0;
	nvgBeginPath(args.vg);
	nvgCircle(args.vg, radius, radius, radius);

	// Background
	if (bgColor.a > 0.0) {
		nvgFillColor(args.vg, bgColor);
		nvgFill(args.vg);
	}

	// Border
	if (borderColor.a > 0.0) {
		nvgStrokeWidth(args.vg, 0.5);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
	}
}


void LightWidget::drawLight(const DrawArgs& args) {
	// Foreground
	if (color.a > 0.0) {
		float radius = std::min(box.size.x, box.size.y) / 2.0;
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, radius, radius, radius);

		nvgFillColor(args.vg, color);
		nvgFill(args.vg);
	}
}


void LightWidget::drawHalo(const DrawArgs& args) {
	// Don't draw halo if rendering in a framebuffer, e.g. screenshots or Module Browser
	if (args.fb)
		return;

	const float halo = settings::haloBrightness;
	if (halo == 0.f)
		return;

	// If light is off, rendering the halo gives no effect.
	if (color.r == 0.f && color.g == 0.f && color.b == 0.f)
		return;

	math::Vec c = box.size.div(2);
	float radius = std::min(box.size.x, box.size.y) / 2.0;
	float oradius = radius + std::min(radius * 4.f, 15.f);

	nvgBeginPath(args.vg);
	nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

	NVGcolor icol = color::mult(color, halo);
	NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
	NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
	nvgFillPaint(args.vg, paint);
	nvgFill(args.vg);
}


} // namespace app
} // namespace rack
