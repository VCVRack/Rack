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
		this->zoom = zoom;

		event::Context eZoomContext;
		event::Zoom eZoom;
		eZoom.context = &eZoomContext;
		Widget::onZoom(eZoom);
	}

	void draw(NVGcontext *vg) override {
		// No need to save the state because that is done in the parent
		nvgScale(vg, zoom, zoom);
		Widget::draw(vg);
	}

	void onHover(const event::Hover &e) override {
		event::Hover e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHover(e2);
	}
	void onButton(const event::Button &e) override {
		event::Button e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onButton(e2);
	}
	void onHoverKey(const event::HoverKey &e) override {
		event::HoverKey e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverKey(e2);
	}
	void onHoverText(const event::HoverText &e) override {
		event::HoverText e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverText(e2);
	}
	void onHoverScroll(const event::HoverScroll &e) override {
		event::HoverScroll e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverScroll(e2);
	}
	void onDragHover(const event::DragHover &e) override {
		event::DragHover e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onDragHover(e2);
	}
	void onPathDrop(const event::PathDrop &e) override {
		event::PathDrop e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onPathDrop(e2);
	}
};


} // namespace rack
