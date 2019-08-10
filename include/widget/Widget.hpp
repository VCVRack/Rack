#pragma once
#include <common.hpp>
#include <math.hpp>
#include <window.hpp>
#include <color.hpp>
#include <event.hpp>
#include <list>


namespace rack {


/** General UI widgets
*/
namespace widget {


/** A node in the 2D [scene graph](https://en.wikipedia.org/wiki/Scene_graph).
The bounding box of a Widget is a rectangle specified by `box` relative to their parent.
The appearance is defined by overriding `draw()`, and the behavior is defined by overriding `step()` and `on*()` event handlers.
*/
struct Widget {
	/** Stores position and size */
	math::Rect box = math::Rect(math::Vec(), math::Vec(INFINITY, INFINITY));
	/** Automatically set when Widget is added as a child to another Widget */
	Widget* parent = NULL;
	std::list<Widget*> children;
	/** Disables rendering but allow stepping */
	bool visible = true;
	/** If set to true, parent will delete Widget in the next step() */
	bool requestedDelete = false;

	virtual ~Widget();

	void setPosition(math::Vec pos);
	void setSize(math::Vec size);
	void show();
	void hide();
	void requestDelete();

	virtual math::Rect getChildrenBoundingBox();
	/**  Returns `v` transformed into the coordinate system of `relative` */
	virtual math::Vec getRelativeOffset(math::Vec v, Widget* relative);
	/** Returns `v` transformed into world coordinates */
	math::Vec getAbsoluteOffset(math::Vec v) {
		return getRelativeOffset(v, NULL);
	}
	/** Returns a subset of the given math::Rect bounded by the box of this widget and all ancestors */
	virtual math::Rect getViewport(math::Rect r);

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

	/** Adds widget to list of children.
	Gives ownership of widget to this widget instance.
	*/
	void addChild(Widget* child);
	void addChildBottom(Widget* child);
	/** Removes widget from list of children if it exists.
	Does not delete widget but transfers ownership to caller
	*/
	void removeChild(Widget* child);
	/** Removes and deletes all children */
	void clearChildren();

	/** Advances the module by one frame */
	virtual void step();

	struct DrawArgs {
		NVGcontext* vg;
		math::Rect clipBox;
		NVGLUframebuffer* fb = NULL;
	};

	/** Draws the widget to the NanoVG context */
	virtual void draw(const DrawArgs& args);
	/** Override draw(const DrawArgs &args) instead */
	DEPRECATED virtual void draw(NVGcontext* vg) {}

	// Events

	/** Recurses an event to all visible Widgets */
	template <typename TMethod, class TEvent>
	void recurseEvent(TMethod f, const TEvent& e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			// Stop propagation if requested
			if (!e.isPropagating())
				break;
			Widget* child = *it;
			// Filter child by visibility
			if (!child->visible)
				continue;

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
			if (!child->box.isContaining(e.pos))
				continue;

			// Clone event and adjust its position
			TEvent e2 = e;
			e2.pos = e.pos.minus(child->box.pos);
			// Call child event handler
			(child->*f)(e2);
		}
	}

	/** Override these event callbacks to respond to events.
	See event.hpp for a description of each event.
	*/
	virtual void onHover(const event::Hover& e) {
		recursePositionEvent(&Widget::onHover, e);
	}
	virtual void onButton(const event::Button& e) {
		recursePositionEvent(&Widget::onButton, e);
	}
	virtual void onDoubleClick(const event::DoubleClick& e) {}
	virtual void onHoverKey(const event::HoverKey& e) {
		recursePositionEvent(&Widget::onHoverKey, e);
	}
	virtual void onHoverText(const event::HoverText& e) {
		recursePositionEvent(&Widget::onHoverText, e);
	}
	virtual void onHoverScroll(const event::HoverScroll& e) {
		recursePositionEvent(&Widget::onHoverScroll, e);
	}
	virtual void onEnter(const event::Enter& e) {}
	virtual void onLeave(const event::Leave& e) {}
	virtual void onSelect(const event::Select& e) {}
	virtual void onDeselect(const event::Deselect& e) {}
	virtual void onSelectKey(const event::SelectKey& e) {}
	virtual void onSelectText(const event::SelectText& e) {}
	virtual void onDragStart(const event::DragStart& e) {}
	virtual void onDragEnd(const event::DragEnd& e) {}
	virtual void onDragMove(const event::DragMove& e) {}
	virtual void onDragHover(const event::DragHover& e) {
		recursePositionEvent(&Widget::onDragHover, e);
	}
	virtual void onDragEnter(const event::DragEnter& e) {}
	virtual void onDragLeave(const event::DragLeave& e) {}
	virtual void onDragDrop(const event::DragDrop& e) {}
	virtual void onPathDrop(const event::PathDrop& e) {
		recursePositionEvent(&Widget::onPathDrop, e);
	}
	virtual void onAction(const event::Action& e) {}
	virtual void onChange(const event::Change& e) {}
	virtual void onZoom(const event::Zoom& e) {
		recurseEvent(&Widget::onZoom, e);
	}
	virtual void onReposition(const event::Reposition& e) {}
	virtual void onResize(const event::Resize& e) {}
	virtual void onAdd(const event::Add& e) {}
	virtual void onRemove(const event::Remove& e) {}
	virtual void onShow(const event::Show& e) {
		recurseEvent(&Widget::onShow, e);
	}
	virtual void onHide(const event::Hide& e) {
		recurseEvent(&Widget::onHide, e);
	}
};


} // namespace widget
} // namespace rack
