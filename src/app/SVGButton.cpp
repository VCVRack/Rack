#include "app/SVGButton.hpp"


namespace rack {


SVGButton::SVGButton() {
	sw = new SVGWidget;
	addChild(sw);
}

void SVGButton::setSVGs(std::shared_ptr<SVG> defaultSVG, std::shared_ptr<SVG> activeSVG) {
	sw->setSVG(defaultSVG);
	box.size = sw->box.size;
	this->defaultSVG = defaultSVG;
	this->activeSVG = activeSVG ? activeSVG : defaultSVG;
}

void SVGButton::onDragStart(event::DragStart &e) {
	event::Action eAction;
	onAction(eAction);
	sw->setSVG(activeSVG);
	dirty = true;
}

void SVGButton::onDragEnd(event::DragEnd &e) {
	sw->setSVG(defaultSVG);
	dirty = true;
}


} // namespace rack
