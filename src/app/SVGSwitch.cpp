#include "app.hpp"


namespace rack {


SVGSwitch::SVGSwitch() {
	sw = new SVGWidget();
	addChild(sw);
}

void SVGSwitch::addFrame(std::shared_ptr<SVG> svg) {
	frames.push_back(svg);
	// If this is our first frame, automatically set SVG and size
	if (!sw->svg) {
		sw->setSVG(svg);
		box.size = sw->box.size;
	}
}

void SVGSwitch::onChange(EventChange &e) {
	assert(frames.size() > 0);
	float valueScaled = rescale(value, minValue, maxValue, 0, frames.size() - 1);
	int index = clamp((int) roundf(valueScaled), 0, (int) frames.size() - 1);
	sw->setSVG(frames[index]);
	dirty = true;
	ParamWidget::onChange(e);
}


} // namespace rack
