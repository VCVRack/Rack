#include "app.hpp"


namespace rack {


SVGKnob::SVGKnob() {
	tw = new TransformWidget();
	setScene(tw);

	sw = new SVGWidget();
	tw->addChild(sw);
}

void SVGKnob::setSVG(std::shared_ptr<SVG> svg) {
	sw->svg = svg;
	sw->wrap();
}

void SVGKnob::step() {
	// Re-transform TransformWidget if dirty
	if (dirty) {
		float angle = mapf(value, minValue, maxValue, minAngle, maxAngle);
		tw->box.size = box.size;
		tw->identity();
		// Resize SVG
		Vec scale = Vec(box.size.x / sw->box.size.x, box.size.y / sw->box.size.y);
		tw->scale(scale);
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
	ParamWidget::onChange();
}



} // namespace rack
