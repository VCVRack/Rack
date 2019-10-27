#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <app/PortWidget.hpp>
#include <app/ModuleWidget.hpp>
#include <engine/Cable.hpp>
#include <map>


namespace rack {
namespace app {


struct CableWidget : widget::OpaqueWidget {
	PortWidget* inputPort = NULL;
	PortWidget* outputPort = NULL;
	PortWidget* hoveredInputPort = NULL;
	PortWidget* hoveredOutputPort = NULL;
	/** Owned. */
	engine::Cable* cable = NULL;
	NVGcolor color;

	CableWidget();
	~CableWidget();
	bool isComplete();
	/** From input/output ports, re-creates a cable and adds it to the Engine. */
	void updateCable();
	/** From a cable, sets the input/output ports.
	Cable must already be added to the Engine.
	*/
	void setCable(engine::Cable* cable);
	math::Vec getInputPos();
	math::Vec getOutputPos();
	json_t* toJson();
	void fromJson(json_t* rootJ);
	void draw(const DrawArgs& args) override;
	void drawPlugs(const DrawArgs& args);
};


} // namespace app
} // namespace rack
