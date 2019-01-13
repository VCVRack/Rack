#pragma once
#include "widgets/Widget.hpp"


namespace rack {


/** Widget that does not respond to events and does not pass events to children */
struct TransparentWidget : Widget {
	/** Override behavior to do nothing instead. */
	void onHover(const event::Hover &e) override {}
	void onButton(const event::Button &e) override {}
	void onHoverKey(const event::HoverKey &e) override {}
	void onHoverText(const event::HoverText &e) override {}
	void onHoverScroll(const event::HoverScroll &e) override {}
	void onDragHover(const event::DragHover &e) override {}
	void onPathDrop(const event::PathDrop &e) override {}
};


} // namespace rack
