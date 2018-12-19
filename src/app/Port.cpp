#include "app/Port.hpp"
#include "app/Scene.hpp"
#include "window.hpp"
#include "context.hpp"
#include "componentlibrary.hpp"


namespace rack {


struct PlugLight : MultiLightWidget {
	PlugLight() {
		addBaseColor(color::GREEN);
		addBaseColor(color::RED);
		box.size = Vec(8, 8);
		bgColor = color::BLACK_TRANSPARENT;
	}
};


Port::Port() {
	plugLight = new PlugLight;
}

Port::~Port() {
	// plugLight is not a child and is thus owned by the Port, so we need to delete it here
	delete plugLight;
	context()->scene->rackWidget->wireContainer->removeAllWires(this);
}

void Port::step() {
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

void Port::draw(NVGcontext *vg) {
	WireWidget *activeWire = context()->scene->rackWidget->wireContainer->activeWire;
	if (activeWire) {
		// Dim the Port if the active wire cannot plug into this Port
		if (type == INPUT ? activeWire->inputPort : activeWire->outputPort)
			nvgGlobalAlpha(vg, 0.5);
	}
}

void Port::onButton(event::Button &e) {
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		context()->scene->rackWidget->wireContainer->removeTopWire(this);

		// HACK
		// Update hovered*Port of active wire if applicable
		event::DragEnter eDragEnter;
		onDragEnter(eDragEnter);
	}
	e.target = this;
}

void Port::onDragStart(event::DragStart &e) {
	// Try to grab wire on top of stack
	WireWidget *wire = context()->scene->rackWidget->wireContainer->getTopWire(this);
	if (type == OUTPUT && context()->window->isModPressed()) {
		wire = NULL;
	}

	if (wire) {
		// Disconnect existing wire
		if (type == INPUT)
			wire->inputPort = NULL;
		else
			wire->outputPort = NULL;
		wire->updateWire();
	}
	else {
		// Create a new wire
		wire = new WireWidget;
		if (type == INPUT)
			wire->inputPort = this;
		else
			wire->outputPort = this;
	}
	context()->scene->rackWidget->wireContainer->setActiveWire(wire);
}

void Port::onDragEnd(event::DragEnd &e) {
	// FIXME
	// If the source Port is deleted, this will be called, removing the cable
	context()->scene->rackWidget->wireContainer->commitActiveWire();
}

void Port::onDragDrop(event::DragDrop &e) {
}

void Port::onDragEnter(event::DragEnter &e) {
	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		WireWidget *topWire = context()->scene->rackWidget->wireContainer->getTopWire(this);
		if (topWire)
			return;
	}

	WireWidget *activeWire = context()->scene->rackWidget->wireContainer->activeWire;
	if (activeWire) {
		if (type == INPUT)
			activeWire->hoveredInputPort = this;
		else
			activeWire->hoveredOutputPort = this;
	}
}

void Port::onDragLeave(event::DragLeave &e) {
	WireWidget *activeWire = context()->scene->rackWidget->wireContainer->activeWire;
	if (activeWire) {
		if (type == INPUT)
			activeWire->hoveredInputPort = NULL;
		else
			activeWire->hoveredOutputPort = NULL;
	}
}


} // namespace rack
