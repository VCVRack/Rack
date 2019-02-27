#pragma once
#include "common.hpp"
#include "math.hpp"
#include <vector>


namespace rack {
namespace widget {


struct Widget;


/** A per-event state shared and writable by all widgets that recursively handle an event. */
struct EventContext {
	/** The Widget that consumes the event.
	This stops propagation of the event if applicable.
	*/
	Widget *consumed = NULL;
};


/** Base class for all events. */
struct Event {
	EventContext *context = NULL;

	void consume(Widget *w) const {
		if (context)
			context->consumed = w;
	}
	Widget *getConsumed() const {
		return context ? context->consumed : NULL;
	}
};


/** An Event prototype with a vector position. */
struct PositionEvent {
	/** The pixel coordinate where the event occurred, relative to the Widget it is called on. */
	math::Vec pos;
};


/** An Event prototype with a GLFW key. */
struct KeyEvent {
	/** GLFW_KEY_* */
	int key;
	/** GLFW_KEY_*. You should usually use `key` instead. */
	int scancode;
	/** GLFW_RELEASE, GLFW_PRESS, or GLFW_REPEAT */
	int action;
	/** GLFW_MOD_* */
	int mods;
};


/** An Event prototype with a Unicode character. */
struct TextEvent {
	/** Unicode code point of the character */
	int codepoint;
};


// Concrete events


/** Occurs every frame when the mouse is hovering over a Widget.
Recurses until consumed.
If `target` is set, other events may occur on that Widget.
*/
struct HoverEvent : Event, PositionEvent {
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};


/** Occurs each mouse button press or release.
Recurses until consumed.
If `target` is set, other events may occur on that Widget.
*/
struct ButtonEvent : Event, PositionEvent {
	/** GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE, etc. */
	int button;
	/** GLFW_PRESS or GLFW_RELEASE */
	int action;
	/** GLFW_MOD_* */
	int mods;
};


/** Occurs when the left mouse button is pressed the second time within a time and position window.
*/
struct DoubleClickEvent : Event {
};


/** Occurs when a key is pressed, released, or repeated while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverKeyEvent : Event, PositionEvent, KeyEvent {
};


/** Occurs when a character is typed while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverTextEvent : Event, PositionEvent, TextEvent {
};


/** Occurs when the mouse scroll wheel is moved while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverScrollEvent : Event, PositionEvent {
	/** Change of scroll wheel position. */
	math::Vec scrollDelta;
};


/** Occurs when a Widget begins consuming the Hover event.
Must consume to set the widget as hovered.
*/
struct EnterEvent : Event {
};


/** Occurs when a different Widget is entered.
*/
struct LeaveEvent : Event {
};


/** Occurs when a Widget begins consuming the Button press event.
Must consume to set the widget as selected.
*/
struct SelectEvent : Event {
};


/** Occurs when a different Widget is selected.
*/
struct DeselectEvent : Event {
};


/** Occurs when a key is pressed, released, or repeated while a Widget is selected.
If consumed, a HoverKey event will not be triggered.
*/
struct SelectKeyEvent : Event, KeyEvent {
};


/** Occurs when text is typed while a Widget is selected.
If consumed, a HoverText event will not be triggered.
*/
struct SelectTextEvent : Event, TextEvent {
};


/** Occurs when a Widget begins being dragged.
Must consume to set the widget as dragged.
*/
struct DragStartEvent : Event {
};


/** Occurs when a Widget stops being dragged by releasing the mouse button.
*/
struct DragEndEvent : Event {
};


/** Occurs every frame on the dragged Widget.
*/
struct DragMoveEvent : Event {
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};


/** Occurs every frame when the mouse is hovering over a Widget while another Widget (possibly the same one) is being dragged.
Recurses until consumed.
*/
struct DragHoverEvent : Event, PositionEvent {
	/** The dragged widget */
	Widget *origin = NULL;
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};

/** Occurs when the mouse enters a Widget while dragging.
Must consume to set the widget as drag-hovered.
*/
struct DragEnterEvent : Event {
	/** The dragged widget */
	Widget *origin = NULL;
};


/** Occurs when the mouse leaves a Widget while dragging.
*/
struct DragLeaveEvent : Event {
	/** The dragged widget */
	Widget *origin = NULL;
};


/** Occurs when the mouse button is released over a Widget while dragging.
*/
struct DragDropEvent : Event {
	/** The dragged widget */
	Widget *origin = NULL;
};


/** Occurs when a selection of files from the operating system is dropped onto a Widget.
Recurses until consumed.
*/
struct PathDropEvent : Event, PositionEvent {
	PathDropEvent(const std::vector<std::string> &paths) : paths(paths) {}

	/** List of file paths in the dropped selection */
	const std::vector<std::string> &paths;
};


/** Occurs after a certain action is triggered on a Widget.
The concept of an "action" is dependent on the type of Widget.
*/
struct ActionEvent : Event {
};


/** Occurs after the value of a Widget changes.
The concept of a "value" is dependent on the type of Widget.
*/
struct ChangeEvent : Event {
};


/** Occurs after the zoom level of a Widget is changed.
Recurses until consumed.
*/
struct ZoomEvent : Event {
};


/** Occurs after a Widget's position is set by Widget::setPos().
*/
struct RepositionEvent : Event {
};


/** Occurs after a Widget's size is set by Widget::setSize().
*/
struct ResizeEvent : Event {
};


/** Occurs after a Widget is added to a parent.
*/
struct AddEvent : Event {
};


/** Occurs before a Widget is removed from its parent.
*/
struct RemoveEvent : Event {
};


/** Occurs after a Widget is shown with Widget::show().
*/
struct ShowEvent : Event {
};


/** Occurs after a Widget is hidden with Widget::hide().
*/
struct HideEvent : Event {
};


struct EventState {
	Widget *rootWidget = NULL;
	/** State widgets
	Don't set these directly unless you know what you're doing. Use the set*() methods instead.
	*/
	Widget *hoveredWidget = NULL;
	Widget *draggedWidget = NULL;
	Widget *dragHoveredWidget = NULL;
	Widget *selectedWidget = NULL;
	/** For double-clicking */
	double lastClickTime = -INFINITY;
	Widget *lastClickedWidget = NULL;

	void setHovered(Widget *w);
	void setDragged(Widget *w);
	void setDragHovered(Widget *w);
	void setSelected(Widget *w);
	/** Prepares a widget for deletion */
	void finalizeWidget(Widget *w);

	void handleButton(math::Vec pos, int button, int action, int mods);
	void handleHover(math::Vec pos, math::Vec mouseDelta);
	void handleLeave();
	void handleScroll(math::Vec pos, math::Vec scrollDelta);
	void handleText(math::Vec pos, int codepoint);
	void handleKey(math::Vec pos, int key, int scancode, int action, int mods);
	void handleDrop(math::Vec pos, const std::vector<std::string> &paths);
	void handleZoom();
};


} // namespace widget
} // namespace rack
