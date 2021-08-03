#pragma once
#include <widget/Widget.hpp>


namespace rack {
namespace widget {


/** Transforms appearance only, not positions of events */
struct TransformWidget : Widget {
	/** The transformation matrix */
	float transform[6];

	TransformWidget() {
		identity();
	}

	void identity() {
		nvgTransformIdentity(transform);
	}

	void translate(math::Vec delta) {
		float t[6];
		nvgTransformTranslate(t, VEC_ARGS(delta));
		nvgTransformPremultiply(transform, t);
	}

	void rotate(float angle) {
		float t[6];
		nvgTransformRotate(t, angle);
		nvgTransformPremultiply(transform, t);
	}

	void rotate(float angle, math::Vec origin) {
		translate(origin);
		rotate(angle);
		translate(origin.neg());
	}

	void scale(math::Vec s) {
		float t[6];
		nvgTransformScale(t, s.x, s.y);
		nvgTransformPremultiply(transform, t);
	}

	void draw(const DrawArgs& args) override {
		// No need to save the state because that is done in the parent
		nvgTransform(args.vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);
		Widget::draw(args);
	}
};


} // namespace widget
} // namespace rack
