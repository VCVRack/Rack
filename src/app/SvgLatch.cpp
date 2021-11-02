#include <app/SvgLatch.hpp>


namespace rack {
namespace app {


SvgLatch::SvgLatch() {
}


SvgLatch::~SvgLatch() {
}


void SvgLatch::onDragStart(const DragStartEvent& e) {
	// Use Switch behavior
	Switch::onDragStart(e);
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Set down frame
	if (frames.size() >= 2) {
		sw->setSvg(frames[1]);
		fb->setDirty();
	}
}


void SvgLatch::onDragEnd(const DragEndEvent& e) {
	// Use Switch behavior
	Switch::onDragEnd(e);
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Set up frame
	if (frames.size() >= 1) {
		sw->setSvg(frames[0]);
		fb->setDirty();
	}
}


void SvgLatch::onChange(const ChangeEvent& e) {
	// Bypass SvgSwitch behavior
	Switch::onChange(e);
}


} // namespace app
} // namespace rack
