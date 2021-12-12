#pragma once
#include <vector>
#include <set>

#include <common.hpp>
#include <math.hpp>



/** Remaps Ctrl to Cmd on Mac
Use this instead of GLFW_MOD_CONTROL, since Cmd should be used on Mac in place of Ctrl on Linux/Windows.
*/
#if defined ARCH_MAC
	#define RACK_MOD_CTRL GLFW_MOD_SUPER
	#define RACK_MOD_CTRL_NAME "⌘"
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

/** A key action state representing the the key is (still) being held.
*/
#define RACK_HELD 3


namespace rack {
namespace widget {


struct Widget;


/** A per-event state shared and writable by all widgets that recursively handle an event. */
struct EventContext {
	/** Whether the event should continue recursing to children Widgets. */
	bool propagating = true;
	/** Whether the event has been consumed by an event handler and no more handlers should consume the event. */
	bool consumed = false;
	/** The widget that responded to the event. */
	Widget* target = NULL;
};


/** Base class for all events. */
struct BaseEvent {
	EventContext* context = NULL;

	/** Prevents the event from being handled by more Widgets.
	*/
	void stopPropagating() const {
		if (!context)
			return;
		context->propagating = false;
	}
	bool isPropagating() const {
		if (!context)
			return true;
		return context->propagating;
	}
	/** Tells the event handler that a particular Widget consumed the event.
	You usually want to stop propagation as well, so call consume() instead.
	*/
	void setTarget(Widget* w) const {
		if (!context)
			return;
		context->target = w;
	}
	Widget* getTarget() const {
		if (!context)
			return NULL;
		return context->target;
	}
	/** Sets the target Widget and stops propagating.
	A NULL Widget may be passed to consume but not set a target.
	*/
	void consume(Widget* w) const {
		if (!context)
			return;
		context->propagating = false;
		context->consumed = true;
		context->target = w;
	}
	void unconsume() const {
		if (!context)
			return;
		context->consumed = false;
	}
	bool isConsumed() const {
		if (!context)
			return false;
		return context->consumed;
	}
};


struct EventState {
	Widget* rootWidget = NULL;
	/** State widgets
	Don't set these directly unless you know what you're doing. Use the set*() methods instead.
	*/
	Widget* hoveredWidget = NULL;
	Widget* draggedWidget = NULL;
	int dragButton = 0;
	Widget* dragHoveredWidget = NULL;
	Widget* selectedWidget = NULL;
	/** For double-clicking */
	double lastClickTime = -INFINITY;
	Widget* lastClickedWidget = NULL;
	std::set<int> heldKeys;

	Widget* getRootWidget() {
		return rootWidget;
	}
	Widget* getHoveredWidget() {
		return hoveredWidget;
	}
	Widget* getDraggedWidget() {
		return draggedWidget;
	}
	Widget* getDragHoveredWidget() {
		return dragHoveredWidget;
	}
	Widget* getSelectedWidget() {
		return selectedWidget;
	}

	void setHoveredWidget(Widget* w);
	void setDraggedWidget(Widget* w, int button);
	void setDragHoveredWidget(Widget* w);
	void setSelectedWidget(Widget* w);
	DEPRECATED void setHovered(Widget* w) {setHoveredWidget(w);}
	DEPRECATED void setDragged(Widget* w, int button) {setDraggedWidget(w, button);}
	DEPRECATED void setDragHovered(Widget* w) {setDragHoveredWidget(w);}
	DEPRECATED void setSelected(Widget* w) {setSelectedWidget(w);}
	/** Prepares a widget for deletion */
	void finalizeWidget(Widget* w);

	bool handleButton(math::Vec pos, int button, int action, int mods);
	bool handleHover(math::Vec pos, math::Vec mouseDelta);
	bool handleLeave();
	bool handleScroll(math::Vec pos, math::Vec scrollDelta);
	bool handleText(math::Vec pos, int codepoint);
	bool handleKey(math::Vec pos, int key, int scancode, int action, int mods);
	bool handleDrop(math::Vec pos, const std::vector<std::string>& paths);
	bool handleDirty();
};


} // namespace widget
} // namespace rack
