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
e.g. `struct MyWidget : virtual Widget {}`
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
	void recurseEvent(TMethod f, const TEvent &e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			Widget *child = *it;
			// Filter child by visibility
			if (!child->visible)
				continue;

			// Call child event handler
			(child->*f)(e);
			// Stop iterating if consumed
			if (e.getConsumed())
				break;
		}
	}

	template <typename TMethod, class TEvent>
	void recursePositionEvent(TMethod f, const TEvent &e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			Widget *child = *it;
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
			// Stop iterating if consumed
			if (e.getConsumed())
				break;
		}
	}

	/** Override these event callbacks to respond to events.
	See events.hpp for a description of each event.
	*/
	virtual void onHover(const event::Hover &e) {recursePositionEvent(&Widget::onHover, e);}
	virtual void onButton(const event::Button &e) {recursePositionEvent(&Widget::onButton, e);}
	virtual void onHoverKey(const event::HoverKey &e) {recursePositionEvent(&Widget::onHoverKey, e);}
	virtual void onHoverText(const event::HoverText &e) {recursePositionEvent(&Widget::onHoverText, e);}
	virtual void onHoverScroll(const event::HoverScroll &e) {recursePositionEvent(&Widget::onHoverScroll, e);}
	virtual void onEnter(const event::Enter &e) {}
	virtual void onLeave(const event::Leave &e) {}
	virtual void onSelect(const event::Select &e) {}
	virtual void onDeselect(const event::Deselect &e) {}
	virtual void onSelectKey(const event::SelectKey &e) {}
	virtual void onSelectText(const event::SelectText &e) {}
	virtual void onDragStart(const event::DragStart &e) {}
	virtual void onDragEnd(const event::DragEnd &e) {}
	virtual void onDragMove(const event::DragMove &e) {}
	virtual void onDragHover(const event::DragHover &e) {recursePositionEvent(&Widget::onDragHover, e);}
	virtual void onDragEnter(const event::DragEnter &e) {}
	virtual void onDragLeave(const event::DragLeave &e) {}
	virtual void onDragDrop(const event::DragDrop &e) {}
	virtual void onPathDrop(const event::PathDrop &e) {recursePositionEvent(&Widget::onPathDrop, e);}
	virtual void onAction(const event::Action &e) {}
	virtual void onChange(const event::Change &e) {}
	virtual void onZoom(const event::Zoom &e) {recurseEvent(&Widget::onZoom, e);}
};


} // namespace rack
