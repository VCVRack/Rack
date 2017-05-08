#include "app.hpp"


namespace rack {


SVGKnob::SVGKnob() {
	padding = Vec(1, 1);

	tw = new TransformWidget();
	addChild(tw);

	sw = new SVGWidget();
	tw->addChild(sw);
}

void SVGKnob::setSVG(std::shared_ptr<SVG> svg) {
	sw->svg = svg;
	sw->wrap();
	tw->box.size = sw->box.size;
	box.size = sw->box.size;
}

void SVGKnob::step() {
	// Re-transform TransformWidget if dirty
	if (dirty) {
		float angle = mapf(value, minValue, maxValue, minAngle, maxAngle);
		tw->identity();
		// Rotate SVG
		Vec center = sw->box.getCenter();
		tw->translate(center);
		tw->rotate(angle);
		tw->translate(center.neg());
	}
	FramebufferWidget::step();
}

void SVGKnob::onChange() {
	dirty = true;
	Knob::onChange();
}



} // namespace rack
