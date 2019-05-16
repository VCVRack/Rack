#pragma once
#include "common.hpp"
#include "math.hpp"
#include <vector>
#include <set>



/** Remaps Ctrl to Cmd on Mac
Use this instead of GLFW_MOD_CONTROL, since Cmd should be used on Mac in place of Ctrl on Linux/Windows.
*/
#if defined ARCH_MAC
	#define RACK_MOD_CTRL GLFW_MOD_SUPER
	#define RACK_MOD_CTRL_NAME "Cmd"
#else
	#define RACK_MOD_CTRL GLFW_MOD_CONTROL
	#define RACK_MOD_CTRL_NAME "Ctrl"
#endif
#define RACK_MOD_SHIFT_NAME "Shift"
#define RACK_MOD_ALT_NAME "Alt"

/** Filters actual mod keys from the mod flags.
Use this if you don't care about GLFW_MOD_CAPS_LOCK and GLFW_MOD_NUM_LOCK.
Example usage:
	if ((e.mod & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) ...
*/
#define RACK_MOD_MASK (GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT | GLFW_MOD_SUPER)


namespace rack {


namespace widget {
	struct Widget;
}


/** Handles user interaction with Widget.
*/
namespace event {


/** A per-event state shared and writable by all widgets that recursively handle an event. */
struct Context {
	/** Whether the event should continue recursing to children Widgets. */
	bool propagating = true;
	/** Whether the event has been consumed by an event handler and no more handlers should consume the event. */
	bool consumed = false;
	/** The widget that responded to the event. */
	widget::Widget *target = NULL;
};


/** Base class for all events. */
struct Base {
	Context *context = NULL;

	/** Prevents the event from being handled by more Widgets.
	*/
	void stopPropagating() const {
		if (!context) return;
		context->propagating = false;
	}
	bool isPropagating() const {
		if (!context) return true;
		return context->propagating;
	}
	/** Tells the event handler that a particular Widget consumed the event.
	You usually want to stop propagation as well, so call consume() instead.
	*/
	void setTarget(widget::Widget *w) const {
		if (!context) return;
		context->target = w;
	}
	widget::Widget *getTarget() const {
		if (!context) return NULL;
		return context->target;
	}
	/** Sets the target Widget and stops propagating.
	A NULL Widget may be passed to consume but not set a target.
	*/
	void consume(widget::Widget *w) const {
		if (!context) return;
		context->propagating = false;
		context->consumed = true;
		context->target = w;
	}
	bool isConsumed() const {
		if (!context) return false;
		return context->consumed;
	}
};


/** An event prototype with a vector position. */
struct PositionBase {
	/** The pixel coordinate where the event occurred, relative to the Widget it is called on. */
	math::Vec pos;
};


#define RACK_HELD 3


/** An event prototype with a GLFW key. */
struct KeyBase {
	/** GLFW_KEY_* */
	int key;
	/** GLFW_KEY_*. You should usually use `key` instead. */
	int scancode;
	/** GLFW_RELEASE, GLFW_PRESS, GLFW_REPEAT, or RACK_HELD */
	int action;
	/** GLFW_MOD_* */
	int mods;
};


/** An event prototype with a Unicode character. */
struct TextBase {
	/** Unicode code point of the character */
	int codepoint;
};


/** Occurs every frame when the mouse is hovering over a Widget.
Recurses.
*/
struct Hover : Base, PositionBase {
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};


/** Occurs each mouse button press or release.
Recurses.
*/
struct Button : Base, PositionBase {
	/** GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE, etc. */
	int button;
	/** GLFW_PRESS or GLFW_RELEASE */
	int action;
	/** GLFW_MOD_* */
	int mods;
};


/** Occurs when the left mouse button is pressed a second time on the same Widget within a time duration.
Must consume the Button event (on left button press) to receive this event.
*/
struct DoubleClick : Base {
};


/** Occurs when a key is pressed, released, or repeated while the mouse is hovering a Widget.
Recurses.
*/
struct HoverKey : Base, PositionBase, KeyBase {
};


/** Occurs when a character is typed while the mouse is hovering a Widget.
Recurses.
*/
struct HoverText : Base, PositionBase, TextBase {
};


/** Occurs when the mouse scroll wheel is moved while the mouse is hovering a Widget.
Recurses.
*/
struct HoverScroll : Base, PositionBase {
	/** Change of scroll wheel position. */
	math::Vec scrollDelta;
};


/** Occurs when a Widget begins consuming the Hover event.
Must consume the Hover event to receive this event.
*/
struct Enter : Base {
};


/** Occurs when a different Widget is entered.
Must consume the Hover event (when a Widget is entered) to receive this event.
*/
struct Leave : Base {
};


/** Occurs when a Widget begins consuming the Button press event for the left mouse button.
Must consume the Button event (on left button press) to receive this event.
*/
struct Select : Base {
};


/** Occurs when a different Widget is selected.
Must consume the Button event (on left button press, when the Widget is selected) to receive this event.
*/
struct Deselect : Base {
};


/** Occurs when a key is pressed, released, or repeated while a Widget is selected.
Must consume to prevent HoverKey from being triggered.
*/
struct SelectKey : Base, KeyBase {
};


/** Occurs when text is typed while a Widget is selected.
Must consume to prevent HoverKey from being triggered.
*/
struct SelectText : Base, TextBase {
};


struct DragBase : Base {
	/** The mouse button held while dragging. */
	int button;
};

/** Occurs when a Widget begins being dragged.
Must consume the Button event (on press) to receive this event.
*/
struct DragStart : DragBase {
};


/** Occurs when a Widget stops being dragged by releasing the mouse button.
Must consume the Button event (on press, when the Widget drag begins) to receive this event.
*/
struct DragEnd : DragBase {
};


/** Occurs every frame on the dragged Widget.
Must consume the Button event (on press, when the Widget drag begins) to receive this event.
*/
struct DragMove : DragBase {
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};


/** Occurs every frame when the mouse is hovering over a Widget while another Widget (possibly the same one) is being dragged.
Recurses.
*/
struct DragHover : DragBase, PositionBase {
	/** The dragged widget */
	widget::Widget *origin = NULL;
	/** Change in mouse position since the last frame. Can be zero. */
	math::Vec mouseDelta;
};

/** Occurs when the mouse enters a Widget while dragging.
Must consume the DragHover event to receive this event.
*/
struct DragEnter : DragBase {
	/** The dragged widget */
	widget::Widget *origin = NULL;
};


/** Occurs when the mouse leaves a Widget while dragging.
Must consume the DragHover event (when the Widget is entered) to receive this event.
*/
struct DragLeave : DragBase {
	/** The dragged widget */
	widget::Widget *origin = NULL;
};


/** Occurs when the mouse button is released over a Widget while dragging.
Must consume the Button event (on release) to receive this event.
*/
struct DragDrop : DragBase {
	/** The dragged widget */
	widget::Widget *origin = NULL;
};


/** Occurs when a selection of files from the operating system is dropped onto a Widget.
Recurses.
*/
struct PathDrop : Base, PositionBase {
	PathDrop(const std::vector<std::string> &paths) : paths(paths) {}

