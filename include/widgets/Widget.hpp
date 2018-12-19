#pragma once
#include "common.hpp"
#include "math.hpp"
#include "window.hpp"
#include "color.hpp"
#include "event.hpp"
#include <list>


namespace rack {


/** A node in the 2D scene graph
It is recommended to inherit virtually from Widget instead of directly.
e.g. `struct MyWidget : VirtualWidget {}`
*/
struct Widget {
	/** Stores position and size */
	math::Rect box = math::Rect(math::Vec(), math::Vec(INFINITY, INFINITY));
	/** Automatically set when Widget is added as a child to another Widget */
	Widget *parent = NULL;
	std::list<Widget*> children;
	/** Disables rendering but allow stepping */
	bool visible = true;
	/** If set to true, parent will delete Widget in the next step() */
	bool requestedDelete = false;

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

	/** Advances the module by one frame */
	virtual void step();
	/** Draws to NanoVG context */
	virtual void draw(NVGcontext *vg);

	// Events

	template <typename TMethod, class TEvent>
	void recursePositionEvent(TMethod f, TEvent &e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			Widget *child = *it;
			// Filter child by visibility and position
			if (!child->visible)
				continue;
			if (!child->box.contains(e.pos))
				continue;

			// Clone event so modifications do not up-propagate
			TEvent e2 = e;
			e2.pos = e.pos.minus(child->box.pos);
			// Call child event handler
			(child->*f)(e2);
			// Up-propagate target if consumed
			if (e2.target) {
				e.target = e2.target;
				break;
			}
		}
	}

	/** Override these event callbacks to respond to events.
	See events.hpp for a description of each event.
	*/
	virtual void onHover(event::Hover &e) {recursePositionEvent(&Widget::onHover, e);}
	virtual void onButton(event::Button &e) {recursePositionEvent(&Widget::onButton, e);}
	virtual void onHoverKey(event::HoverKey &e) {recursePositionEvent(&Widget::onHoverKey, e);}
	virtual void onHoverText(event::HoverText &e) {recursePositionEvent(&Widget::onHoverText, e);}
	virtual void onHoverScroll(event::HoverScroll &e) {recursePositionEvent(&Widget::onHoverScroll, e);}
	virtual void onEnter(event::Enter &e) {}
	virtual void onLeave(event::Leave &e) {}
	virtual void onSelect(event::Select &e) {}
	virtual void onDeselect(event::Deselect &e) {}
	virtual void onSelectKey(event::SelectKey &e) {}
	virtual void onSelectText(event::SelectText &e) {}
	virtual void onDragStart(event::DragStart &e) {}
	virtual void onDragEnd(event::DragEnd &e) {}
	virtual void onDragMove(event::DragMove &e) {}
	virtual void onDragHover(event::DragHover &e) {recursePositionEvent(&Widget::onDragHover, e);}
	virtual void onDragEnter(event::DragEnter &e) {}
	virtual void onDragLeave(event::DragLeave &e) {}
	virtual void onDragDrop(event::DragDrop &e) {}
	virtual void onPathDrop(event::PathDrop &e) {recursePositionEvent(&Widget::onPathDrop, e);}
	virtual void onAction(event::Action &e) {}
	virtual void onChange(event::Change &e) {}
	virtual void onZoom(event::Zoom &e) {}
};


/** Inherit from this class instead of inheriting from Widget directly.
Allows multiple inheritance in the class hierarchy.
*/
struct VirtualWidget : virtual Widget {};


} // namespace rack
