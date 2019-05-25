#pragma once
#include <widget/Widget.hpp>


namespace rack {
namespace widget {


/** A Widget that stops propagation of all recursive PositionEvents but gives a chance for children to consume first.
Also consumes Hover and Button for left-clicks.
*/
struct OpaqueWidget : Widget {
	void onHover(const event::Hover &e) override {
		Widget::onHover(e);
		e.stopPropagating();
		// Consume if not consumed by child
		if (!e.isConsumed())
			e.consume(this);
	}
	void onButton(const event::Button &e) override {
		Widget::onButton(e);
		e.stopPropagating();
		if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
			// Consume if not consumed by child
			if (!e.isConsumed())
				e.consume(this);
		}
	}
	void onHoverKey(const event::HoverKey &e) override {
		Widget::onHoverKey(e);
		e.stopPropagating();
	}
	void onHoverText(const event::HoverText &e) override {
		Widget::onHoverText(e);
		e.stopPropagating();
	}
	void onHoverScroll(const event::HoverScroll &e) override {
		Widget::onHoverScroll(e);
		e.stopPropagating();
	}
	void onDragHover(const event::DragHover &e) override {
		Widget::onDragHover(e);
		e.stopPropagating();
		// Consume if not consumed by child
		if (!e.isConsumed())
			e.consume(this);
	}
	void onPathDrop(const event::PathDrop &e) override {
		Widget::onPathDrop(e);
		e.stopPropagating();
	}
};


} // namespace widget
} // namespace rack
