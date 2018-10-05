#pragma once
#include <vector>
#include "widgets/Widget.hpp"


namespace rack {


struct EventWidget;


namespace event {


struct Event {
	/** Set this to the Widget that consumes (responds to) the event.
	This stops propagation of the event if applicable.
	*/
	Widget *target = NULL;

	virtual ~Event() {}
	/** Triggers the event on an EventWidget.
	Calls the appropriate `EventWidget::on()` method.
	*/
	virtual void trigger(EventWidget *w) = 0;
};


struct Position {
	/** The pixel coordinate where the event occurred, relative to the Widget it is called on. */
	math::Vec pos;
};


struct Key {
	/** GLFW_KEY_* */
	int key;
	/** GLFW_KEY_*. You should usually use `key` instead. */
	int scancode;
	/** GLFW_RELEASE, GLFW_PRESS, or GLFW_REPEAT */
	int action;
	/** GLFW_MOD_* */
	int mods;
};


struct Text {
	/** Unicode code point of the character */
	int codepoint;
};


#define EVENT_TRIGGER_DECLARATION() void trigger(EventWidget *w) override
#define EVENT_TRIGGER_DEFINITION(_event) inline void _event::trigger(EventWidget *w) { w->on(*this); }


/** Occurs every frame when the mouse is hovering over a Widget.
Recurses until consumed.
If target is set, other events may occur on that Widget.
*/
struct Hover : Event, Position {
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs each mouse button press or release.
Recurses until consumed.
If target is set, other events may occur on that Widget.
*/
struct Button : Event, Position {
	/** GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE, etc. */
	int button;
	/** GLFW_PRESS or GLFW_RELEASE */
	int action;
	/** GLFW_MOD_* */
	int mods;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a key is pressed while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverKey : Event, Position, Key {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a character is typed while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverText : Event, Position, Text {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when the mouse scroll wheel is moved while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverScroll : Event, Position {
	/** Change of scroll wheel position. */
	math::Vec scrollDelta;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a Widget begins consuming the Hover event.
*/
struct Enter : Event {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a different Widget is entered.
*/
struct Leave : Event {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a Widget begins consuming the Button press event.
*/
struct Select : Event {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a different Widget is selected.
*/
struct Deselect : Event {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a key is pressed while a Widget is selected.
*/
struct SelectKey : Event, Key {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when text is typed while a Widget is selected.
*/
struct SelectText : Event, Text {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a Widget begins being dragged.
Must consume to allow the drag to occur.
*/
struct DragStart : Event {
	int button;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a Widget stops being dragged by releasing the mouse button.
*/
struct DragEnd : Event {
	int button;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a dragged Widget is moved.
Called once per frame, even when mouseDelta is zero.
*/
struct DragMove : Event {
	int button;
	math::Vec mouseDelta;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when the mouse enters a Widget while dragging.
*/
struct DragEnter : Event {
	Widget *origin = NULL;
	int button;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when the mouse leaves a Widget while dragging.
*/
struct DragLeave : Event {
	Widget *origin = NULL;
	int button;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when the mouse button is released over a Widget while dragging.
*/
struct DragDrop : Event {
	Widget *origin = NULL;
	int button;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when a selection of files from the operating system are dropped onto a Widget.
*/
struct PathDrop : Event, Position {
	/** List of file paths in the dropped selection */
	std::vector<std::string> paths;
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when an certain action is triggered on a Widget.
*/
struct Action : Event {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when the value of a Widget changes.
*/
struct Change : Event {
	EVENT_TRIGGER_DECLARATION();
};


/** Occurs when the zoom level of a Widget is changed.
Recurses.
*/
struct Zoom : Event {
	EVENT_TRIGGER_DECLARATION();
};


} // namespace event
} // namespace rack
