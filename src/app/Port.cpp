#include "app.hpp"
#include "gui.hpp"
#include "engine.hpp"
#include "componentlibrary.hpp"


namespace rack {


struct PlugLight : MultiLightWidget {
	PlugLight() {
		addBaseColor(COLOR_GREEN);
		addBaseColor(COLOR_RED);
		box.size = Vec(8, 8);
		bgColor = COLOR_BLACK_TRANSPARENT;
	}
};


Port::Port() {
	plugLight = new PlugLight();
}

Port::~Port() {
	// plugLight is not a child and is thus owned by the Port, so we need to delete it here
	delete plugLight;
	gRackWidget->wireContainer->removeAllWires(this);
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
	WireWidget *activeWire = gRackWidget->wireContainer->activeWire;
	if (activeWire) {
		// Dim the Port if the active wire cannot plug into this Port
		if (type == INPUT ? activeWire->inputPort : activeWire->outputPort)
			nvgGlobalAlpha(vg, 0.5);
	}
}

void Port::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		gRackWidget->wireContainer->removeTopWire(this);

		// HACK
		// Update hovered*Port of active wire if applicable
		EventDragEnter e;
		onDragEnter(e);
	}
	e.consumed = true;
	e.target = this;
}

void Port::onDragStart(EventDragStart &e) {
	// Try to grab wire on top of stack
	WireWidget *wire = gRackWidget->wireContainer->getTopWire(this);
	if (type == OUTPUT && guiIsModPressed()) {
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
		wire = new WireWidget();
		if (type == INPUT)
			wire->inputPort = this;
		else
			wire->outputPort = this;
	}
	gRackWidget->wireContainer->setActiveWire(wire);
}

void Port::onDragEnd(EventDragEnd &e) {
	// FIXME
	// If the source Port is deleted, this will be called, removing the cable
	gRackWidget->wireContainer->commitActiveWire();
}

void Port::onDragDrop(EventDragDrop &e) {
}

void Port::onDragEnter(EventDragEnter &e) {
	// Reject ports if this is an input port and something is already plugged into it
	if (type == INPUT) {
		WireWidget *topWire = gRackWidget->wireContainer->getTopWire(this);
		if (topWire)
			return;
	}

	WireWidget *activeWire = gRackWidget->wireContainer->activeWire;
	if (activeWire) {
		if (type == INPUT)
			activeWire->hoveredInputPort = this;
		else
			activeWire->hoveredOutputPort = this;
	}
}

void Port::onDragLeave(EventDragEnter &e) {
	WireWidget *activeWire = gRackWidget->wireContainer->activeWire;
	if (activeWire) {
		if (type == INPUT)
			activeWire->hoveredInputPort = NULL;
		else
			activeWire->hoveredOutputPort = NULL;
	}
}


} // namespace rack
