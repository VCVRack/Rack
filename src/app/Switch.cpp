#include <app/Switch.hpp>
#include <context.hpp>
#include <app/Scene.hpp>
#include <random.hpp>
#include <history.hpp>


namespace rack {
namespace app {


void Switch::initParamQuantity() {
	ParamWidget::initParamQuantity();
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		pq->snapEnabled = true;
		if (momentary) {
			pq->resetEnabled = false;
			pq->randomizeEnabled = false;
		}
	}
}

void Switch::step() {
	engine::ParamQuantity* pq = getParamQuantity();
	if (momentaryPressed) {
		momentaryPressed = false;
		// Wait another frame.
	}
	else if (momentaryReleased) {
		momentaryReleased = false;
		if (pq) {
			// Set to minimum value
			pq->setMin();
		}
	}
	ParamWidget::step();
}

void Switch::onDoubleClick(const event::DoubleClick& e) {
	// Don't reset parameter on double-click
}

void Switch::onDragStart(const event::DragStart& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	engine::ParamQuantity* pq = getParamQuantity();
	if (momentary) {
		if (pq) {
			// Set to maximum value
			pq->setMax();
			momentaryPressed = true;
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

void Switch::onDragEnd(const event::DragEnd& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (momentary) {
		momentaryReleased = true;
	}
}


} // namespace app
} // namespace rack
