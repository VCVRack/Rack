#pragma once
#include <list>
#include "common.hpp"
#include "math.hpp"
#include "window.hpp"
#include "color.hpp"


namespace rack {


namespace event {

struct Event;

} // namespace event


/** A node in the 2D scene graph */
struct Widget {
	/** Stores position and size */
	math::Rect box = math::Rect(math::Vec(), math::Vec(INFINITY, INFINITY));
	Widget *parent = NULL;
	std::list<Widget*> children;
	/** Disable rendering but continue stepping */
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

	/** Trigger an event on this Widget. */
	virtual void handleEvent(event::Event &e) {}
};


} // namespace rack
