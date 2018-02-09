#include "ui.hpp"


namespace rack {

void ProgressBar::draw(NVGcontext *vg) {
	float progress = rescale(value, minValue, maxValue, 0.0, 1.0);
	bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_ALL, BND_DEFAULT, progress, getText().c_str(), NULL);
}


} // namespace rack
