#pragma once
#include "widget/Widget.hpp"


namespace rack {
namespace widget {


/** A Widget that stops propagation of all recursive PositionEvents but gives a chance for children to consume first.
Remember to call these methods in your subclass if you wish to preserve default OpaqueWidget behavior.
*/
struct OpaqueWidget : Widget {
	void onHover(const HoverEvent &e) override {
		Widget::onHover(e);
		e.stopPropagating();
		// Consume if not consumed by child
		if (!e.getTarget())
			e.setTarget(this);
	}
	void onButton(const ButtonEvent &e) override {
		Widget::onButton(e);
		e.stopPropagating();
		// Consume if not consumed by child
		if (!e.getTarget())
			e.setTarget(this);
	}
	void onHoverKey(const HoverKeyEvent &e) override {
		Widget::onHoverKey(e);
		e.stopPropagating();
	}
	void onHoverText(const HoverTextEvent &e) override {
		Widget::onHoverText(e);
		e.stopPropagating();
	}
	void onHoverScroll(const HoverScrollEvent &e) override {
		Widget::onHoverScroll(e);
		e.stopPropagating();
	}
	void onDragHover(const DragHoverEvent &e) override {
		Widget::onDragHover(e);
		e.stopPropagating();
	}
	void onPathDrop(const PathDropEvent &e) override {
		Widget::onPathDrop(e);
		e.stopPropagating();
	}
};


} // namespace widget
} // namespace rack
