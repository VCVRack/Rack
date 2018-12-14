#pragma once
#include "widgets/Widget.hpp"


namespace rack {


/** Transforms appearance only, not positions of events */
struct TransformWidget : VirtualWidget {
	/** The transformation matrix */
	float transform[6];

	TransformWidget() {
		identity();
	}

	void identity() {
		nvgTransformIdentity(transform);
	}

	void translate(Vec delta) {
		float t[6];
		nvgTransformTranslate(t, delta.x, delta.y);
		nvgTransformPremultiply(transform, t);
	}

	void rotate(float angle) {
		float t[6];
		nvgTransformRotate(t, angle);
		nvgTransformPremultiply(transform, t);
	}

	void scale(Vec s) {
		float t[6];
		nvgTransformScale(t, s.x, s.y);
		nvgTransformPremultiply(transform, t);
	}

	void draw(NVGcontext *vg) override {
		// No need to save the state because that is done in the parent
		nvgTransform(vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);
		Widget::draw(vg);
	}
};


} // namespace rack
