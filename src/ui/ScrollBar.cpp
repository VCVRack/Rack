#include <ui/ScrollBar.hpp>
#include <ui/ScrollWidget.hpp>
#include <context.hpp>
#include <window.hpp>


namespace rack {
namespace ui {


// Internal not currently used


ScrollBar::ScrollBar() {
	box.size = math::Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
}


ScrollBar::~ScrollBar() {
}


void ScrollBar::draw(const DrawArgs& args) {
	ScrollWidget* sw = dynamic_cast<ScrollWidget*>(parent);
	assert(sw);

	BNDwidgetState state = BND_DEFAULT;
	if (APP->event->getHoveredWidget() == this)
		state = BND_HOVER;
	if (APP->event->getDraggedWidget() == this)
		state = BND_ACTIVE;

	float handleOffset = sw->getHandleOffset().get(vertical);
	float handleSize = sw->getHandleSize().get(vertical);
	bndScrollBar(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, handleOffset, handleSize);
}


void ScrollBar::onButton(const event::Button& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
		ScrollWidget* sw = dynamic_cast<ScrollWidget*>(parent);
		assert(sw);

		float pos = e.pos.get(vertical);
		pos /= box.size.get(vertical);
		float handleOffset = sw->getHandleOffset().get(vertical);
		float handleSize = sw->getHandleSize().get(vertical);
		float handlePos = math::rescale(handleOffset, 0.f, 1.f, handleSize / 2.f, 1.f - handleSize / 2.f);
		math::Rect offsetBound = sw->getContainerOffsetBound();

		// Check if user clicked on handle
		if (std::fabs(pos - handlePos) > handleSize / 2.f) {
			// Jump to absolute position of the handle
			float offset = math::rescale(pos, handleSize / 2.f, 1.f - handleSize / 2.f, 0.f, 1.f);
			sw->offset.get(vertical) = sw->containerBox.pos.get(vertical) + offset * (sw->containerBox.size.get(vertical) - sw->box.size.get(vertical));
		}
	}
	OpaqueWidget::onButton(e);
}


void ScrollBar::onDragStart(const event::DragStart& e) {
}


void ScrollBar::onDragEnd(const event::DragEnd& e) {
}


void ScrollBar::onDragMove(const event::DragMove& e) {
	ScrollWidget* sw = dynamic_cast<ScrollWidget*>(parent);
	assert(sw);

	// Move handle absolutely.
	float mouseDelta = e.mouseDelta.get(vertical);
	mouseDelta /= getAbsoluteZoom();

	float handleSize = sw->getHandleSize().get(vertical);
	float handleBound = (1.f - handleSize) * box.size.get(vertical);
	float offsetBound = sw->getContainerOffsetBound().size.get(vertical);
	float offsetDelta = mouseDelta * offsetBound / handleBound;
	sw->offset.get(vertical) += offsetDelta;
}


} // namespace ui
} // namespace rack
