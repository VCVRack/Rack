#pragma once
#include "widgets/Widget.hpp"


namespace rack {


/** Widget that consumes recursing events but gives a chance for children to consume first.
You can of course override the events.
You may also call OpaqueWidget::on*() from the overridden method to continue recursing/consuming the event.
*/
struct OpaqueWidget : virtual Widget {
	void onHover(event::Hover &e) override {
		Widget::onHover(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onButton(event::Button &e) override {
		Widget::onButton(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onHoverKey(event::HoverKey &e) override {
		Widget::onHoverKey(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onHoverText(event::HoverText &e) override {
		Widget::onHoverText(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onHoverScroll(event::HoverScroll &e) override {
		Widget::onHoverScroll(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onDragHover(event::DragHover &e) override {
		Widget::onDragHover(e);
		if (!e.getConsumed())
			e.consume(this);
	}
	void onPathDrop(event::PathDrop &e) override {
		Widget::onPathDrop(e);
		if (!e.getConsumed())
			e.consume(this);
	}
};


} // namespace rack
