#pragma once
#include <list>

#include <common.hpp>
#include <math.hpp>
#include <window/Window.hpp>
#include <color.hpp>
#include <widget/event.hpp>
#include <weakptr.hpp>


namespace rack {
/** Base UI widget types */
namespace widget {


/** A node in the 2D [scene graph](https://en.wikipedia.org/wiki/Scene_graph).
The bounding box of a Widget is a rectangle specified by `box` relative to their parent.
The appearance is defined by overriding `draw()`, and the behavior is defined by overriding `step()` and `on*()` event handlers.
*/
struct Widget : WeakBase {
	/** Position relative to parent and size of widget. */
	math::Rect box = math::Rect(math::Vec(), math::Vec(INFINITY, INFINITY));
	/** Automatically set when Widget is added as a child to another Widget */
	Widget* parent = NULL;
	std::list<Widget*> children;
	/** Disables rendering but allow stepping.
	Use isVisible(), setVisible(), show(), or hide() instead of using this variable directly.
	*/
	bool visible = true;
	/** If set to true, parent will delete Widget in the next step().
	Use requestDelete() instead of using this variable directly.
	*/
	bool requestedDelete = false;

	virtual ~Widget();

	math::Rect getBox();
	/** Calls setPosition() and then setSize(). */
	void setBox(math::Rect box);
	math::Vec getPosition();
	/** Sets position and triggers RepositionEvent if position changed. */
	void setPosition(math::Vec pos);
	math::Vec getSize();
	/** Sets size and triggers ResizeEvent if size changed. */
	void setSize(math::Vec size);
	widget::Widget* getParent();
	bool isVisible();
	/** Sets `visible` and triggers ShowEvent or HideEvent if changed. */
	void setVisible(bool visible);
	/** Makes Widget visible and triggers ShowEvent if changed. */
	void show() {
		setVisible(true);
	}
	/** Makes Widget not visible and triggers HideEvent if changed. */
	void hide() {
		setVisible(false);
	}

	/** Requests this Widget's parent to delete it in the next step(). */
	void requestDelete();

	/** Returns the smallest rectangle containing this widget's children (visible and invisible) in its local coordinates.
	Returns `Rect(Vec(inf, inf), Vec(-inf, -inf))` if there are no children.
	*/
	virtual math::Rect getChildrenBoundingBox();
	virtual math::Rect getVisibleChildrenBoundingBox();
	/** Returns whether `ancestor` is a parent or distant parent of this widget.
	*/
	bool isDescendantOf(Widget* ancestor);
	/**  Returns `v` (given in local coordinates) transformed into the coordinate system of `ancestor`.
	*/
	virtual math::Vec getRelativeOffset(math::Vec v, Widget* ancestor);
	/** Returns `v` transformed into world/root/global/absolute coordinates.
	*/
	math::Vec getAbsoluteOffset(math::Vec v) {
		return getRelativeOffset(v, NULL);
	}
	/** Returns the zoom level in the coordinate system of `ancestor`.
	Only `ZoomWidget` should override this to return value other than 1.
	*/
	virtual float getRelativeZoom(Widget* ancestor);
	float getAbsoluteZoom() {
		return getRelativeZoom(NULL);
	}
	/** Returns a subset of the given Rect bounded by the box of this widget and all ancestors.
	*/
	virtual math::Rect getViewport(math::Rect r = math::Rect::inf());

	template <class T>
	T* getAncestorOfType() {
		if (!parent)
			return NULL;
		T* p = dynamic_cast<T*>(parent);
		if (p)
			return p;
		return parent->getAncestorOfType<T>();
	}

	template <class T>
	T* getFirstDescendantOfType() {
		for (Widget* child : children) {
			T* c = dynamic_cast<T*>(child);
			if (c)
				return c;
			c = child->getFirstDescendantOfType<T>();
			if (c)
				return c;
		}
		return NULL;
	}

