#include "app.hpp"


namespace rack {

void Panel::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);

	// Background color
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);

	// Background image
	if (backgroundImage) {
		int width, height;
		nvgImageSize(vg, backgroundImage->handle, &width, &height);
		NVGpaint paint = nvgImagePattern(vg, 0.0, 0.0, width, height, 0.0, backgroundImage->handle, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
	}

	// Border color
	nvgStrokeColor(vg, borderColor);
	nvgStrokeWidth(vg, 0.5);
	nvgStroke(vg);
}

} // namespace rack
