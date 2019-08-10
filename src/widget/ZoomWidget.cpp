#include <widget/ZoomWidget.hpp>


namespace rack {
namespace widget {


math::Vec ZoomWidget::getRelativeOffset(math::Vec v, Widget* relative) {
	return Widget::getRelativeOffset(v.mult(zoom), relative);
}

math::Rect ZoomWidget::getViewport(math::Rect r) {
	r.pos = r.pos.mult(zoom);
	r.size = r.size.mult(zoom);
	r = Widget::getViewport(r);
	r.pos = r.pos.div(zoom);
	r.size = r.size.div(zoom);
	return r;
}

void ZoomWidget::setZoom(float zoom) {
	if (zoom == this->zoom)
		return;
	this->zoom = zoom;

	event::Context cZoom;
	event::Zoom eZoom;
	eZoom.context = &cZoom;
	Widget::onZoom(eZoom);
}

void ZoomWidget::draw(const DrawArgs& args) {
	DrawArgs zoomCtx = args;
	zoomCtx.clipBox.pos = zoomCtx.clipBox.pos.div(zoom);
	zoomCtx.clipBox.size = zoomCtx.clipBox.size.div(zoom);
	// No need to save the state because that is done in the parent
	nvgScale(args.vg, zoom, zoom);
	Widget::draw(zoomCtx);
}


} // namespace widget
} // namespace rack
