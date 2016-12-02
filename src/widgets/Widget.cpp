#include "Rack.hpp"
#include <algorithm>


namespace rack {

Widget::~Widget() {
	// You should only delete orphaned widgets
	assert(!parent);
	// Stop dragging and hovering this widget
	if (gHoveredWidget == this)
		gHoveredWidget = NULL;
	if (gDraggedWidget == this)
		gDraggedWidget = NULL;
	clearChildren();
}

Vec Widget::getAbsolutePos() {
	// Recursively compute position offset from parents
	if (!parent) {
		return box.pos;
	}
	else {
		return box.pos.plus(parent->getAbsolutePos());
	}
}

Rect Widget::getChildrenBoundingBox() {
	if (children.empty()) {
		return Rect();
	}

	Vec topLeft = Vec(INFINITY, INFINITY);
	Vec bottomRight = Vec(-INFINITY, -INFINITY);
	for (Widget *child : children) {
		topLeft = topLeft.min(child->box.pos);
		bottomRight = bottomRight.max(child->box.getBottomRight());
	}
	return Rect(topLeft, bottomRight.minus(topLeft));
}

void Widget::addChild(Widget *widget) {
	assert(!widget->parent);
	widget->parent = this;
	children.push_back(widget);
}

void Widget::removeChild(Widget *widget) {
	assert(widget->parent == this);
	auto it = std::find(children.begin(), children.end(), widget);
	if (it != children.end()) {
		children.erase(it);
		widget->parent = NULL;
	}
}

void Widget::clearChildren() {
	for (Widget *child : children) {
		child->parent = NULL;
		delete child;
	}
	children.clear();
}

void Widget::step() {
	for (Widget *child : children) {
		child->step();
	}
}

void Widget::draw(NVGcontext *vg) {
	nvgSave(vg);
	nvgTranslate(vg, box.pos.x, box.pos.y);
	for (Widget *child : children) {
		child->draw(vg);
	}
	nvgRestore(vg);
}

Widget *Widget::pick(Vec pos) {
	if (!box.contains(pos))
		return NULL;
	pos = pos.minus(box.pos);
	for (auto it = children.rbegin(); it != children.rend(); it++) {
		Widget *child = *it;
		Widget *picked = child->pick(pos);
		if (picked)
			return picked;
	}
	return this;
}


} // namespace rack
