#pragma once
#include <widget/Widget.hpp>


namespace rack {
namespace widget {


/** A Widget with a dynamic zoom level. */
struct ZoomWidget : Widget {
	float zoom = 1.f;

	math::Vec getRelativeOffset(math::Vec v, Widget* relative) override;
	math::Rect getViewport(math::Rect r) override;
	void setZoom(float zoom);
	void draw(const DrawArgs& args) override;

	void onHover(const event::Hover& e) override {
		event::Hover e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHover(e2);
	}
	void onButton(const event::Button& e) override {
		event::Button e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onButton(e2);
	}
	void onHoverKey(const event::HoverKey& e) override {
		event::HoverKey e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverKey(e2);
	}
	void onHoverText(const event::HoverText& e) override {
		event::HoverText e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverText(e2);
	}
	void onHoverScroll(const event::HoverScroll& e) override {
		event::HoverScroll e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverScroll(e2);
	}
	void onDragHover(const event::DragHover& e) override {
		event::DragHover e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onDragHover(e2);
	}
	void onPathDrop(const event::PathDrop& e) override {
		event::PathDrop e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onPathDrop(e2);
	}
};


} // namespace widget
} // namespace rack
