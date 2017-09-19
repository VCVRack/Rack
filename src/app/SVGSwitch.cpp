#include "app.hpp"


namespace rack {


SVGSwitch::SVGSwitch() {
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

void SVGSwitch::step() {
	FramebufferWidget::step();
}

void SVGSwitch::onChange() {
	int index = roundf(rescalef(value, minValue, maxValue, 0, frames.size() - 1));
	if (0 <= index && index < (int)frames.size())
		sw->svg = frames[index];
	dirty = true;
	ParamWidget::onChange();
}


} // namespace rack
