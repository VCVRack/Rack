#pragma once
#include "widgets/Widget.hpp"


namespace rack {


/** Widget that does not respond to events and does not pass events to children */
struct TransparentWidget : virtual Widget {
	/** Override behavior to do nothing instead. */
	void onHover(event::Hover &e) override {}
	void onButton(event::Button &e) override {}
	void onHoverKey(event::HoverKey &e) override {}
	void onHoverText(event::HoverText &e) override {}
	void onHoverScroll(event::HoverScroll &e) override {}
	void onPathDrop(event::PathDrop &e) override {}
};


} // namespace rack
