#include "app.hpp"


namespace rack {


void SVGPanel::addBackground(std::shared_ptr<SVG> svg) {
	SVGWidget *sw = new SVGWidget();
	sw->wrap();
	sw->svg = svg;
	addChild(sw);

	PanelBorder *pb = new PanelBorder();
	sw->box.size = box.size;
	addChild(pb);
}


} // namespace rack
