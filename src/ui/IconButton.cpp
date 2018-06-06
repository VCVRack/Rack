#include "ui.hpp"


namespace rack {


IconButton::IconButton() {
	box.size.x = BND_WIDGET_HEIGHT;
	sw = new SVGWidget();
	sw->box.pos = Vec(2.75, 1.75);
	addChild(sw);
}

void IconButton::setSVG(std::shared_ptr<SVG> svg) {
	sw->setSVG(svg);
}


} // namespace rack