	/** Checks if the given widget is a child of `this` widget.
	*/
	bool hasChild(Widget* child);
	/** Adds widget to the top of the children.
	Gives ownership of widget to this widget instance.
	*/
	void addChild(Widget* child);
	/** Adds widget to the bottom of the children.
	*/
	void addChildBottom(Widget* child);
	/** Adds widget directly below another widget.
	The sibling widget must already be a child of `this` widget.
	*/
	void addChildBelow(Widget* child, Widget* sibling);
	void addChildAbove(Widget* child, Widget* sibling);
	/** Removes widget from list of children if it exists.
	Triggers RemoveEvent of child.
	Does not delete widget but transfers ownership to caller
	*/
	void removeChild(Widget* child);
	/** Removes and deletes all child Widgets.
	Triggers RemoveEvent of all children.
	*/
	void clearChildren();

	/** Advances the module by one frame */
	virtual void step();

	struct DrawArgs {
		NVGcontext* vg = NULL;
		/** Local box representing the visible viewport. */
		math::Rect clipBox;
		NVGLUframebuffer* fb = NULL;
	};

	/** Draws the widget to the NanoVG context.

	When overriding, call the superclass's `draw(args)` to recurse to children.
	*/
	virtual void draw(const DrawArgs& args);
	/** Override draw(const DrawArgs &args) instead */
	DEPRECATED virtual void draw(NVGcontext* vg) {}

	/** Draw additional layers.

	Custom widgets may draw its children multiple times on different layers, passing an arbitrary layer number each time.
	Layer 0 calls children's draw().
	When overriding, always wrap draw commands in `if (layer == ...) {}` to avoid drawing on all layers.
	When overriding, call the superclass's `drawLayer(args, layer)` to recurse to children.
	*/
	virtual void drawLayer(const DrawArgs& args, int layer);

	/** Draws a particular child.
	Saves and restores NanoVG context to prevent changing the given context.
	*/
	void drawChild(Widget* child, const DrawArgs& args, int layer = 0);

	// Events

	/** Recurses an event to all visible Widgets */
	template <typename TMethod, class TEvent>
	void recurseEvent(TMethod f, const TEvent& e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			// Stop propagation if requested
			if (!e.isPropagating())
				break;
			Widget* child = *it;
			// Don't filter child by visibility. Typically only position events need to be filtered by visibility.
			// if (!child->visible)
			// 	continue;

			// Clone event for (currently) no reason
			TEvent e2 = e;
			// Call child event handler
			(child->*f)(e2);
		}
	}

	/** Recurses an event to all visible Widgets until it is consumed. */
	template <typename TMethod, class TEvent>
	void recursePositionEvent(TMethod f, const TEvent& e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			// Stop propagation if requested
			if (!e.isPropagating())
				break;
			Widget* child = *it;
			// Filter child by visibility and position
			if (!child->visible)
				continue;
			if (!child->box.contains(e.pos))
				continue;

			// Clone event and adjust its position
			TEvent e2 = e;
			e2.pos = e.pos.minus(child->box.pos);
			// Call child event handler
			(child->*f)(e2);
		}
	}

	using BaseEvent = widget::BaseEvent;

	/** An event prototype with a vector position. */
	struct PositionBaseEvent {
		/** The pixel coordinate where the event occurred, relative to the Widget it is called on. */
		math::Vec pos;
	};

	/** Occurs every frame when the mouse is hovering over a Widget.
	Recurses.
	Consume this event to allow Enter and Leave to occur.
	*/
	struct HoverEvent : BaseEvent, PositionBaseEvent {
		/** Change in mouse position since the last frame. Can be zero. */
		math::Vec mouseDelta;
	};
	virtual void onHover(const HoverEvent& e) {
		recursePositionEvent(&Widget::onHover, e);
	}

	/** Occurs each mouse button press or release.
	Recurses.
	Consume this event to allow DoubleClick, Select, Deselect, SelectKey, SelectText, DragStart, DragEnd, DragMove, and DragDrop to occur.
	*/
	struct ButtonEvent : BaseEvent, PositionBaseEvent {
		/** GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE, etc. */
		int button;
		/** GLFW_PRESS or GLFW_RELEASE */
		int action;
		/** GLFW_MOD_* */
		int mods;
	};
	virtual void onButton(const ButtonEvent& e) {
		recursePositionEvent(&Widget::onButton, e);
	}

	/** Occurs when the left mouse button is pressed a second time on the same Widget within a time duration.
	Must consume the Button event (on left button press) to receive this event.
	*/
	struct DoubleClickEvent : BaseEvent {};
	virtual void onDoubleClick(const DoubleClickEvent& e) {}

