#include "Rack.hpp"


namespace rack {

void Light::draw(NVGcontext *vg) {
	SpriteWidget::draw(vg);

	if (color.a == 0.0)
		return;
	// Draw glow
	Vec c = box.getCenter();
	float radius = box.size.x / 2.0;
	NVGcolor icol, ocol;
	NVGpaint paint;
	// Inner glow
	icol = nvgRGBf(1.0, 1.0, 1.0);
	icol.a = clampf(color.a, 0.0, 1.0);
	ocol = color;
	ocol.a = clampf(color.a, 0.0, 1.0);
	paint = nvgRadialGradient(vg, c.x+1, c.y+3, 0.0, radius, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgBeginPath(vg);
	nvgCircle(vg, c.x, c.y, radius);
	nvgFill(vg);
	// Outer glow
	icol = color;
	icol.a = clampf(color.a / 10.0, 0.0, 1.0);
	ocol = color;
	ocol.a = 0.0;
	float oradius = radius + 20.0;
	paint = nvgRadialGradient(vg, c.x, c.y, radius, oradius, icol, ocol);
	nvgFillPaint(vg, paint);
	nvgBeginPath(vg);
	nvgRect(vg, c.x - oradius, c.y - oradius, 2*oradius, 2*oradius);
	nvgFill(vg);
}


} // namespace rack
