#include <algorithm>

#include <widget/Widget.hpp>
#include <context.hpp>


namespace rack {
namespace widget {


Widget::~Widget() {
	// You should only delete orphaned widgets
	assert(!parent);
	clearChildren();
}


math::Rect Widget::getBox() {
	return box;
}


void Widget::setBox(math::Rect box) {
	setPosition(box.pos);
	setSize(box.size);
}


math::Vec Widget::getPosition() {
	return box.pos;
}


void Widget::setPosition(math::Vec pos) {
	if (pos.equals(box.pos))
		return;
	box.pos = pos;
	// Dispatch Reposition event
	RepositionEvent eReposition;
	onReposition(eReposition);
}


math::Vec Widget::getSize() {
	return box.size;
}


void Widget::setSize(math::Vec size) {
	if (size.equals(box.size))
		return;
	box.size = size;
	// Dispatch Resize event
	ResizeEvent eResize;
	onResize(eResize);
}


widget::Widget *Widget::getParent() {
	return parent;
}


bool Widget::isVisible() {
	return visible;
}


void Widget::setVisible(bool visible) {
	if (visible == this->visible)
		return;
	this->visible = visible;
	if (visible) {
		// Dispatch Show event
		ShowEvent eShow;
		onShow(eShow);
	}
	else {
		// Dispatch Hide event
		HideEvent eHide;
		onHide(eHide);
	}
}


void Widget::requestDelete() {
	requestedDelete = true;
}


math::Rect Widget::getChildrenBoundingBox() {
	math::Vec min = math::Vec(INFINITY, INFINITY);
	math::Vec max = math::Vec(-INFINITY, -INFINITY);
	for (Widget* child : children) {
		min = min.min(child->box.getTopLeft());
		max = max.max(child->box.getBottomRight());
	}
	return math::Rect::fromMinMax(min, max);
}


math::Rect Widget::getVisibleChildrenBoundingBox() {
	math::Vec min = math::Vec(INFINITY, INFINITY);
	math::Vec max = math::Vec(-INFINITY, -INFINITY);
	for (Widget* child : children) {
		if (!child->isVisible())
			continue;
		min = min.min(child->box.getTopLeft());
		max = max.max(child->box.getBottomRight());
	}
	return math::Rect::fromMinMax(min, max);
}


bool Widget::isDescendantOf(Widget* ancestor) {
	if (!parent)
		return false;
	if (parent == ancestor)
		return true;
	return parent->isDescendantOf(ancestor);
}


math::Vec Widget::getRelativeOffset(math::Vec v, Widget* ancestor) {
	if (this == ancestor)
		return v;
	// Translate offset
	v = v.plus(box.pos);
	if (!parent)
		return v;
	return parent->getRelativeOffset(v, ancestor);
}


float Widget::getRelativeZoom(Widget* ancestor) {
	if (this == ancestor)
		return 1.f;
	if (!parent)
		return 1.f;
	return parent->getRelativeZoom(ancestor);
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


bool Widget::hasChild(Widget* child) {
	if (!child)
		return false;
	auto it = std::find(children.begin(), children.end(), child);
	return (it != children.end());
}


void Widget::addChild(Widget* child) {
	assert(child);
	assert(!child->parent);
	// Add child
	child->parent = this;
	children.push_back(child);
	// Dispatch Add event
	AddEvent eAdd;
	child->onAdd(eAdd);
}


void Widget::addChildBottom(Widget* child) {
	assert(child);
	assert(!child->parent);
	// Add child
	child->parent = this;
	children.push_front(child);
	// Dispatch Add event
	AddEvent eAdd;
	child->onAdd(eAdd);
}


void Widget::addChildBelow(Widget* child, Widget* sibling) {
	assert(child);
	assert(!child->parent);
	auto it = std::find(children.begin(), children.end(), sibling);
	assert(it != children.end());
	// Add child
	child->parent = this;
	children.insert(it, child);
	// Dispatch Add event
	AddEvent eAdd;
	child->onAdd(eAdd);
}


void Widget::addChildAbove(Widget* child, Widget* sibling) {
	assert(child);
	assert(!child->parent);
	auto it = std::find(children.begin(), children.end(), sibling);
	assert(it != children.end());
	// Add child
	child->parent = this;
	it++;
	children.insert(it, child);
	// Dispatch Add event
	AddEvent eAdd;
	child->onAdd(eAdd);
}


void Widget::removeChild(Widget* child) {
	assert(child);
	// Make sure `this` is the child's parent
	assert(child->parent == this);
	// Dispatch Remove event
	RemoveEvent eRemove;
	child->onRemove(eRemove);
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
	for (Widget* child : children) {
		// Dispatch Remove event
		RemoveEvent eRemove;
		child->onRemove(eRemove);
		APP->event->finalizeWidget(child);
		child->parent = NULL;
		delete child;
	}
	children.clear();
}


void Widget::step() {
	for (auto it = children.begin(); it != children.end();) {
		Widget* child = *it;
		// Delete children if a delete is requested
		if (child->requestedDelete) {
			// Dispatch Remove event
			RemoveEvent eRemove;
			child->onRemove(eRemove);
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


void Widget::draw(const DrawArgs& args) {
	// Iterate children
	for (Widget* child : children) {
		// Don't draw if invisible
		if (!child->isVisible())
			continue;
		// Don't draw if child is outside clip box
		if (!args.clipBox.intersects(child->box))
			continue;

		drawChild(child, args);
	}
}


void Widget::drawLayer(const DrawArgs& args, int layer) {
	// Iterate children
	for (Widget* child : children) {
		// Don't draw if invisible
		if (!child->isVisible())
			continue;
		// Don't draw if child is outside clip box
		if (!args.clipBox.intersects(child->box))
			continue;

		drawChild(child, args, layer);
	}
}


void Widget::drawChild(Widget* child, const DrawArgs& args, int layer) {
	DrawArgs childArgs = args;
	// Intersect child clip box with self
	childArgs.clipBox = childArgs.clipBox.intersect(child->box);
	// Offset clip box by child pos
	childArgs.clipBox.pos = childArgs.clipBox.pos.minus(child->box.pos);

	nvgSave(args.vg);
	nvgTranslate(args.vg, child->box.pos.x, child->box.pos.y);

	if (layer == 0) {
		child->draw(childArgs);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		// Call deprecated draw function, which does nothing by default
		child->draw(args.vg);
#pragma GCC diagnostic pop
	}
	else {
		child->drawLayer(childArgs, layer);
	}

	nvgRestore(args.vg);
}


} // namespace widget
} // namespace rack
