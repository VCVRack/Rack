#pragma once
#include "common.hpp"
#include "math.hpp"
#include "window.hpp"
#include "color.hpp"
#include "widget/event.hpp"
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
	Widget *parent = NULL;
	std::list<Widget*> children;
	/** Disables rendering but allow stepping */
	bool visible = true;
	/** If set to true, parent will delete Widget in the next step() */
	bool requestedDelete = false;

	virtual ~Widget();

	void setPos(math::Vec pos);
	void setSize(math::Vec size);
	void show();
	void hide();
	void requestDelete();

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
	void addChild(Widget *child);
	/** Removes widget from list of children if it exists.
	Does not delete widget but transfers ownership to caller
	*/
	void removeChild(Widget *child);
	/** Removes and deletes all children */
	void clearChildren();

	/** Advances the module by one frame */
	virtual void step();

	struct DrawArgs {
		NVGcontext *vg;
		math::Rect clipBox;
		NVGLUframebuffer *fb = NULL;
	};

	/** Draws the widget to the NanoVG context */
	virtual void draw(const DrawArgs &args);
	/** Override draw(const DrawArgs &args) instead */
	DEPRECATED virtual void draw(NVGcontext *vg) {}

	// Events

	/** Recurses an event to all visible Widgets */
	template <typename TMethod, class TEvent>
	void recurseEvent(TMethod f, const TEvent &e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			Widget *child = *it;
			// Filter child by visibility
			if (!child->visible)
				continue;

			// Call child event handler
			(child->*f)(e);
		}
	}

	/** Recurses an event to all visible Widgets until it is consumed. */
	template <typename TMethod, class TEvent>
	void recursePositionEvent(TMethod f, const TEvent &e) {
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			Widget *child = *it;
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
			// Stop iterating if consumed
			if (e.getConsumed())
				break;
		}
	}

	/** Override these event callbacks to respond to events.
	See events.hpp for a description of each event.
	*/
	virtual void onHover(const HoverEvent &e) {recursePositionEvent(&Widget::onHover, e);}
	virtual void onButton(const ButtonEvent &e) {recursePositionEvent(&Widget::onButton, e);}
	virtual void onDoubleClick(const DoubleClickEvent &e) {}
	virtual void onHoverKey(const HoverKeyEvent &e) {recursePositionEvent(&Widget::onHoverKey, e);}
	virtual void onHoverText(const HoverTextEvent &e) {recursePositionEvent(&Widget::onHoverText, e);}
	virtual void onHoverScroll(const HoverScrollEvent &e) {recursePositionEvent(&Widget::onHoverScroll, e);}
	virtual void onEnter(const EnterEvent &e) {}
	virtual void onLeave(const LeaveEvent &e) {}
	virtual void onSelect(const SelectEvent &e) {}
	virtual void onDeselect(const DeselectEvent &e) {}
	virtual void onSelectKey(const SelectKeyEvent &e) {}
	virtual void onSelectText(const SelectTextEvent &e) {}
	virtual void onDragStart(const DragStartEvent &e) {}
	virtual void onDragEnd(const DragEndEvent &e) {}
	virtual void onDragMove(const DragMoveEvent &e) {}
	virtual void onDragHover(const DragHoverEvent &e) {recursePositionEvent(&Widget::onDragHover, e);}
	virtual void onDragEnter(const DragEnterEvent &e) {}
	virtual void onDragLeave(const DragLeaveEvent &e) {}
	virtual void onDragDrop(const DragDropEvent &e) {}
	virtual void onPathDrop(const PathDropEvent &e) {recursePositionEvent(&Widget::onPathDrop, e);}
	virtual void onAction(const ActionEvent &e) {}
	virtual void onChange(const ChangeEvent &e) {}
	virtual void onZoom(const ZoomEvent &e) {recurseEvent(&Widget::onZoom, e);}
	virtual void onReposition(const RepositionEvent &e) {}
	virtual void onResize(const ResizeEvent &e) {}
	virtual void onAdd(const AddEvent &e) {}
	virtual void onRemove(const RemoveEvent &e) {}
	virtual void onShow(const ShowEvent &e) {recurseEvent(&Widget::onShow, e);}
	virtual void onHide(const HideEvent &e) {recurseEvent(&Widget::onHide, e);}
};


} // namespace widget
} // namespace rack
