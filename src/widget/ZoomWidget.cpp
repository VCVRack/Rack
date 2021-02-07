#include <widget/ZoomWidget.hpp>


namespace rack {
namespace widget {


math::Vec ZoomWidget::getRelativeOffset(math::Vec v, Widget* relative) {
	// Transform `v` (which is in child coordinates) to local coordinates.
	v = v.mult(zoom);
	return Widget::getRelativeOffset(v, relative);
}


float ZoomWidget::getRelativeZoom(Widget* relative) {
	return zoom * Widget::getRelativeZoom(relative);
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

	// Trigger Dirty event
	widget::EventContext cDirty;
	DirtyEvent eDirty;
	eDirty.context = &cDirty;
	Widget::onDirty(eDirty);
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
