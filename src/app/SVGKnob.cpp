#include "app.hpp"


namespace rack {


SVGKnob::SVGKnob() {
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
		tw->box.size = box.size;
		float angle = 0.0;
		if (std::isfinite(minValue) && std::isfinite(maxValue))
			angle = rescalef(value, minValue, maxValue, minAngle, maxAngle);
		tw->identity();
		// Scale SVG to box
		tw->scale(box.size.div(sw->box.size));
		// Rotate SVG
		Vec center = sw->box.getCenter();
		tw->translate(center);
		tw->rotate(angle);
		tw->translate(center.neg());
	}
	FramebufferWidget::step();
}

void SVGKnob::onChange(EventChange &e) {
	dirty = true;
	Knob::onChange(e);
}



} // namespace rack
