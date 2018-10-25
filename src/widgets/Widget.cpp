#include "global_pre.hpp"
#include "widgets.hpp"
#include "app.hpp"
#include <algorithm>
#include "global_ui.hpp"


namespace rack {

Widget::~Widget() {
	// You should only delete orphaned widgets
	assert(!parent);
	// Stop dragging and hovering this widget
	if (global_ui->widgets.gHoveredWidget == this) global_ui->widgets.gHoveredWidget = NULL;
	if (global_ui->widgets.gDraggedWidget == this) global_ui->widgets.gDraggedWidget = NULL;
	if (global_ui->widgets.gDragHoveredWidget == this) global_ui->widgets.gDragHoveredWidget = NULL;
	if (global_ui->widgets.gFocusedWidget == this) global_ui->widgets.gFocusedWidget = NULL;
	if (global_ui->widgets.gTempWidget == this) global_ui->widgets.gTempWidget = NULL;
	clearChildren();
}

Rect Widget::getChildrenBoundingBox() {
	Rect bound;
	for (Widget *child : children) {
		if (child == children.front()) {
			bound = child->box;
		}
		else {
			bound = bound.expand(child->box);
		}
	}
	return bound;
}

Vec Widget::getRelativeOffset(Vec v, Widget *relative) {
	if (this == relative) {
		return v;
	}
	v = v.plus(box.pos);
	if (parent) {
		v = parent->getRelativeOffset(v, relative);
	}
	return v;
}

Rect Widget::getViewport(Rect r) {
	Rect bound;
	if (parent) {
		bound = parent->getViewport(box);
	}
	else {
		bound = box;
	}
	bound.pos = bound.pos.minus(box.pos);
	return r.clamp(bound);
}

void Widget::addChild(Widget *widget) {
	assert(!widget->parent);
	widget->parent = this;
	children.push_back(widget);
}

void Widget::removeChild(Widget *widget) {
	assert(widget->parent == this);
	auto it = std::find(children.begin(), children.end(), widget);
	assert(it != children.end());
	children.erase(it);
	widget->parent = NULL;
}

void Widget::clearChildren() {
	for (Widget *child : children) {
		child->parent = NULL;
		delete child;
	}
	children.clear();
}

void Widget::finalizeEvents() {
	// Stop dragging and hovering this widget
	if (global_ui->widgets.gHoveredWidget == this) {
		EventMouseLeave e;
		global_ui->widgets.gHoveredWidget->onMouseLeave(e);
		global_ui->widgets.gHoveredWidget = NULL;
	}
	if (global_ui->widgets.gDraggedWidget == this) {
		EventDragEnd e;
		global_ui->widgets.gDraggedWidget->onDragEnd(e);
		global_ui->widgets.gDraggedWidget = NULL;
	}
	if (global_ui->widgets.gDragHoveredWidget == this) {
		global_ui->widgets.gDragHoveredWidget = NULL;
	}
	if (global_ui->widgets.gFocusedWidget == this) {
		EventDefocus e;
		global_ui->widgets.gFocusedWidget->onDefocus(e);
		global_ui->widgets.gFocusedWidget = NULL;
	}
	for (Widget *child : children) {
		child->finalizeEvents();
	}
}

void Widget::step() {
	for (Widget *child : children) {
		child->step();
	}
}

void Widget::draw(NVGcontext *vg) {
   for (Widget *child : children) {
      // printf("xxx Widget::draw: 1 child=%p vg=%p\n", child, vg);
      if (!child->visible)
         continue;
      // printf("xxx Widget::draw: 2 child=%p\n", child);
      nvgSave(vg);
      // printf("xxx Widget::draw: 3 child=%p p=(%f; %f)\n", child, child->box.pos.x, child->box.pos.y);
      nvgTranslate(vg, child->box.pos.x, child->box.pos.y);
      // printf("xxx Widget::draw: 4 child=%p\n", child);
      child->draw(vg);
      // printf("xxx Widget::draw: 5 child=%p\n", child);
      nvgRestore(vg);
      // printf("xxx Widget::draw: 6 child=%p\n", child);
   }
}

#define RECURSE_EVENT_POSITION(_method) { \
	Vec pos = e.pos; \
	for (auto it = children.rbegin(); it != children.rend(); it++) { \
		Widget *child = *it; \
		if (!child->visible) \
			continue; \
		if (child->box.contains(pos)) { \
			e.pos = pos.minus(child->box.pos); \
			child->_method(e); \
			if (e.consumed) \
				break; \
		} \
	} \
	e.pos = pos; \
}


void Widget::onMouseDown(EventMouseDown &e) {
	RECURSE_EVENT_POSITION(onMouseDown);
}

void Widget::onMouseUp(EventMouseUp &e) {
	RECURSE_EVENT_POSITION(onMouseUp);
}

void Widget::onMouseMove(EventMouseMove &e) {
	RECURSE_EVENT_POSITION(onMouseMove);
}

void Widget::onHoverKey(EventHoverKey &e) {
	RECURSE_EVENT_POSITION(onHoverKey);
}

void Widget::onScroll(EventScroll &e) {
	RECURSE_EVENT_POSITION(onScroll);
}

void Widget::onPathDrop(EventPathDrop &e) {
	RECURSE_EVENT_POSITION(onPathDrop);
}

void Widget::onZoom(EventZoom &e) {
	for (auto it = children.rbegin(); it != children.rend(); it++) {
		Widget *child = *it;
		child->onZoom(e);
	}
}

} // namespace rack
