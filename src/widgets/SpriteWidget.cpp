#include "Rack.hpp"


namespace rack {

void SpriteWidget::draw(NVGcontext *vg) {
	int imageId = loadImage(spriteFilename);
	if (imageId < 0) {
		// printf("Could not load image %s for SpriteWidget\n", spriteFilename.c_str());
		return;
	}

	Vec pos = box.pos.plus(spriteOffset);

	int width, height;
	nvgImageSize(vg, imageId, &width, &height);
	int stride = width / spriteSize.x;
	if (stride == 0) {
		printf("Width of SpriteWidget is %d but spriteSize is %f\n", width, spriteSize.x);
		return;
	}
	Vec offset = Vec((index % stride) * spriteSize.x, (index / stride) * spriteSize.y);
	NVGpaint paint = nvgImagePattern(vg, pos.x - offset.x, pos.y - offset.y, width, height, 0.0, imageId, 1.0);
	nvgFillPaint(vg, paint);
	nvgBeginPath(vg);
	nvgRect(vg, pos.x, pos.y, spriteSize.x, spriteSize.y);
	nvgFill(vg);
}


} // namespace rack
