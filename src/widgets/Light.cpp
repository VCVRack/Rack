#include "scene.hpp"


namespace rack {


void Light::draw(NVGcontext *vg) {
	NVGcolor colorOutline = nvgLerpRGBA(color, nvgRGBf(0.0, 0.0, 0.0), 0.5);
	Vec c = box.getCenter();
	Vec r = box.size.div(2.0);

	nvgBeginPath(vg);
	nvgEllipse(vg, c.x, c.y, r.x - 1.0, r.y - 1.0);
	nvgFillColor(vg, color);
	nvgFill(vg);

	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, colorOutline);
	nvgStroke(vg);

	// float radius = box.size.x / 2.0;
	// NVGcolor icol, ocol;
	// NVGpaint paint;
	// icol = color;
	// icol.a = clampf(color.a / 10.0, 0.0, 1.0);
	// ocol = color;
	// ocol.a = 0.0;
	// float oradius = radius + 20.0;
	// paint = nvgRadialGradient(vg, c.x, c.y, radius, oradius, icol, ocol);
	// nvgFillPaint(vg, paint);
	// nvgBeginPath(vg);
	// nvgRect(vg, c.x - oradius, c.y - oradius, 2*oradius, 2*oradius);
	// nvgFill(vg);
}


} // namespace rack
