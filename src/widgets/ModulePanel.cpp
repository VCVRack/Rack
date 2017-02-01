#include "rack.hpp"


namespace rack {

void ModulePanel::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	NVGpaint paint;

	// Background color
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);

	// Background image
	if (!imageFilename.empty()) {
		int imageId = loadImage(imageFilename);
		int width, height;
		nvgImageSize(vg, imageId, &width, &height);
		paint = nvgImagePattern(vg, box.pos.x, box.pos.y, width, height, 0.0, imageId, 1.0);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
	}
}

} // namespace rack
