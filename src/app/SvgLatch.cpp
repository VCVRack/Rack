#include <app/SvgLatch.hpp>


namespace rack {
namespace app {


struct SvgLatch::Internal {
	bool unlatch = false;
};


SvgLatch::SvgLatch() {
	internal = new Internal;
}


SvgLatch::~SvgLatch() {
	delete internal;
}


void SvgLatch::onDragStart(const DragStartEvent& e) {
	ParamWidget::onDragStart(e);
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Set value
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		if (pq->isMax()) {
			internal->unlatch = true;
		}
		else {
			pq->setMax();
		}
	}
	// Set down frame
	if (frames.size() >= 2) {
		sw->setSvg(frames[1]);
		fb->setDirty();
	}
}


void SvgLatch::onDragEnd(const DragEndEvent& e) {
	ParamWidget::onDragEnd(e);
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Set value
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		if (internal->unlatch) {
			internal->unlatch = false;
			pq->setMin();
		}
	}
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
