#include "widgets.hpp"
#include "app.hpp"
#include "event.hpp"
#include <algorithm>


namespace rack {

Widget::~Widget() {
	// You should only delete orphaned widgets
	assert(!parent);
	event::gContext->finalizeWidget(this);
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

void Widget::step() {
	for (auto it = children.begin(); it != children.end();) {
		Widget *child = *it;
		// Delete children if a delete is requested
		if (child->requestedDelete) {
			it = children.erase(it);
			child->parent = NULL;
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