	/** An event prototype with a GLFW key. */
	struct KeyBaseEvent {
		/** The key corresponding to what it would be called in its position on a QWERTY US keyboard.
		For example, the WASD directional keys used for first-person shooters will always be reported as "WASD", regardless if they say "ZQSD" on an AZERTY keyboard.
		You should usually not use these for printable characters such as "Ctrl+V" key commands. Instead, use `keyName`.
		You *should* use these for non-printable keys, such as Escape, arrow keys, Home, F1-12, etc.
		You should also use this for Enter, Tab, and Space. Although they are printable keys, they do not appear in `keyName`.
		See GLFW_KEY_* for the list of possible values.
		*/
		int key;
		/** Platform-dependent "software" key code.
		This variable is only included for completion. There should be no reason for you to use this.
		You should instead use `key` (for non-printable characters) or `keyName` (for printable characters).
		Values are platform independent and can change between different keyboards or keyboard layouts on the same OS.
		*/
		int scancode;
		/** String containing the lowercase key name, if it produces a printable character.
		This is the only variable that correctly represents the label printed on any keyboard layout, whether it's QWERTY, AZERTY, QWERTZ, Dvorak, etc.
		For example, if the user presses the key labeled "q" regardless of the key position, `keyName` will be "q".
		For non-printable characters this is an empty string.
		Enter, Tab, and Space do not give a `keyName`. Use `key` instead.
		Shift has no effect on the key name. Shift+1 results in "1", Shift+q results in "q", etc.
		*/
		std::string keyName;
		/** The type of event occurring with the key.
		Possible values are GLFW_RELEASE, GLFW_PRESS, GLFW_REPEAT, or RACK_HELD.
		RACK_HELD is sent every frame while the key is held.
		*/
		int action;
		/** Bitwise OR of key modifiers, such as Ctrl or Shift.
		Use (mods & RACK_MOD_MASK) == RACK_MOD_CTRL to check for Ctrl on Linux and Windows but Cmd on Mac.
		See GLFW_MOD_* for the list of possible values.
		*/
		int mods;
	};

	/** Occurs when a key is pressed, released, or repeated while the mouse is hovering a Widget.
	Recurses.
	*/
	struct HoverKeyEvent : BaseEvent, PositionBaseEvent, KeyBaseEvent {};
	virtual void onHoverKey(const HoverKeyEvent& e) {
		recursePositionEvent(&Widget::onHoverKey, e);
	}

	/** An event prototype with a Unicode character. */
	struct TextBaseEvent {
		/** Unicode code point of the character */
		int codepoint;
	};
	/** Occurs when a character is typed while the mouse is hovering a Widget.
	Recurses.
	*/
	struct HoverTextEvent : BaseEvent, PositionBaseEvent, TextBaseEvent {};
	virtual void onHoverText(const HoverTextEvent& e) {
		recursePositionEvent(&Widget::onHoverText, e);
	}

	/** Occurs when the mouse scroll wheel is moved while the mouse is hovering a Widget.
	Recurses.
	*/
	struct HoverScrollEvent : BaseEvent, PositionBaseEvent {
		/** Change of scroll wheel position. */
		math::Vec scrollDelta;
	};
	virtual void onHoverScroll(const HoverScrollEvent& e) {
		recursePositionEvent(&Widget::onHoverScroll, e);
	}

	/** Occurs when a Widget begins consuming the Hover event.
	Must consume the Hover event to receive this event.
	The target sets `hoveredWidget`, which allows Leave to occur.
	*/
	struct EnterEvent : BaseEvent {};
	virtual void onEnter(const EnterEvent& e) {}

	/** Occurs when a different Widget is entered.
	Must consume the Hover event (when a Widget is entered) to receive this event.
	*/
	struct LeaveEvent : BaseEvent {};
	virtual void onLeave(const LeaveEvent& e) {}

	/** Occurs when a Widget begins consuming the Button press event for the left mouse button.
	Must consume the Button event (on left button press) to receive this event.
	The target sets `selectedWidget`, which allows SelectText and SelectKey to occur.
	*/
	struct SelectEvent : BaseEvent {};
	virtual void onSelect(const SelectEvent& e) {}

