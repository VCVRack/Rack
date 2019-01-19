#include "app/PortWidget.hpp"
#include "app/Scene.hpp"
#include "window.hpp"
#include "app.hpp"
#include "componentlibrary.hpp"


namespace rack {


struct PlugLight : MultiLightWidget {
	PlugLight() {
		addBaseColor(color::GREEN);
		addBaseColor(color::RED);
		box.size = math::Vec(8, 8);
		bgColor = color::BLACK_TRANSPARENT;
	}
};


PortWidget::PortWidget() {
	plugLight = new PlugLight;
}

PortWidget::~PortWidget() {
	// plugLight is not a child and is thus owned by the PortWidget, so we need to delete it here
	delete plugLight;
	// HACK
	// See ModuleWidget::~ModuleWidget for description
	if (module)
		app()->scene->rackWidget->cableContainer->removeAllCables(this);
}

void PortWidget::step() {
	if (!module)
		return;

	std::vector<float> values(2);
	if (type == INPUT) {
		values[0] = module->inputs[portId].plugLights[0].getBrightness();
		values[1] = module->inputs[portId].plugLights[1].getBrightness();
	}
	else {
		values[0] = module->outputs[portId].plugLights[0].getBrightness();
		values[1] = module->outputs[portId].plugLights[1].getBrightness();
	}
	plugLight->setValues(values);
}

void PortWidget::draw(NVGcontext *vg) {
	CableWidget *activeCable = app()->scene->rackWidget->cableContainer->activeCable;
	if (activeCable) {
		// Dim the PortWidget if the active cable cannot plug into this PortWidget
		if (type == INPUT ? activeCable->inputPort : activeCable->outputPort)
			nvgGlobalAlpha(vg, 0.5);
	}
	Widget::draw(vg);
}

void PortWidget::onButton(const event::Button &e) {
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		app()->scene->rackWidget->cableContainer->removeTopCable(this);

		// HACK
		// Update hovered*PortWidget of active cable if applicable
		// event::DragEnter eDragEnter;
		// onDragEnter(eDragEnter);
	}
	e.consume(this);
}

void PortWidget::onDragStart(const event::DragStart &e) {
	// Try to grab cable on top of stack
	CableWidget *cable = NULL;
	if (type == OUTPUT && (app()->window->getMods() & WINDOW_MOD_MASK) == WINDOW_MOD_CTRL) {
		// Keep cable NULL
	}
	else {
		cable = app()->scene->rackWidget->cableContainer->getTopCable(this);
	}

	if (cable) {
		// Disconnect existing cable
		(type == INPUT ? cable->inputPort : cable->outputPort) = NULL;
		cable->updateCable();
	}
	else {
		// Create a new cable
		cable = new CableWidget;
		(type == INPUT ? cable->inputPort : cable->outputPort) = this;
	}
	app()->scene->rackWidget->cableContainer->setActiveCable(cable);
}

void PortWidget::onDragEnd(const event::DragEnd &e) {
	// FIXME
	// If the source PortWidget is deleted, this will be called, removing the cable
	app()->scene->rackWidget->cableContainer->commitActiveCable();
}

void PortWidget::onDragDrop(const event::DragDrop &e) {
	PortWidget *originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	setHovered();
}

void PortWidget::onDragEnter(const event::DragEnter &e) {
	PortWidget *originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	setHovered();
}

void PortWidget::onDragLeave(const event::DragLeave &e) {
	PortWidget *originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	CableWidget *activeCable = app()->scene->rackWidget->cableContainer->activeCable;
	if (activeCable) {
		(type == INPUT ? activeCable->hoveredInputPort : activeCable->hoveredOutputPort) = NULL;
	}
}

void PortWidget::setHovered() {
	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		CableWidget *topCable = app()->scene->rackWidget->cableContainer->getTopCable(this);
		if (topCable)
			return;
	}

	CableWidget *activeCable = app()->scene->rackWidget->cableContainer->activeCable;
	if (activeCable) {
		(type == INPUT ? activeCable->hoveredInputPort : activeCable->hoveredOutputPort) = this;
	}
}


} // namespace rack
