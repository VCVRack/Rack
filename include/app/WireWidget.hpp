#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "app/PortWidget.hpp"
#include "engine/Wire.hpp"


namespace rack {


struct WireWidget : OpaqueWidget {
	PortWidget *outputPort = NULL;
	PortWidget *inputPort = NULL;
	PortWidget *hoveredOutputPort = NULL;
	PortWidget *hoveredInputPort = NULL;
	Wire *wire = NULL;
	NVGcolor color;

	WireWidget();
	~WireWidget();
	/** Synchronizes the plugged state of the widget to the owned wire */
	void updateWire();
	math::Vec getOutputPos();
	math::Vec getInputPos();
	json_t *toJson();
	void fromJson(json_t *rootJ);
	void draw(NVGcontext *vg) override;
	void drawPlugs(NVGcontext *vg);
};


} // namespace rack
