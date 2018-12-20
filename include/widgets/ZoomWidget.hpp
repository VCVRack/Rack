#pragma once
#include "widgets/Widget.hpp"


namespace rack {


struct ZoomWidget : virtual Widget {
	float zoom = 1.f;

	math::Vec getRelativeOffset(math::Vec v, Widget *relative) override {
		return Widget::getRelativeOffset(v.mult(zoom), relative);
	}

	math::Rect getViewport(math::Rect r) override {
		r.pos = r.pos.mult(zoom);
		r.size = r.size.mult(zoom);
		r = Widget::getViewport(r);
		r.pos = r.pos.div(zoom);
		r.size = r.size.div(zoom);
		return r;
	}

	void setZoom(float zoom) {
		if (zoom != this->zoom) {
			event::Zoom eZoom;
			Widget::onZoom(eZoom);
		}
		this->zoom = zoom;
	}

	void draw(NVGcontext *vg) override {
		// No need to save the state because that is done in the parent
		nvgScale(vg, zoom, zoom);
		Widget::draw(vg);
	}

	void onHover(event::Hover &e) override {
		e.pos = e.pos.div(zoom);
		Widget::onHover(e);
	}
	void onButton(event::Button &e) override {
		e.pos = e.pos.div(zoom);
		Widget::onButton(e);
	}
	void onHoverKey(event::HoverKey &e) override {
		e.pos = e.pos.div(zoom);
		Widget::onHoverKey(e);
	}
	void onHoverText(event::HoverText &e) override {
		e.pos = e.pos.div(zoom);
		Widget::onHoverText(e);
	}
	void onHoverScroll(event::HoverScroll &e) override {
		e.pos = e.pos.div(zoom);
		Widget::onHoverScroll(e);
	}
	void onDragHover(event::DragHover &e) override {
		e.pos = e.pos.div(zoom);
		Widget::onDragHover(e);
	}
	void onPathDrop(event::PathDrop &e) override {
		e.pos = e.pos.div(zoom);
		Widget::onPathDrop(e);
	}
};


} // namespace rack
