#pragma once
#include <map>
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <app/PortWidget.hpp>
#include <app/ModuleWidget.hpp>
#include <engine/Cable.hpp>


namespace rack {
namespace app {


struct CableWidget : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	/** Owned. */
	engine::Cable* cable = NULL;
	NVGcolor color;
	PortWidget* inputPort = NULL;
	PortWidget* outputPort = NULL;
	PortWidget* hoveredInputPort = NULL;
	PortWidget* hoveredOutputPort = NULL;

	CableWidget();
	~CableWidget();
	void setNextCableColor();
	bool isComplete();
	/** Based on the input/output ports, re-creates the cable and removes/adds it to the Engine. */
	void updateCable();
	/** From a cable, sets the input/output ports.
	Cable must already be added to the Engine.
	Adopts ownership.
	*/
	void setCable(engine::Cable* cable);
	math::Vec getInputPos();
	math::Vec getOutputPos();
	json_t* toJson();
	void fromJson(json_t* rootJ);
	void draw(const DrawArgs& args) override;
	void drawPlugs(const DrawArgs& args);
	engine::Cable* releaseCable();
};


} // namespace app
} // namespace rack
