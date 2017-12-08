#include "widgets.hpp"


namespace rack {


Vec ZoomWidget::getRelativeOffset(Vec v, Widget *relative) {
	return Widget::getRelativeOffset(v.mult(zoom), relative);
}

Rect ZoomWidget::getViewport(Rect r) {
	r.pos = r.pos.mult(zoom);
	r.size = r.size.mult(zoom);
	r = Widget::getViewport(r);
	r.pos = r.pos.div(zoom);
	r.size = r.size.div(zoom);
	return r;
}

void ZoomWidget::setZoom(float zoom) {
	if (zoom != this->zoom) {
		EventZoom e;
		onZoom(e);
	}
	this->zoom = zoom;
}

void ZoomWidget::draw(NVGcontext *vg) {
	nvgScale(vg, zoom, zoom);
	Widget::draw(vg);
}

void ZoomWidget::onMouseDown(EventMouseDown &e) {
	Vec pos = e.pos;
	e.pos = e.pos.div(zoom);
	Widget::onMouseDown(e);
	e.pos = pos;
}

void ZoomWidget::onMouseUp(EventMouseUp &e) {
	Vec pos = e.pos;
	e.pos = e.pos.div(zoom);
	Widget::onMouseUp(e);
	e.pos = pos;
}

void ZoomWidget::onMouseMove(EventMouseMove &e) {
	Vec pos = e.pos;
	e.pos = e.pos.div(zoom);
	Widget::onMouseMove(e);
	e.pos = pos;
}

void ZoomWidget::onHoverKey(EventHoverKey &e) {
	Vec pos = e.pos;
	e.pos = e.pos.div(zoom);
	Widget::onHoverKey(e);
	e.pos = pos;
}

void ZoomWidget::onScroll(EventScroll &e) {
	Vec pos = e.pos;
	e.pos = e.pos.div(zoom);
	Widget::onScroll(e);
	e.pos = pos;
}

void ZoomWidget::onPathDrop(EventPathDrop &e) {
	Vec pos = e.pos;
	e.pos = e.pos.div(zoom);
	Widget::onPathDrop(e);
	e.pos = pos;
}


} // namespace rack
