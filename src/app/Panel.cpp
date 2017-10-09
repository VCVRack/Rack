#include "app.hpp"


namespace rack {


void Panel::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);

	// Background color
	if (backgroundColor.a > 0) {
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
	}

	// Background image
	if (backgroundImage) {
		int width, height;
		nvgImageSize(vg, backgroundImage->handle, &width, &height);
		NVGpaint paint = nvgImagePattern(vg, 0.0, 0.0, width, height, 0.0, backgroundImage->handle, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
	}

	// Border
	NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);
	nvgBeginPath(vg);
	nvgRect(vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStrokeWidth(vg, 1.0);
	nvgStroke(vg);

	Widget::draw(vg);
}

} // namespace rack