	/** Occurs when a different Widget is selected.
	Must consume the Button event (on left button press, when the Widget is selected) to receive this event.
	*/
	struct DeselectEvent : BaseEvent {};
	virtual void onDeselect(const DeselectEvent& e) {}

	/** Occurs when a key is pressed, released, or repeated while a Widget is selected.
	Must consume to prevent HoverKey from being triggered.
	*/
	struct SelectKeyEvent : BaseEvent, KeyBaseEvent {};
	virtual void onSelectKey(const SelectKeyEvent& e) {}

	/** Occurs when text is typed while a Widget is selected.
	Must consume to prevent HoverKey from being triggered.
	*/
	struct SelectTextEvent : BaseEvent, TextBaseEvent {};
	virtual void onSelectText(const SelectTextEvent& e) {}

	struct DragBaseEvent : BaseEvent {
		/** The mouse button held while dragging. */
		int button;
	};
	/** Occurs when a Widget begins being dragged.
	Must consume the Button event (on press) to receive this event.
	The target sets `draggedWidget`, which allows DragEnd, DragMove, DragHover, DragEnter, and DragDrop to occur.
	*/
	struct DragStartEvent : DragBaseEvent {};
	virtual void onDragStart(const DragStartEvent& e) {}

	/** Occurs when a Widget stops being dragged by releasing the mouse button.
	Must consume the Button event (on press, when the Widget drag begins) to receive this event.
	*/
	struct DragEndEvent : DragBaseEvent {};
	virtual void onDragEnd(const DragEndEvent& e) {}

	/** Occurs every frame on the dragged Widget.
	Must consume the Button event (on press, when the Widget drag begins) to receive this event.
	*/
	struct DragMoveEvent : DragBaseEvent {
		/** Change in mouse position since the last frame. Can be zero. */
		math::Vec mouseDelta;
	};
	virtual void onDragMove(const DragMoveEvent& e) {}

	/** Occurs every frame when the mouse is hovering over a Widget while another Widget (possibly the same one) is being dragged.
	Recurses.
	Consume this event to allow DragEnter and DragLeave to occur.
	*/
	struct DragHoverEvent : DragBaseEvent, PositionBaseEvent {
		/** The dragged widget */
		Widget* origin = NULL;
		/** Change in mouse position since the last frame. Can be zero. */
		math::Vec mouseDelta;
	};
	virtual void onDragHover(const DragHoverEvent& e) {
		recursePositionEvent(&Widget::onDragHover, e);
	}

	/** Occurs when the mouse enters a Widget while dragging.
	Must consume the DragHover event to receive this event.
	The target sets `draggedWidget`, which allows DragLeave to occur.
	*/
	struct DragEnterEvent : DragBaseEvent {
		/** The dragged widget */
		Widget* origin = NULL;
	};
	virtual void onDragEnter(const DragEnterEvent& e) {}

	/** Occurs when the mouse leaves a Widget while dragging.
	Must consume the DragHover event (when the Widget is entered) to receive this event.
	*/
	struct DragLeaveEvent : DragBaseEvent {
		/** The dragged widget */
		Widget* origin = NULL;
	};
	virtual void onDragLeave(const DragLeaveEvent& e) {}

	/** Occurs when the mouse button is released over a Widget while dragging.
	Must consume the Button event (on release) to receive this event.
	*/
	struct DragDropEvent : DragBaseEvent {
		/** The dragged widget */
		Widget* origin = NULL;
	};
	virtual void onDragDrop(const DragDropEvent& e) {}

	/** Occurs when a selection of files from the operating system is dropped onto a Widget.
	Recurses.
	*/
	struct PathDropEvent : BaseEvent, PositionBaseEvent {
		PathDropEvent(const std::vector<std::string>& paths) : paths(paths) {}

		/** List of file paths in the dropped selection */
		const std::vector<std::string>& paths;
	};
	virtual void onPathDrop(const PathDropEvent& e) {
		recursePositionEvent(&Widget::onPathDrop, e);
	}

	/** Occurs after a certain action is triggered on a Widget.
	The concept of an "action" is defined by the type of Widget.
	*/
	struct ActionEvent : BaseEvent {};
	virtual void onAction(const ActionEvent& e) {}

	/** Occurs after the value of a Widget changes.
	The concept of a "value" is defined by the type of Widget.
	*/
	struct ChangeEvent : BaseEvent {};
	virtual void onChange(const ChangeEvent& e) {}

