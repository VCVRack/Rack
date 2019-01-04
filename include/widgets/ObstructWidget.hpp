#pragma once
#include "widgets/Widget.hpp"


namespace rack {


/** Widget that consumes recursing events without giving a chance for children to consume.
*/
struct ObstructWidget : virtual Widget {
	void onHover(const event::Hover &e) override {
		e.consume(this);
	}
	void onButton(const event::Button &e) override {
		e.consume(this);
	}
	void onHoverKey(const event::HoverKey &e) override {
		e.consume(this);
	}
	void onHoverText(const event::HoverText &e) override {
		e.consume(this);
	}
	void onHoverScroll(const event::HoverScroll &e) override {
		e.consume(this);
	}
	void onDragHover(const event::DragHover &e) override {
		e.consume(this);
	}
	void onPathDrop(const event::PathDrop &e) override {
		e.consume(this);
	}
};


} // namespace rack
