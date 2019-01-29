#include "widget/Widget.hpp"
#include "event.hpp"
#include "app.hpp"
#include <algorithm>


namespace rack {
namespace widget {


Widget::~Widget() {
	// You should only delete orphaned widgets
	assert(!parent);
	clearChildren();
}

math::Rect Widget::getChildrenBoundingBox() {
	math::Vec min = math::Vec(INFINITY, INFINITY);
	math::Vec max = math::Vec(-INFINITY, -INFINITY);
	for (Widget *child : children) {
		min = min.min(child->box.getTopLeft());
		max = max.max(child->box.getBottomRight());
	}
	return math::Rect::fromMinMax(min, max);
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

void Widget::addChild(Widget *child) {
	assert(child);
	assert(!child->parent);
	child->parent = this;
	children.push_back(child);
}

void Widget::removeChild(Widget *child) {
	assert(child);
	// Make sure `this` is the child's parent
	assert(child->parent == this);
	// Prepare to remove widget from the event state
	APP->event->finalizeWidget(child);
	// Delete child from children list
	auto it = std::find(children.begin(), children.end(), child);
	assert(it != children.end());
	children.erase(it);
	// Revoke child's parent
	child->parent = NULL;
}

void Widget::clearChildren() {
	for (Widget *child : children) {
		APP->event->finalizeWidget(child);
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
			APP->event->finalizeWidget(child);
			it = children.erase(it);
			child->parent = NULL;
			delete child;
			continue;
		}

		child->step();
		it++;
	}
}

void Widget::draw(const DrawContext &ctx) {
	// Iterate children
	for (Widget *child : children) {
		// Don't draw if invisible
		if (!child->visible)
			continue;
		// Don't draw if child is outside clip box
		if (!ctx.clipBox.isIntersecting(child->box))
			continue;

		DrawContext childCtx = ctx;
		// Intersect child clip box with self
		childCtx.clipBox = childCtx.clipBox.intersect(child->box);
		childCtx.clipBox.pos = childCtx.clipBox.pos.minus(child->box.pos);

		nvgSave(ctx.vg);
		nvgTranslate(ctx.vg, child->box.pos.x, child->box.pos.y);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		// Call deprecated draw function, which does nothing by default
		child->draw(ctx.vg);
#pragma GCC diagnostic pop

		child->draw(childCtx);

		// Draw red hitboxes
		// if (APP->event->hoveredWidget == child) {
		// 	nvgBeginPath(ctx.vg);
		// 	nvgRect(ctx.vg, 0, 0, child->box.size.x, child->box.size.y);
		// 	nvgFillColor(ctx.vg, nvgRGBAf(1, 0, 0, 0.5));
		// 	nvgFill(ctx.vg);
		// }

		nvgRestore(ctx.vg);
	}
}


} // namespace widget
} // namespace rack
