#include "app/SVGKnob.hpp"


namespace rack {


SVGKnob::SVGKnob() {
	shadow = new CircularShadow;
	addChild(shadow);
	shadow->box.size = math::Vec();

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
	shadow->box.pos = math::Vec(0, sw->box.size.y * 0.1);
	// shadow->box = shadow->box.grow(math::Vec(2, 2));
}

void SVGKnob::step() {
	Knob::step();
	FramebufferWidget::step();
}

void SVGKnob::onChange(const event::Change &e) {
	// Re-transform the TransformWidget
	if (paramQuantity) {
		float angle;
		if (paramQuantity->isBounded()) {
			angle = math::rescale(paramQuantity->getScaledValue(), 0.f, 1.f, minAngle, maxAngle);
			angle = std::fmod(angle, 2*M_PI);
		}
		else {
			angle = math::rescale(paramQuantity->getValue(), 0.f, 1.f, minAngle, maxAngle);
		}
		tw->identity();
		// Rotate SVG
		math::Vec center = sw->box.getCenter();
		tw->translate(center);
		tw->rotate(angle);
		tw->translate(center.neg());
		dirty = true;
	}
	Knob::onChange(e);
}


} // namespace rack
