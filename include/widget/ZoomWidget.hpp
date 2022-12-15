#pragma once
#include <widget/Widget.hpp>


namespace rack {
namespace widget {


/** Resizes the scale of appearance and PositionEvents of children. */
struct ZoomWidget : Widget {
	/** Use setZoom() and getZoom() instead of using this variable directly. */
	float zoom = 1.f;

	math::Vec getRelativeOffset(math::Vec v, Widget* ancestor) override;
	float getRelativeZoom(Widget* ancestor) override;
	math::Rect getViewport(math::Rect r) override;
	float getZoom();
	/** Sets zoom scale and triggers DirtyEvent recursively if scale is changed, so children FramebufferWidgets are redrawn. */
	void setZoom(float zoom);
	void draw(const DrawArgs& args) override;
	void drawLayer(const DrawArgs& args, int layer) override;

	void onHover(const HoverEvent& e) override {
		HoverEvent e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHover(e2);
	}
	void onButton(const ButtonEvent& e) override {
		ButtonEvent e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onButton(e2);
	}
	void onHoverKey(const HoverKeyEvent& e) override {
		HoverKeyEvent e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverKey(e2);
	}
	void onHoverText(const HoverTextEvent& e) override {
		HoverTextEvent e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverText(e2);
	}
	void onHoverScroll(const HoverScrollEvent& e) override {
		HoverScrollEvent e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onHoverScroll(e2);
	}
	void onDragHover(const DragHoverEvent& e) override {
		DragHoverEvent e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onDragHover(e2);
	}
	void onPathDrop(const PathDropEvent& e) override {
		PathDropEvent e2 = e;
		e2.pos = e.pos.div(zoom);
		Widget::onPathDrop(e2);
	}
};


} // namespace widget
} // namespace rack
