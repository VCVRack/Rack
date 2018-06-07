#pragma once
#include <list>

#include "util/math.hpp"


namespace rack {


struct Widget;


struct Event {
	/** Set this to true to signal that no other widgets should receive the event */
	bool consumed = false;
};

struct EventPosition : Event {
	Vec pos;
};

///////////

struct EventMouseDown : EventPosition {
	int button;
	/** The widget which responded to the click. Set it to `this` if consumed. */
	Widget *target = NULL;
};

struct EventMouseUp : EventPosition {
	/** 0 for left mouse button, 1 for right, 2 for middle */
	int button;
	Widget *target = NULL;
};

struct EventMouseMove : EventPosition {
	Vec mouseRel;
	Widget *target = NULL;
};

struct EventHoverKey : EventPosition {
	int key;
	Widget *target = NULL;
};

struct EventMouseEnter : Event {
};

struct EventMouseLeave : Event {
};

struct EventFocus : Event {
};

struct EventDefocus : Event {
};

struct EventText : Event {
	int codepoint;
};

struct EventKey : Event {
	int key;
};

struct EventScroll : EventPosition {
	Vec scrollRel;
};

/////////////

struct EventDragStart : Event {
};

struct EventDragEnd : Event {
};

struct EventDragMove : Event {
	Vec mouseRel;
};

struct EventDragEnter : Event {
	Widget *origin = NULL;
};

struct EventDragLeave : Event {
	Widget *origin = NULL;
};

struct EventDragDrop : Event {
	Widget *origin = NULL;
};

struct EventPathDrop : EventPosition {
	std::list<std::string> paths;
};

struct EventAction : Event {
};

struct EventChange : Event {
};

struct EventZoom : Event {
};


} // namespace rack
