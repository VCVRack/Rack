#include "widgets.hpp"


namespace rack {


TransformWidget::TransformWidget() {
	identity();
}

Rect TransformWidget::getChildrenBoundingBox() {
	Rect bound = Widget::getChildrenBoundingBox();
	Vec topLeft = bound.pos;
	Vec bottomRight = bound.getBottomRight();
	nvgTransformPoint(&topLeft.x, &topLeft.y, transform, topLeft.x, topLeft.y);
	nvgTransformPoint(&bottomRight.x, &bottomRight.y, transform, bottomRight.x, bottomRight.y);
	return Rect(topLeft, bottomRight.minus(topLeft));
}

void TransformWidget::identity() {
	nvgTransformIdentity(transform);
}

void TransformWidget::translate(Vec delta) {
	float t[6];
	nvgTransformTranslate(t, delta.x, delta.y);
	nvgTransformPremultiply(transform, t);
}

void TransformWidget::rotate(float angle) {
	float t[6];
	nvgTransformRotate(t, angle);
	nvgTransformPremultiply(transform, t);
}

void TransformWidget::scale(Vec s) {
	float t[6];
	nvgTransformScale(t, s.x, s.y);
	nvgTransformPremultiply(transform, t);
}

void TransformWidget::draw(NVGcontext *vg) {
	// No need to save the state because that is done in the parent
	nvgTransform(vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);
	Widget::draw(vg);
}


} // namespace rack
