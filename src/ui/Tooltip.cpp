#include "ui/Tooltip.hpp"
#include "context.hpp"
#include "window.hpp"


namespace rack {


void Tooltip::step() {
	// Wrap size to contents
	box.size.x = bndLabelWidth(context()->window->vg, -1, text.c_str()) + 10.0;
	box.size.y = bndLabelHeight(context()->window->vg, -1, text.c_str(), INFINITY);
	Widget::step();
}

void Tooltip::draw(NVGcontext *vg) {
	bndTooltipBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	bndMenuLabel(vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
	Widget::draw(vg);
}


} // namespace rack
