#include <app/LightWidget.hpp>
#include <color.hpp>


namespace rack {
namespace app {


void LightWidget::draw(const DrawArgs& args) {
	drawLight(args);
	drawHalo(args);
}

void LightWidget::drawLight(const DrawArgs& args) {
	float radius = std::min(box.size.x, box.size.y) / 2.0;

	nvgBeginPath(args.vg);
	nvgCircle(args.vg, radius, radius, radius);

	// Background
	if (bgColor.a > 0.0) {
		nvgFillColor(args.vg, bgColor);
		nvgFill(args.vg);
	}

	// Foreground
	if (color.a > 0.0) {
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);
	}

	// Border
	if (borderColor.a > 0.0) {
		nvgStrokeWidth(args.vg, 0.5);
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);
	}
}

void LightWidget::drawHalo(const DrawArgs& args) {
	float radius = std::min(box.size.x, box.size.y) / 2.0;
	float oradius = 4.0 * radius;

	nvgBeginPath(args.vg);
	nvgRect(args.vg, radius - oradius, radius - oradius, 2 * oradius, 2 * oradius);

	NVGpaint paint;
	NVGcolor icol = color::mult(color, 0.07);
	NVGcolor ocol = nvgRGB(0, 0, 0);
	paint = nvgRadialGradient(args.vg, radius, radius, radius, oradius, icol, ocol);
	nvgFillPaint(args.vg, paint);
	nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
	nvgFill(args.vg);
}


} // namespace app
} // namespace rack
