#pragma once
#include "widgets/Widget.hpp"
#include "event.hpp"


namespace rack {


/** A widget that responds to events */
struct EventWidget : Widget {
	void handleEvent(event::Event &e) override {
		e.trigger(this);
	}

	template <class TEvent>
	void recurseEvent(TEvent &e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			Widget *child = *it;
			if (!child->visible)
				continue;
			if (!child->box.contains(e.pos))
				continue;
			TEvent e2 = e;
			e2.pos = e.pos.minus(child->box.pos);
			child->handleEvent(e2);
			if (e2.target) {
				e.target = e.target;
				break;
			}
		}
	}

	/** Override these event callbacks to respond to events.
	See events.hpp for a description of each event.
	*/
	virtual void on(event::Hover &e) {recurseEvent(e);}
	virtual void on(event::Button &e) {recurseEvent(e);}
	virtual void on(event::HoverKey &e) {recurseEvent(e);}
	virtual void on(event::HoverText &e) {recurseEvent(e);}
	virtual void on(event::HoverScroll &e) {recurseEvent(e);}
	virtual void on(event::Enter &e) {}
	virtual void on(event::Leave &e) {}
	virtual void on(event::Select &e) {}
	virtual void on(event::Deselect &e) {}
	virtual void on(event::SelectKey &e) {}
	virtual void on(event::SelectText &e) {}
	virtual void on(event::DragStart &e) {}
	virtual void on(event::DragEnd &e) {}
	virtual void on(event::DragMove &e) {}
	virtual void on(event::DragEnter &e) {}
	virtual void on(event::DragLeave &e) {}
	virtual void on(event::DragDrop &e) {}
	virtual void on(event::PathDrop &e) {recurseEvent(e);}
	virtual void on(event::Action &e) {}
	virtual void on(event::Change &e) {}
	virtual void on(event::Zoom &e) {}
};


/** These definitions simply call each `EventWidget::on()` function above.
They need to be defined here because EventWidget is not defined at the time of each event's definition.
*/
EVENT_TRIGGER_DEFINITION(event::Hover)
EVENT_TRIGGER_DEFINITION(event::Button)
EVENT_TRIGGER_DEFINITION(event::HoverKey)
EVENT_TRIGGER_DEFINITION(event::HoverText)
EVENT_TRIGGER_DEFINITION(event::HoverScroll)
EVENT_TRIGGER_DEFINITION(event::Enter)
EVENT_TRIGGER_DEFINITION(event::Leave)
EVENT_TRIGGER_DEFINITION(event::Select)
EVENT_TRIGGER_DEFINITION(event::Deselect)
EVENT_TRIGGER_DEFINITION(event::SelectKey)
EVENT_TRIGGER_DEFINITION(event::SelectText)
EVENT_TRIGGER_DEFINITION(event::DragStart)
EVENT_TRIGGER_DEFINITION(event::DragEnd)
EVENT_TRIGGER_DEFINITION(event::DragMove)
EVENT_TRIGGER_DEFINITION(event::DragEnter)
EVENT_TRIGGER_DEFINITION(event::DragLeave)
EVENT_TRIGGER_DEFINITION(event::DragDrop)
EVENT_TRIGGER_DEFINITION(event::PathDrop)
EVENT_TRIGGER_DEFINITION(event::Action)
EVENT_TRIGGER_DEFINITION(event::Change)
EVENT_TRIGGER_DEFINITION(event::Zoom)


} // namespace rack
