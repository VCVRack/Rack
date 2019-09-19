#include <app/SvgKnob.hpp>


namespace rack {
namespace app {


SvgKnob::SvgKnob() {
	fb = new widget::FramebufferWidget;
	addChild(fb);

	shadow = new CircularShadow;
	fb->addChild(shadow);
	shadow->box.size = math::Vec();

	tw = new widget::TransformWidget;
	fb->addChild(tw);

	sw = new widget::SvgWidget;
	tw->addChild(sw);
}

void SvgKnob::setSvg(std::shared_ptr<Svg> svg) {
	sw->setSvg(svg);
	tw->box.size = sw->box.size;
	fb->box.size = sw->box.size;
	box.size = sw->box.size;
	shadow->box.size = sw->box.size;
	// Move shadow downward by 10%
	shadow->box.pos = math::Vec(0, sw->box.size.y * 0.10);
	// shadow->box = shadow->box.grow(math::Vec(2, 2));
}

void SvgKnob::onChange(const event::Change& e) {
	// Re-transform the widget::TransformWidget
	if (paramQuantity) {
		float angle;
		if (paramQuantity->isBounded()) {
			angle = math::rescale(paramQuantity->getScaledValue(), 0.f, 1.f, minAngle, maxAngle);
		}
		else {
			angle = math::rescale(paramQuantity->getValue(), -1.f, 1.f, minAngle, maxAngle);
		}
		angle = std::fmod(angle, 2 * M_PI);
		tw->identity();
		// Rotate SVG
		math::Vec center = sw->box.getCenter();
		tw->translate(center);
		tw->rotate(angle);
		tw->translate(center.neg());
		fb->dirty = true;
	}
	Knob::onChange(e);
}


} // namespace app
} // namespace rack
