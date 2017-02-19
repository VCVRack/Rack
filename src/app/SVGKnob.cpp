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

void SVGKnob::draw(NVGcontext *vg) {
	// Draw circular shadow below knob
	// TODO This is a hack. Make a CircularShadow its own class
	{
		nvgBeginPath(vg);
		float margin = 5.0;
		nvgRect(vg, -margin, -margin, box.size.x + 2*margin, box.size.y + 2*margin);
		nvgFillColor(vg, nvgRGBAf(0.0, 0.0, 0.0, 0.25));
		Vec c = box.size.div(2.0);
		float radius = c.x - 1;
		NVGcolor icol = nvgRGBAf(0.0, 0.0, 0.0, 0.25);
		NVGcolor ocol = nvgRGBAf(0.0, 0.0, 0.0, 0.0);
		NVGpaint paint = nvgRadialGradient(vg, c.x, c.y + 1, radius, radius + 3, icol, ocol);
		nvgFillPaint(vg, paint);
		nvgFill(vg);
	}

	FramebufferWidget::draw(vg);
}

void SVGKnob::onChange() {
	dirty = true;
	ParamWidget::onChange();
}



} // namespace rack
