#include <app/Switch.hpp>
#include <context.hpp>
#include <app/Scene.hpp>
#include <random.hpp>
#include <history.hpp>


namespace rack {
namespace app {


struct Switch::Internal {
	/** Whether momentary switch was pressed this frame. */
	bool momentaryPressed = false;
	/** Whether momentary switch was released this frame. */
	bool momentaryReleased = false;
};


Switch::Switch() {
	internal = new Internal;
}

Switch::~Switch() {
	delete internal;
}

void Switch::initParamQuantity() {
	ParamWidget::initParamQuantity();
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		pq->snapEnabled = true;
		pq->smoothEnabled = false;
		if (momentary) {
			pq->resetEnabled = false;
			pq->randomizeEnabled = false;
		}
	}
}

void Switch::step() {
	engine::ParamQuantity* pq = getParamQuantity();
	if (internal->momentaryPressed) {
		internal->momentaryPressed = false;
		// Wait another frame.
	}
	else if (internal->momentaryReleased) {
		internal->momentaryReleased = false;
		if (pq) {
			// Set to minimum value
			pq->setMin();
		}
	}
	ParamWidget::step();
}

void Switch::onDoubleClick(const DoubleClickEvent& e) {
	// Don't reset parameter on double-click
	OpaqueWidget::onDoubleClick(e);
}

void Switch::onDragStart(const DragStartEvent& e) {
	ParamWidget::onDragStart(e);

	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	engine::ParamQuantity* pq = getParamQuantity();
	if (momentary) {
		internal->momentaryPressed = true;
		if (pq) {
			// Set to maximum value
			pq->setMax();
		}
	}
	else {
		if (pq) {
			float oldValue = pq->getValue();
			if (pq->isMax()) {
				// Reset value back to minimum
				pq->setMin();
			}
			else {
				// Increment value by 1
				pq->setValue(std::round(pq->getValue()) + 1.f);
			}

			float newValue = pq->getValue();
			if (oldValue != newValue) {
				// Push ParamChange history action
				history::ParamChange* h = new history::ParamChange;
				h->name = "move switch";
				h->moduleId = module->id;
				h->paramId = paramId;
				h->oldValue = oldValue;
				h->newValue = newValue;
				APP->history->push(h);
			}
		}
	}
}

void Switch::onDragEnd(const DragEndEvent& e) {
	ParamWidget::onDragEnd(e);

	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (momentary) {
		internal->momentaryReleased = true;
	}
}


} // namespace app
} // namespace rack
