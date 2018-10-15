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
		if (!e.target)
			e.target = this;
	}
	void onButton(event::Button &e) override {
		Widget::onButton(e);
		if (!e.target)
			e.target = this;
	}
	void onHoverKey(event::HoverKey &e) override {
		Widget::onHoverKey(e);
		if (!e.target)
			e.target = this;
	}
	void onHoverText(event::HoverText &e) override {
		Widget::onHoverText(e);
		if (!e.target)
			e.target = this;
	}
	void onHoverScroll(event::HoverScroll &e) override {
		Widget::onHoverScroll(e);
		if (!e.target)
			e.target = this;
	}
	void onDragHover(event::DragHover &e) override {
		Widget::onDragHover(e);
		if (!e.target)
			e.target = this;
	}
	void onPathDrop(event::PathDrop &e) override {
		Widget::onPathDrop(e);
		if (!e.target)
			e.target = this;
	}
};


} // namespace rack
