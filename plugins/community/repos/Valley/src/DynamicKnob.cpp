#include "ValleyWidgets.hpp"

DynamicKnob::DynamicKnob() {
    shadow = new CircularShadow();
	addChild(shadow);
	shadow->box.size = Vec();

	tw = new TransformWidget();
	addChild(tw);
	sw = new SVGWidget();
	tw->addChild(sw);
    _visibility = nullptr;
    _viewMode = ACTIVE_HIGH_VIEW;
}

void DynamicKnob::setSVG(std::shared_ptr<SVG> svg) {
	sw->svg = svg;
	sw->wrap();
	tw->box.size = sw->box.size;
	box.size = sw->box.size;
    shadow->box.size = sw->box.size;
	shadow->box.pos = Vec(0, sw->box.size.y * 0.1);
}

void DynamicKnob::step() {
	// Re-transform TransformWidget if dirty
    if(_visibility != nullptr) {
        if(*_visibility) {
            visible = true;
        }
        else {
            visible = false;
        }
        if(_viewMode == ACTIVE_LOW_VIEW) {
            visible = !visible;
        }
    }
    else {
        visible = true;
    }
	if (dirty) {
		tw->box.size = box.size;
        float angle;
		if (isfinite(minValue) && isfinite(maxValue)) {
			angle = rescale(value, minValue, maxValue, minAngle, maxAngle);
		}
		else {
			angle = rescale(value, -1.0, 1.0, minAngle, maxAngle);
			angle = fmodf(angle, 2*M_PI);
		}
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

void DynamicKnob::onChange(EventChange &e) {
	dirty = true;
	Knob::onChange(e);
}
