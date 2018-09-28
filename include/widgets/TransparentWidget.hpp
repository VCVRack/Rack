#pragma once
#include "widgets/EventWidget.hpp"


namespace rack {


/** Widget that does not respond to events and does not pass events to children */
struct TransparentWidget : virtual EventWidget {
	/** Override behavior to do nothing instead. */
	void on(event::Hover &e) override {}
	void on(event::Button &e) override {}
	void on(event::HoverKey &e) override {}
	void on(event::HoverText &e) override {}
	void on(event::HoverScroll &e) override {}
	void on(event::PathDrop &e) override {}
};


} // namespace rack