	/** List of file paths in the dropped selection */
	const std::vector<std::string> &paths;
};


/** Occurs after a certain action is triggered on a Widget.
The concept of an "action" is defined by the type of Widget.
*/
struct Action : Base {
};


/** Occurs after the value of a Widget changes.
The concept of a "value" is defined by the type of Widget.
*/
struct Change : Base {
};


/** Occurs after the zoom level of a Widget is changed.
Recurses.
*/
struct Zoom : Base {
};


/** Occurs after a Widget's position is set by Widget::setPosition().
*/
struct Reposition : Base {
};


/** Occurs after a Widget's size is set by Widget::setSize().
*/
struct Resize : Base {
};


/** Occurs after a Widget is added to a parent.
*/
struct Add : Base {
};


/** Occurs before a Widget is removed from its parent.
*/
struct Remove : Base {
};


/** Occurs after a Widget is shown with Widget::show().
Recurses.
*/
struct Show : Base {
};


/** Occurs after a Widget is hidden with Widget::hide().
Recurses.
*/
struct Hide : Base {
};


struct State {
	widget::Widget *rootWidget = NULL;
	/** State widgets
	Don't set these directly unless you know what you're doing. Use the set*() methods instead.
	*/
	widget::Widget *hoveredWidget = NULL;
	widget::Widget *draggedWidget = NULL;
	int dragButton = 0;
	widget::Widget *dragHoveredWidget = NULL;
	widget::Widget *selectedWidget = NULL;
	/** For double-clicking */
	double lastClickTime = -INFINITY;
	widget::Widget *lastClickedWidget = NULL;
	std::set<int> heldKeys;

	void setHovered(widget::Widget *w);
	void setDragged(widget::Widget *w, int button);
	void setDragHovered(widget::Widget *w);
	void setSelected(widget::Widget *w);
	/** Prepares a widget for deletion */
	void finalizeWidget(widget::Widget *w);

	bool handleButton(math::Vec pos, int button, int action, int mods);
	bool handleHover(math::Vec pos, math::Vec mouseDelta);
	bool handleLeave();
	bool handleScroll(math::Vec pos, math::Vec scrollDelta);
	bool handleText(math::Vec pos, int codepoint);
	bool handleKey(math::Vec pos, int key, int scancode, int action, int mods);
	bool handleDrop(math::Vec pos, const std::vector<std::string> &paths);
	bool handleZoom();
};


} // namespace event
} // namespace rack
