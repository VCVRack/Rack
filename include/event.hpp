#pragma once
#include "common.hpp"
#include "math.hpp"
#include <vector>


namespace rack {


namespace widget {
	struct Widget;
} // namespace widget


/** Event state machine for Widgets
*/
namespace event {


struct Context {
	/** The Widget that consumes the event.
	This stops propagation of the event if applicable.
	*/
	widget::Widget *consumed = NULL;
};


/** Base event class */
struct Event {
	Context *context = NULL;

	void consume(widget::Widget *w) const {
		if (context)
			context->consumed = w;
	}
	widget::Widget *getConsumed() const {
		return context ? context->consumed : NULL;
	}
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

// Events

struct Text {
	/** Unicode code point of the character */
	int codepoint;
};


/** Occurs every frame when the mouse is hovering over a Widget.
Recurses until consumed.
If `target` is set, other events may occur on that Widget.
*/
struct Hover : Event, Position {
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};


/** Occurs each mouse button press or release.
Recurses until consumed.
If `target` is set, other events may occur on that Widget.
*/
struct Button : Event, Position {
	/** GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE, etc. */
	int button;
	/** GLFW_PRESS or GLFW_RELEASE */
	int action;
	/** GLFW_MOD_* */
	int mods;
};


/** Occurs when the left mouse button is pressed the second time within a time and position window.
*/
struct DoubleClick : Event {
};


/** Occurs when a key is pressed, released, or repeated while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverKey : Event, Position, Key {
};


/** Occurs when a character is typed while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverText : Event, Position, Text {
};


/** Occurs when the mouse scroll wheel is moved while the mouse is hovering a Widget.
Recurses until consumed.
*/
struct HoverScroll : Event, Position {
	/** Change of scroll wheel position. */
	math::Vec scrollDelta;
};


/** Occurs when a Widget begins consuming the Hover event.
Must consume to set the widget as hovered.
*/
struct Enter : Event {
};


/** Occurs when a different Widget is entered.
*/
struct Leave : Event {
};


/** Occurs when a Widget begins consuming the Button press event.
Must consume to set the widget as selected.
*/
struct Select : Event {
};


/** Occurs when a different Widget is selected.
*/
struct Deselect : Event {
};


/** Occurs when a key is pressed, released, or repeated while a Widget is selected.
If consumed, a HoverKey event will not be triggered.
*/
struct SelectKey : Event, Key {
};


/** Occurs when text is typed while a Widget is selected.
If consumed, a HoverText event will not be triggered.
*/
struct SelectText : Event, Text {
};


/** Occurs when a Widget begins being dragged.
Must consume to set the widget as dragged.
*/
struct DragStart : Event {
};


/** Occurs when a Widget stops being dragged by releasing the mouse button.
*/
struct DragEnd : Event {
};


/** Occurs every frame on the dragged Widget.
*/
struct DragMove : Event {
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};


/** Occurs every frame when the mouse is hovering over a Widget while another Widget (possibly the same one) is being dragged.
Recurses until consumed.
*/
struct DragHover : Event, Position {
	/** The dragged widget */
	widget::Widget *origin = NULL;
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};

/** Occurs when the mouse enters a Widget while dragging.
Must consume to set the widget as drag-hovered.
*/
struct DragEnter : Event {
	/** The dragged widget */
	widget::Widget *origin = NULL;
};


/** Occurs when the mouse leaves a Widget while dragging.
*/
struct DragLeave : Event {
	/** The dragged widget */
	widget::Widget *origin = NULL;
};


/** Occurs when the mouse button is released over a Widget while dragging.
*/
struct DragDrop : Event {
	/** The dragged widget */
	widget::Widget *origin = NULL;
};


/** Occurs when a selection of files from the operating system is dropped onto a Widget.
Recurses until consumed.
*/
struct PathDrop : Event, Position {
	PathDrop(const std::vector<std::string> &paths) : paths(paths) {}

	/** List of file paths in the dropped selection */
	const std::vector<std::string> &paths;
};


/** Occurs after a certain action is triggered on a Widget.
The concept of an "action" is dependent on the type of Widget.
*/
struct Action : Event {
};


/** Occurs after the value of a Widget changes.
The concept of a "value" is dependent on the type of Widget.
*/
struct Change : Event {
};


/** Occurs after the zoom level of a Widget is changed.
Recurses until consumed.
*/
struct Zoom : Event {
};


/** Occurs after a Widget's position is set by Widget::setPos().
*/
struct Reposition : Event {
};


/** Occurs after a Widget's size is set by Widget::setSize().
*/
struct Resize : Event {
};


/** Occurs after a Widget is added to a parent.
*/
struct Add : Event {
};


/** Occurs before a Widget is removed from its parent.
*/
struct Remove : Event {
};


/** Occurs after a Widget is shown with Widget::show().
*/
struct Show : Event {
};


/** Occurs after a Widget is hidden with Widget::hide().
*/
struct Hide : Event {
};


struct State {
	widget::Widget *rootWidget = NULL;
	/** State widgets
	Don't set these directly unless you know what you're doing. Use the set*() methods instead.
	*/
	widget::Widget *hoveredWidget = NULL;
	widget::Widget *draggedWidget = NULL;
	widget::Widget *dragHoveredWidget = NULL;
	widget::Widget *selectedWidget = NULL;
	/** For double-clicking */
	double lastClickTime = -INFINITY;
	widget::Widget *lastClickedWidget = NULL;

	void setHovered(widget::Widget *w);
	void setDragged(widget::Widget *w);
	void setDragHovered(widget::Widget *w);
	void setSelected(widget::Widget *w);
	/** Prepares a widget for deletion */
	void finalizeWidget(widget::Widget *w);

	void handleButton(math::Vec pos, int button, int action, int mods);
	void handleHover(math::Vec pos, math::Vec mouseDelta);
	void handleLeave();
	void handleScroll(math::Vec pos, math::Vec scrollDelta);
	void handleText(math::Vec pos, int codepoint);
	void handleKey(math::Vec pos, int key, int scancode, int action, int mods);
	void handleDrop(math::Vec pos, const std::vector<std::string> &paths);
	void handleZoom();
};


} // namespace event
} // namespace rack
