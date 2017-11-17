#include "widgets.hpp"


namespace rack {

void SpriteWidget::draw(NVGcontext *vg) {
	int width, height;
	nvgImageSize(vg, spriteImage->handle, &width, &height);
	int stride = width / spriteSize.x;
	if (stride == 0) {
		warn("Size of SpriteWidget is %d, %d but spriteSize is %f, %f", width, height, spriteSize.x, spriteSize.y);
		return;
	}
	Vec offset = Vec((index % stride) * spriteSize.x, (index / stride) * spriteSize.y);
	NVGpaint paint = nvgImagePattern(vg, spriteOffset.x - offset.x, spriteOffset.y - offset.y, width, height, 0.0, spriteImage->handle, 1.0);
	nvgFillPaint(vg, paint);
	nvgBeginPath(vg);
	nvgRect(vg, spriteOffset.x, spriteOffset.y, spriteSize.x, spriteSize.y);
	nvgFill(vg);
}


} // namespace rack
