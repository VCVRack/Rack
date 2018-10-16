#pragma once
#include "math.hpp"
#include <vector>


namespace rack {


struct Widget;


namespace event {


/** Base event class */
struct Event {
	/** The Widget that consumes the event.
	Set to `this` in your event handler method if consumed.
	This stops propagation of the event if applicable.
	*/
	Widget *target = NULL;
};


struct Position {
	/** The pixel coordinate where the event occurred, relative to the Widget it is called on. */
	Vec pos;
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


/** Occurs every frame when the mouse is hovering over a Widget.
Recurses until consumed.
If `target` is set, other events may occur on that Widget.
*/
struct Hover : Event, Position {
	/** Change in mouse position since the last frame. Can be zero. */
	Vec mouseDelta;
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


/** Occurs when a key is pressed while the mouse is hovering a Widget.
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
	Vec scrollDelta;
};


/** Occurs when a Widget begins consuming the Hover event.
*/
struct Enter : Event {
};


/** Occurs when a different Widget is entered.
*/
struct Leave : Event {
};


/** Occurs when a Widget begins consuming the Button press event.
*/
struct Select : Event {
};


/** Occurs when a different Widget is selected.
*/
struct Deselect : Event {
};


/** Occurs when a key is pressed while a Widget is selected.
If consumed, a HoverKey event will not be triggered.
*/
struct SelectKey : Event, Key {
};


/** Occurs when text is typed while a Widget is selected.
*/
struct SelectText : Event, Text {
};


/** Occurs when a Widget begins being dragged.
Must consume to allow the drag to occur.
*/
struct DragStart : Event {
};


/** Occurs when a Widget stops being dragged by releasing the mouse button.
*/
struct DragEnd : Event {
};


/** Occurs every frame on the dragged Widget.
`mouseDelta` may be zero.
*/
struct DragMove : Event {
	Vec mouseDelta;
};


/** Occurs every frame when the mouse is hovering over a Widget while dragging.
Must consume to allow DragEnter, DragLeave, and DragDrop to occur.
*/
struct DragHover : Event, Position {
	/** Change in mouse position since the last frame. Can be zero. */
	Vec mouseDelta;
};

/** Occurs when the mouse enters a Widget while dragging.
*/
struct DragEnter : Event {
	Widget *origin = NULL;
};


/** Occurs when the mouse leaves a Widget while dragging.
*/
struct DragLeave : Event {
	Widget *origin = NULL;
};


/** Occurs when the mouse button is released over a Widget while dragging.
*/
struct DragDrop : Event {
	Widget *origin = NULL;
};


/** Occurs when a selection of files from the operating system are dropped onto a Widget.
*/
struct PathDrop : Event, Position {
	/** List of file paths in the dropped selection */
	std::vector<std::string> paths;
};


/** Occurs when an certain action is triggered on a Widget.
*/
struct Action : Event {
};


/** Occurs when the value of a Widget changes.
*/
struct Change : Event {
};


/** Occurs when the zoom level of a Widget is changed.
Recurses.
*/
struct Zoom : Event {
};


struct Context {
	Widget *rootWidget = NULL;
	Widget *hoveredWidget = NULL;
	Widget *draggedWidget = NULL;
	Widget *dragHoveredWidget = NULL;
	Widget *selectedWidget = NULL;
	/** For middle-click dragging */
	Widget *scrollWidget = NULL;

	void handleButton(Vec pos, int button, int action, int mods);
	void handleHover(Vec pos, Vec mouseDelta);
	void handleLeave();
	void handleScroll(Vec pos, Vec scrollDelta);
	void handleText(Vec pos, int codepoint);
	void handleKey(Vec pos, int key, int scancode, int action, int mods);
	void handleDrop(Vec pos, std::vector<std::string> paths);
	void handleZoom();
	/** Prepares a widget for deletion */
	void finalizeWidget(Widget *w);
};


// TODO Move this into a global context class
extern Context *gContext;


} // namespace event
} // namespace rack
