#include "app.hpp"


namespace rack {


SVGSwitch::SVGSwitch() {
	padding = Vec(1, 1);

	sw = new SVGWidget();
	addChild(sw);
}

void SVGSwitch::addFrame(std::shared_ptr<SVG> svg) {
	frames.push_back(svg);
	// Automatically set the frame as this SVG file.
	// This allows us to wrap() the widget after calling
	if (!sw->svg)
		sw->svg = svg;
}

void SVGSwitch::setIndex(int index) {
	if (0 <= index && index < (int)frames.size())
		sw->svg = frames[index];
	dirty = true;
}

void SVGSwitch::step() {
	FramebufferWidget::step();
}


} // namespace rack
