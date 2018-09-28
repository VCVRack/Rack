#pragma once
#include "widgets/EventWidget.hpp"


namespace rack {


/** Widget that consumes recursing events but gives a chance for children to consume first.
You can of course override the events.
You may also call OpaqueWidget::on() from the overridden method to continue recursing/consuming the event.
*/
struct OpaqueWidget : virtual EventWidget {
	template <class TEvent>
	void consumeEvent(TEvent &e) {
		EventWidget::on(e);
		if (!e.target) {
			e.target = this;
		}
	}

	void on(event::Hover &e) override {consumeEvent(e);}
	void on(event::Button &e) override {consumeEvent(e);}
	void on(event::HoverKey &e) override {consumeEvent(e);}
	void on(event::HoverText &e) override {consumeEvent(e);}
	// void on(event::HoverScroll &e) override {consumeEvent(e);}
	void on(event::PathDrop &e) override {consumeEvent(e);}
};


} // namespace rack
