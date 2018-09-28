#pragma once
#include "widgets/EventWidget.hpp"


namespace rack {


struct ZoomWidget : virtual EventWidget {
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
			EventWidget::on(eZoom);
		}
		this->zoom = zoom;
	}

	void draw(NVGcontext *vg) override {
		// No need to save the state because that is done in the parent
		nvgScale(vg, zoom, zoom);
		Widget::draw(vg);
	}

	void on(event::Hover &e) override {
		event::Hover e2 = e;
		e2.pos = e.pos.div(zoom);
		EventWidget::on(e2);
	}

	void on(event::Button &e) override {
		event::Button e2 = e;
		e2.pos = e.pos.div(zoom);
		EventWidget::on(e2);
	}

	void on(event::HoverKey &e) override {
		event::HoverKey e2 = e;
		e2.pos = e.pos.div(zoom);
		EventWidget::on(e2);
	}

	void on(event::HoverText &e) override {
		event::HoverText e2 = e;
		e2.pos = e.pos.div(zoom);
		EventWidget::on(e2);
	}

	void on(event::HoverScroll &e) override {
		event::HoverScroll e2 = e;
		e2.pos = e.pos.div(zoom);
		EventWidget::on(e2);
	}

	void on(event::PathDrop &e) override {
		event::PathDrop e2 = e;
		e2.pos = e.pos.div(zoom);
		EventWidget::on(e2);
	}
};


} // namespace rack
