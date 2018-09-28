#include "widgets.hpp"
#include "app.hpp"
#include <algorithm>


namespace rack {

Widget::~Widget() {
	// You should only delete orphaned widgets
	assert(!parent);
	// Stop dragging and hovering this widget
	if (gHoveredWidget == this) gHoveredWidget = NULL;
	if (gDraggedWidget == this) gDraggedWidget = NULL;
	if (gDragHoveredWidget == this) gDragHoveredWidget = NULL;
	if (gSelectedWidget == this) gSelectedWidget = NULL;
	clearChildren();
}

math::Rect Widget::getChildrenBoundingBox() {
	math::Rect bound;
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

math::Vec Widget::getRelativeOffset(math::Vec v, Widget *relative) {
	if (this == relative) {
		return v;
	}
	v = v.plus(box.pos);
	if (parent) {
		v = parent->getRelativeOffset(v, relative);
	}
	return v;
}

math::Rect Widget::getViewport(math::Rect r) {
	math::Rect bound;
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

void Widget::step() {
	for (auto it = children.begin(); it != children.end();) {
		Widget *child = *it;
		// Delete children if a delete is requested
		if (child->requestedDelete) {
			it = children.erase(it);
			delete child;
			continue;
		}

		child->step();
		it++;
	}
}

void Widget::draw(NVGcontext *vg) {
	for (Widget *child : children) {
		if (!child->visible)
			continue;
		nvgSave(vg);
		nvgTranslate(vg, child->box.pos.x, child->box.pos.y);
		child->draw(vg);
		nvgRestore(vg);
	}
}


} // namespace rack
