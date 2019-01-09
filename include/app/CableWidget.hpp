#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "app/PortWidget.hpp"
#include "engine/Cable.hpp"


namespace rack {


struct CableWidget : OpaqueWidget {
	PortWidget *outputPort = NULL;
	PortWidget *inputPort = NULL;
	PortWidget *hoveredOutputPort = NULL;
	PortWidget *hoveredInputPort = NULL;
	Cable *cable = NULL;
	NVGcolor color;

	CableWidget();
	~CableWidget();
	/** Synchronizes the plugged state of the widget to the owned cable */
	void updateCable();
	math::Vec getOutputPos();
	math::Vec getInputPos();
	json_t *toJson();
	void fromJson(json_t *rootJ);
	void draw(NVGcontext *vg) override;
	void drawPlugs(NVGcontext *vg);
};


} // namespace rack
