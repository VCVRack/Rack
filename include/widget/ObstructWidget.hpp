#pragma once
#include "widget/Widget.hpp"


namespace rack {
namespace widget {


/** A Widget that consumes recursing events without giving a chance for children to consume.
*/
struct ObstructWidget : Widget {
	void onHover(const HoverEvent &e) override {
		e.consume(this);
	}
	void onButton(const ButtonEvent &e) override {
		e.consume(this);
	}
	void onHoverKey(const HoverKeyEvent &e) override {
		e.consume(this);
	}
	void onHoverText(const HoverTextEvent &e) override {
		e.consume(this);
	}
	void onDragHover(const DragHoverEvent &e) override {
		e.consume(this);
	}
};


} // namespace widget
} // namespace rack
