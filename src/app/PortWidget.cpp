#include "app/PortWidget.hpp"
#include "app/Scene.hpp"
#include "window.hpp"
#include "app.hpp"
#include "history.hpp"
#include "component.hpp"


namespace rack {
namespace app {


struct PlugLight : MultiLightWidget {
	PlugLight() {
		addBaseColor(component::GREEN);
		addBaseColor(component::RED);
		addBaseColor(component::BLUE);
		box.size = math::Vec(8, 8);
		bgColor = component::BLACK_TRANSPARENT;
	}
};


PortWidget::PortWidget() {
	plugLight = new PlugLight;
}

PortWidget::~PortWidget() {
	// plugLight is not a child and is thus owned by the PortWidget, so we need to delete it here
	delete plugLight;
	// HACK
	if (module)
		APP->scene->rackWidget->clearCablesOnPort(this);
}

void PortWidget::step() {
	if (!module)
		return;

	std::vector<float> values(3);
	// Input and Outputs are not virtual, so we can't cast to Port to make this less redundant.
	if (type == OUTPUT) {
		values[0] = module->outputs[portId].plugLights[0].getBrightness();
		values[1] = module->outputs[portId].plugLights[1].getBrightness();
		values[2] = module->outputs[portId].plugLights[2].getBrightness();
	}
	else {
		values[0] = module->inputs[portId].plugLights[0].getBrightness();
		values[1] = module->inputs[portId].plugLights[1].getBrightness();
		values[2] = module->inputs[portId].plugLights[2].getBrightness();
	}
	plugLight->setBrightnesses(values);
}

void PortWidget::draw(const widget::DrawContext &ctx) {
	CableWidget *cw = APP->scene->rackWidget->incompleteCable;
	if (cw) {
		// Dim the PortWidget if the active cable cannot plug into this PortWidget
		if (type == OUTPUT ? cw->outputPort : cw->inputPort)
			nvgGlobalAlpha(ctx.vg, 0.5);
	}
	widget::Widget::draw(ctx);
}

void PortWidget::onButton(const event::Button &e) {
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		CableWidget *cw = APP->scene->rackWidget->getTopCable(this);
		if (cw) {
			// history::CableRemove
			history::CableRemove *h = new history::CableRemove;
			h->setCable(cw);
			APP->history->push(h);

			APP->scene->rackWidget->removeCable(cw);
			delete cw;
		}
	}
	e.consume(this);
}

void PortWidget::onDragStart(const event::DragStart &e) {
	CableWidget *cw = NULL;
	if (type == OUTPUT && (APP->window->getMods() & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
		// Keep cable NULL
	}
	else {
		// Grab cable on top of stack
		cw = APP->scene->rackWidget->getTopCable(this);
	}

	if (cw) {
		// history::CableRemove
		history::CableRemove *h = new history::CableRemove;
		h->setCable(cw);
		APP->history->push(h);

		// Disconnect and reuse existing cable
		APP->scene->rackWidget->removeCable(cw);
		if (type == OUTPUT)
			cw->setOutput(NULL);
		else
			cw->setInput(NULL);
	}
	else {
		// Create a new cable
		cw = new CableWidget;
		if (type == OUTPUT)
			cw->setOutput(this);
		else
			cw->setInput(this);
	}
	APP->scene->rackWidget->setIncompleteCable(cw);
}

void PortWidget::onDragEnd(const event::DragEnd &e) {
	CableWidget *cw = APP->scene->rackWidget->releaseIncompleteCable();
	if (cw->isComplete()) {
		APP->scene->rackWidget->addCable(cw);

		// history::CableAdd
		history::CableAdd *h = new history::CableAdd;
		h->setCable(cw);
		APP->history->push(h);
	}
	else {
		delete cw;
	}
}

void PortWidget::onDragDrop(const event::DragDrop &e) {
	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		if (APP->scene->rackWidget->getTopCable(this))
			return;
	}

	CableWidget *cw = APP->scene->rackWidget->incompleteCable;
	if (cw) {
		cw->hoveredOutputPort = cw->hoveredInputPort = NULL;
		if (type == OUTPUT)
			cw->setOutput(this);
		else
			cw->setInput(this);
	}
}

void PortWidget::onDragEnter(const event::DragEnter &e) {
	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		if (APP->scene->rackWidget->getTopCable(this))
			return;
	}

	CableWidget *cw = APP->scene->rackWidget->incompleteCable;
	if (cw) {
		if (type == OUTPUT)
			cw->hoveredOutputPort = this;
		else
			cw->hoveredInputPort = this;
	}
}

void PortWidget::onDragLeave(const event::DragLeave &e) {
	PortWidget *originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	CableWidget *cw = APP->scene->rackWidget->incompleteCable;
	if (cw) {
		if (type == OUTPUT)
			cw->hoveredOutputPort = NULL;
		else
			cw->hoveredInputPort = NULL;
	}
}


} // namespace app
} // namespace rack
