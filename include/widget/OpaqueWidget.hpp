#pragma once
#include "widget/Widget.hpp"


namespace rack {
namespace widget {


/** A Widget that consumes recursing events but gives a chance for children to consume first.
You can of course override the events.
You may also call OpaqueWidget::on*() from the overridden method to continue recursing/consuming the event.
*/
struct OpaqueWidget : Widget {
	void onHover(const HoverEvent &e) override {
		Widget::onHover(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onButton(const ButtonEvent &e) override {
		Widget::onButton(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onHoverKey(const HoverKeyEvent &e) override {
		Widget::onHoverKey(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onHoverText(const HoverTextEvent &e) override {
		Widget::onHoverText(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onDragHover(const DragHoverEvent &e) override {
		Widget::onDragHover(e);
		if (!e.getConsumed())
			e.consume(this);
	}
};


} // namespace widget
} // namespace rack
