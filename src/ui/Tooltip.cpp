#include "ui.hpp"
#include "window.hpp"


namespace rack {


Tooltip::Tooltip() {
}

void Tooltip::draw(NVGcontext *vg) {
	// Wrap size to contents
	float bounds[4];
	nvgTextBounds(gVg, 0.0, 0.0, text.c_str(), NULL, bounds);
	box.size = Vec(bounds[2], BND_WIDGET_HEIGHT);

	bndTooltipBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndMenuLabel(vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
	Widget::draw(vg);
}


} // namespace rack