	/** Occurs when the pixel buffer of this module must be refreshed.
	Recurses.
	*/
	struct DirtyEvent : BaseEvent {};
	virtual void onDirty(const DirtyEvent& e) {
		recurseEvent(&Widget::onDirty, e);
	}

	/** Occurs after a Widget's position is set by Widget::setPosition().
	*/
	struct RepositionEvent : BaseEvent {};
	virtual void onReposition(const RepositionEvent& e) {}

	/** Occurs after a Widget's size is set by Widget::setSize().
	*/
	struct ResizeEvent : BaseEvent {};
	virtual void onResize(const ResizeEvent& e) {}

	/** Occurs after a Widget is added to a parent.
	*/
	struct AddEvent : BaseEvent {};
	virtual void onAdd(const AddEvent& e) {}

	/** Occurs before a Widget is removed from its parent.
	*/
	struct RemoveEvent : BaseEvent {};
	virtual void onRemove(const RemoveEvent& e) {}

	/** Occurs after a Widget is shown with Widget::show().
	Recurses.
	*/
	struct ShowEvent : BaseEvent {};
	virtual void onShow(const ShowEvent& e) {
		recurseEvent(&Widget::onShow, e);
	}

	/** Occurs after a Widget is hidden with Widget::hide().
	Recurses.
	*/
	struct HideEvent : BaseEvent {};
	virtual void onHide(const HideEvent& e) {
		recurseEvent(&Widget::onHide, e);
	}

	/** Occurs after the Window (including OpenGL and NanoVG contexts) are created.
	Recurses.
	*/
	struct ContextCreateEvent : BaseEvent {
		NVGcontext* vg;
	};
	virtual void onContextCreate(const ContextCreateEvent& e) {
		recurseEvent(&Widget::onContextCreate, e);
	}

	/** Occurs before the Window (including OpenGL and NanoVG contexts) are destroyed.
	Recurses.
	*/
	struct ContextDestroyEvent : BaseEvent {
		NVGcontext* vg;
	};
	virtual void onContextDestroy(const ContextDestroyEvent& e) {
		recurseEvent(&Widget::onContextDestroy, e);
	}
};


} // namespace widget

/** Deprecated Rack v1 event namespace.
Use events defined in the widget::Widget class instead of this `event::` namespace in new code.
*/
namespace event {
using Base = widget::BaseEvent;
using PositionBase = widget::Widget::PositionBaseEvent;
using KeyBase = widget::Widget::KeyBaseEvent;
using TextBase = widget::Widget::TextBaseEvent;
using Hover = widget::Widget::HoverEvent;
using Button = widget::Widget::ButtonEvent;
using DoubleClick = widget::Widget::DoubleClickEvent;
using HoverKey = widget::Widget::HoverKeyEvent;
using HoverText = widget::Widget::HoverTextEvent;
using HoverScroll = widget::Widget::HoverScrollEvent;
using Enter = widget::Widget::EnterEvent;
using Leave = widget::Widget::LeaveEvent;
using Select = widget::Widget::SelectEvent;
using Deselect = widget::Widget::DeselectEvent;
using SelectKey = widget::Widget::SelectKeyEvent;
using SelectText = widget::Widget::SelectTextEvent;
using DragBase = widget::Widget::DragBaseEvent;
using DragStart = widget::Widget::DragStartEvent;
using DragEnd = widget::Widget::DragEndEvent;
using DragMove = widget::Widget::DragMoveEvent;
using DragHover = widget::Widget::DragHoverEvent;
using DragEnter = widget::Widget::DragEnterEvent;
using DragLeave = widget::Widget::DragLeaveEvent;
using DragDrop = widget::Widget::DragDropEvent;
using PathDrop = widget::Widget::PathDropEvent;
using Action = widget::Widget::ActionEvent;
using Change = widget::Widget::ChangeEvent;
using Dirty = widget::Widget::DirtyEvent;
using Reposition = widget::Widget::RepositionEvent;
using Resize = widget::Widget::ResizeEvent;
using Add = widget::Widget::AddEvent;
using Remove = widget::Widget::RemoveEvent;
using Show = widget::Widget::ShowEvent;
using Hide = widget::Widget::HideEvent;
}


} // namespace rack
