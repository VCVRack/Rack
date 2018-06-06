#include "ui.hpp"


namespace rack {


IconButton::IconButton() {
	box.size.x = BND_TOOL_WIDTH;
	sw = new SVGWidget();
	sw->box.pos = Vec(2, 2);
	addChild(sw);
}

void IconButton::setSVG(std::shared_ptr<SVG> svg) {
	sw->setSVG(svg);
}


} // namespace rack
