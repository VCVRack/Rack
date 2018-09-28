#include "app.hpp"


namespace rack {


SVGButton::SVGButton() {
	sw = new SVGWidget();
	addChild(sw);
}

void SVGButton::setSVGs(std::shared_ptr<SVG> defaultSVG, std::shared_ptr<SVG> activeSVG) {
	sw->setSVG(defaultSVG);
	box.size = sw->box.size;
	this->defaultSVG = defaultSVG;
	this->activeSVG = activeSVG ? activeSVG : defaultSVG;
}

void SVGButton::on(event::DragStart &e) {
	event::Action eAction;
	handleEvent(eAction);
	sw->setSVG(activeSVG);
	dirty = true;
}

void SVGButton::on(event::DragEnd &e) {
	sw->setSVG(defaultSVG);
	dirty = true;
}


} // namespace rack
