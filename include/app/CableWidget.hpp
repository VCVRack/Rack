#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "app/PortWidget.hpp"
#include "app/ModuleWidget.hpp"
#include "engine/Cable.hpp"
#include <map>


namespace rack {


struct CableWidget : OpaqueWidget {
	PortWidget *outputPort = NULL;
	PortWidget *inputPort = NULL;
	PortWidget *hoveredOutputPort = NULL;
	PortWidget *hoveredInputPort = NULL;
	Cable *cable;
	NVGcolor color;

	CableWidget();
	~CableWidget();
	bool isComplete();
	void setOutputPort(PortWidget *outputPort);
	void setInputPort(PortWidget *inputPort);
	math::Vec getOutputPos();
	math::Vec getInputPos();
	json_t *toJson();
	void fromJson(json_t *rootJ, const std::map<int, ModuleWidget*> &moduleWidgets);
	void draw(NVGcontext *vg) override;
	void drawPlugs(NVGcontext *vg);
};


} // namespace rack
