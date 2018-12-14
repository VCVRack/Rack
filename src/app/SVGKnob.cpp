#include "app.hpp"


namespace rack {


SVGKnob::SVGKnob() {
	shadow = new CircularShadow;
	addChild(shadow);
	shadow->box.size = Vec();

	tw = new TransformWidget;
	addChild(tw);

	sw = new SVGWidget;
	tw->addChild(sw);
}

void SVGKnob::setSVG(std::shared_ptr<SVG> svg) {
	sw->setSVG(svg);
	tw->box.size = sw->box.size;
	box.size = sw->box.size;
	shadow->box.size = sw->box.size;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);
	// shadow->box = shadow->box.grow(Vec(2, 2));
}

void SVGKnob::step() {
	// Re-transform TransformWidget if dirty
	if (dirty && quantity) {
		float angle;
		if (quantity->isBounded()) {
			angle = rescale(quantity->getValue(), -1.f, 1.f, minAngle, maxAngle);
			angle = std::fmod(angle, 2*M_PI);
		}
		else {
			angle = rescale(quantity->getScaledValue(), 0.f, 1.f, minAngle, maxAngle);
		}
		tw->identity();
		// Rotate SVG
		Vec center = sw->box.getCenter();
		tw->translate(center);
		tw->rotate(angle);
		tw->translate(center.neg());
	}
	FramebufferWidget::step();
}

void SVGKnob::onChange(event::Change &e) {
	dirty = true;
	ParamWidget::onChange(e);
}


} // namespace rack
