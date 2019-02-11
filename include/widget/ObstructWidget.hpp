#pragma once
#include "widget/Widget.hpp"


namespace rack {
namespace widget {


/** A Widget that consumes recursing events without giving a chance for children to consume.
*/
struct ObstructWidget : Widget {
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
	void onDragHover(const event::DragHover &e) override {
		e.consume(this);
	}
};


} // namespace widget
} // namespace rack
