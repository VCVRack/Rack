#pragma once
#include "window.hpp"


namespace rack {


/** A node in the 2D scene graph
Never inherit from Widget directly. Instead, inherit from VirtualWidget declared below.
*/
struct Widget {
	/** Stores position and size */
	math::Rect box = math::Rect(math::Vec(), math::Vec(INFINITY, INFINITY));
	Widget *parent = NULL;
	std::list<Widget*> children;
	bool visible = true;

	virtual ~Widget();

	virtual math::Rect getChildrenBoundingBox();
	/**  Returns `v` transformed into the coordinate system of `relative` */
	virtual math::Vec getRelativeOffset(math::Vec v, Widget *relative);
	/** Returns `v` transformed into world coordinates */
	math::Vec getAbsoluteOffset(math::Vec v) {
		return getRelativeOffset(v, NULL);
	}
	/** Returns a subset of the given math::Rect bounded by the box of this widget and all ancestors */
	virtual math::Rect getViewport(math::Rect r);

	template <class T>
	T *getAncestorOfType() {
		if (!parent) return NULL;
		T *p = dynamic_cast<T*>(parent);
		if (p) return p;
		return parent->getAncestorOfType<T>();
	}

	template <class T>
	T *getFirstDescendantOfType() {
		for (Widget *child : children) {
			T *c = dynamic_cast<T*>(child);
			if (c) return c;
			c = child->getFirstDescendantOfType<T>();
			if (c) return c;
		}
		return NULL;
	}

	/** Adds widget to list of children.
	Gives ownership of widget to this widget instance.
	*/
	void addChild(Widget *widget);
	/** Removes widget from list of children if it exists.
	Does not delete widget but transfers ownership to caller
	*/
	void removeChild(Widget *widget);
	/** Removes and deletes all children */
	void clearChildren();
	/** Recursively finalizes event start/end pairs as needed */
	void finalizeEvents();

	/** Advances the module by one frame */
	virtual void step();
	/** Draws to NanoVG context */
	virtual void draw(NVGcontext *vg);

	// Events

	/** Called when a mouse button is pressed over this widget */
	virtual void onMouseDown(EventMouseDown &e);
	/** Called when a mouse button is released over this widget */
	virtual void onMouseUp(EventMouseUp &e);
	/** Called when the mouse moves over this widget.
	Called on every frame, even if `mouseRel = math::Vec(0, 0)`.
	*/
	virtual void onMouseMove(EventMouseMove &e);
	/** Called when a key is pressed while hovering over this widget */
	virtual void onHoverKey(EventHoverKey &e);
	/** Called when this widget begins responding to `onMouseMove` events */
	virtual void onMouseEnter(EventMouseEnter &e) {}
	/** Called when this widget no longer responds to `onMouseMove` events */
	virtual void onMouseLeave(EventMouseLeave &e) {}
	/** Called when this widget gains focus by responding to the `onMouseDown` event */
	virtual void onFocus(EventFocus &e) {}
	virtual void onDefocus(EventDefocus &e) {}
	/** Called when a printable character is received while this widget is focused */
	virtual void onText(EventText &e) {}
	/** Called when a key is pressed while this widget is focused */
	virtual void onKey(EventKey &e) {}
	/** Called when the scroll wheel is moved while the mouse is hovering over this widget */
	virtual void onScroll(EventScroll &e);

	/** Called when a widget responds to `onMouseDown` for a left button press */
	virtual void onDragStart(EventDragStart &e) {}
	/** Called when the left button is released and this widget is being dragged */
	virtual void onDragEnd(EventDragEnd &e) {}
	/** Called when a widget responds to `onMouseMove` and is being dragged */
	virtual void onDragMove(EventDragMove &e) {}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	virtual void onDragEnter(EventDragEnter &e) {}
	virtual void onDragLeave(EventDragEnter &e) {}
	/** Called when a drag action ends while hovering this widget */
	virtual void onDragDrop(EventDragDrop &e) {}
	/** Called when an OS selection of files is dragged-and-dropped on this widget */
	virtual void onPathDrop(EventPathDrop &e);

	/** Called when an event triggers an action */
	virtual void onAction(EventAction &e) {}
	/** For widgets with some concept of values, called when the value is changed */
	virtual void onChange(EventChange &e) {}
	/** Called when the zoom level is changed of this widget */
	virtual void onZoom(EventZoom &e);

	/** Helper function for creating and initializing a Widget with certain arguments (in this case just the position).
	In this project, you will find this idiom everywhere, as an easier alternative to constructor arguments, for building a Widget (or a subclass) with a one-liner.
	Example:
		addChild(Widget::create<SVGWidget>(math::Vec(10, 10)))
	*/
	template <typename T = Widget>
	static T *create(math::Vec pos = math::Vec()) {
		T *o = new T();
		o->box.pos = pos;
		return o;
	}
};


} // namespace rack
