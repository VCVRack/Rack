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
	PortWidget* outputPort = NULL;
	PortWidget* inputPort = NULL;
	PortWidget* hoveredOutputPort = NULL;
	PortWidget* hoveredInputPort = NULL;
	/** Owned. */
	engine::Cable* cable;
	NVGcolor color;

	CableWidget();
	~CableWidget();
	bool isComplete();
	void setOutput(PortWidget* outputPort);
	void setInput(PortWidget* inputPort);
	math::Vec getOutputPos();
	math::Vec getInputPos();
	json_t* toJson();
	void fromJson(json_t* rootJ);
	void draw(const DrawArgs& args) override;
	void drawPlugs(const DrawArgs& args);
};


} // namespace app
} // namespace rack
