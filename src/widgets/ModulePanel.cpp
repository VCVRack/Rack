#include "scene.hpp"


namespace rack {

void ModulePanel::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	NVGpaint paint;

	// Background color
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);

	// Background image
	if (backgroundImage) {
		int width, height;
		nvgImageSize(vg, backgroundImage->handle, &width, &height);
		paint = nvgImagePattern(vg, box.pos.x, box.pos.y, width, height, 0.0, backgroundImage->handle, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
	}
}

} // namespace rack
