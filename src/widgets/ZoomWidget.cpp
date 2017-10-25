#include "widgets.hpp"


namespace rack {

Rect ZoomWidget::getViewport(Rect r) {
	r.pos = r.pos.mult(zoom);
	r.size = r.size.mult(zoom);
	r = Widget::getViewport(r);
	r.pos = r.pos.div(zoom);
	r.size = r.size.div(zoom);
	return r;
}

void ZoomWidget::setZoom(float zoom) {
	if (zoom != this->zoom)
		onZoom();
	this->zoom = zoom;
}

void ZoomWidget::draw(NVGcontext *vg) {
	nvgScale(vg, zoom, zoom);
	Widget::draw(vg);
}

Widget *ZoomWidget::onMouseDown(Vec pos, int button) {
	return Widget::onMouseDown(pos.div(zoom), button);
}

Widget *ZoomWidget::onMouseUp(Vec pos, int button) {
	return Widget::onMouseUp(pos.div(zoom), button);
}

Widget *ZoomWidget::onMouseMove(Vec pos, Vec mouseRel) {
	return Widget::onMouseMove(pos.div(zoom), mouseRel);
}

Widget *ZoomWidget::onHoverKey(Vec pos, int key) {
	return Widget::onHoverKey(pos.div(zoom), key);
}

Widget *ZoomWidget::onScroll(Vec pos, Vec scrollRel) {
	return Widget::onScroll(pos.div(zoom), scrollRel);
}


} // namespace rack
