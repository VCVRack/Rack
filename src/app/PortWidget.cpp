#include "app/PortWidget.hpp"
#include "app/Scene.hpp"
#include "window.hpp"
#include "context.hpp"
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
	context()->scene->rackWidget->wireContainer->removeAllWires(this);
}

void PortWidget::step() {
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
	WireWidget *activeWire = context()->scene->rackWidget->wireContainer->activeWire;
	if (activeWire) {
		// Dim the PortWidget if the active wire cannot plug into this PortWidget
		if (type == INPUT ? activeWire->inputPort : activeWire->outputPort)
			nvgGlobalAlpha(vg, 0.5);
	}
}

void PortWidget::onButton(event::Button &e) {
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		context()->scene->rackWidget->wireContainer->removeTopWire(this);

		// HACK
		// Update hovered*PortWidget of active wire if applicable
		// event::DragEnter eDragEnter;
		// onDragEnter(eDragEnter);
	}
	e.target = this;
}

void PortWidget::onDragStart(event::DragStart &e) {
	// Try to grab wire on top of stack
	WireWidget *wire = NULL;
	if (type == INPUT || !context()->window->isModPressed()) {
		wire = context()->scene->rackWidget->wireContainer->getTopWire(this);
	}

	if (wire) {
		// Disconnect existing wire
		(type == INPUT ? wire->inputPort : wire->outputPort) = NULL;
		wire->updateWire();
	}
	else {
		// Create a new wire
		wire = new WireWidget;
		(type == INPUT ? wire->inputPort : wire->outputPort) = this;
	}
	context()->scene->rackWidget->wireContainer->setActiveWire(wire);
}

void PortWidget::onDragEnd(event::DragEnd &e) {
	// FIXME
	// If the source PortWidget is deleted, this will be called, removing the cable
	context()->scene->rackWidget->wireContainer->commitActiveWire();
}

void PortWidget::onDragDrop(event::DragDrop &e) {
	PortWidget *originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	// Fake onDragEnter because onDragLeave is triggered immediately before this one
	event::DragEnter eDragEnter;
	eDragEnter.origin = e.origin;
	onDragEnter(eDragEnter);
}

void PortWidget::onDragEnter(event::DragEnter &e) {
	PortWidget *originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		WireWidget *topWire = context()->scene->rackWidget->wireContainer->getTopWire(this);
		if (topWire)
			return;
	}

	WireWidget *activeWire = context()->scene->rackWidget->wireContainer->activeWire;
	if (activeWire) {
		(type == INPUT ? activeWire->hoveredInputPort : activeWire->hoveredOutputPort) = this;
	}
}

void PortWidget::onDragLeave(event::DragLeave &e) {
	PortWidget *originPort = dynamic_cast<PortWidget*>(e.origin);
	if (!originPort)
		return;

	WireWidget *activeWire = context()->scene->rackWidget->wireContainer->activeWire;
	if (activeWire) {
		(type == INPUT ? activeWire->hoveredInputPort : activeWire->hoveredOutputPort) = NULL;
	}
}


} // namespace rack
