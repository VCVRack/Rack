#include <app/CircularShadow.hpp>


namespace rack {
namespace app {


CircularShadow::CircularShadow() {
	blurRadius = 0;
	opacity = 0.15;
}

void CircularShadow::draw(const DrawArgs& args) {
	if (opacity <= 0.0)
		return;

	nvgBeginPath(args.vg);
	nvgRect(args.vg, -blurRadius, -blurRadius, box.size.x + 2 * blurRadius, box.size.y + 2 * blurRadius);
	math::Vec center = box.size.div(2.0);
	float radius = center.x;
	NVGcolor icol = nvgRGBAf(0.0, 0.0, 0.0, opacity);
	NVGcolor ocol = nvgRGBAf(0.0, 0.0, 0.0, 0.0);
	NVGpaint paint = nvgRadialGradient(args.vg, center.x, center.y, radius - blurRadius, radius, icol, ocol);
	nvgFillPaint(args.vg, paint);
	nvgFill(args.vg);
}


} // namespace app
} // namespace rack
