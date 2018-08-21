//***********************************************************************************************
//Geodesics: A modular collection for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from Valley Rack Free by Dale Johnson
//See ./LICENSE.txt for all licenses
//***********************************************************************************************


#ifndef IM_WIDGETS_HPP
#define IM_WIDGETS_HPP

#include "rack.hpp"
#include "window.hpp"

using namespace rack;

namespace rack_plugin_Geodesics {

// ******** Dynamic SVGPanel ******** 

struct PanelBorderWidget : TransparentWidget { // from SVGPanel.cpp
	int** expWidth = nullptr;
	void draw(NVGcontext *vg) override;
};

struct DynamicSVGPanel : FramebufferWidget { // like SVGPanel (in app.hpp and SVGPanel.cpp) but with dynmically assignable panel
    int* mode;
    int oldMode;
	int* expWidth;
    std::vector<std::shared_ptr<SVG>> panels;
    SVGWidget* visiblePanel;
    PanelBorderWidget* border;
    DynamicSVGPanel();
    void addPanel(std::shared_ptr<SVG> svg);
    void step() override;
};



// ******** Dynamic Ports ********

// General Dynamic Port creation
template <class TDynamicPort>
TDynamicPort* createDynamicPort(Vec pos, Port::PortType type, Module *module, int portId,
                                               int* mode) {
	TDynamicPort *dynPort = Port::create<TDynamicPort>(pos, type, module, portId);
	dynPort->mode = mode;
	dynPort->box.pos = dynPort->box.pos.minus(dynPort->box.size.div(2));// centering
	return dynPort;
}

// Dynamic SVGPort (see SVGPort in app.hpp and SVGPort.cpp)
struct DynamicSVGPort : SVGPort {
    int* mode;
    int oldMode;
    std::vector<std::shared_ptr<SVG>> frames;

    DynamicSVGPort();
    void addFrame(std::shared_ptr<SVG> svg);
    void step() override;
};



// ******** Dynamic Params ********

// General Dynamic Param creation
template <class TDynamicParam>
TDynamicParam* createDynamicParam(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue,
                                               int* mode) {
	TDynamicParam *dynParam = ParamWidget::create<TDynamicParam>(pos, module, paramId, minValue, maxValue, defaultValue);
	dynParam->mode = mode;
	dynParam->box.pos = dynParam->box.pos.minus(dynParam->box.size.div(2));// centering
	return dynParam;
}

// General Dynamic Param creation version two with float* instead of one int*
template <class TDynamicParam>
TDynamicParam* createDynamicParam2(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue,
                                               float* wider, float* paramReadRequest) {
	TDynamicParam *dynParam = ParamWidget::create<TDynamicParam>(pos, module, paramId, minValue, maxValue, defaultValue);
	dynParam->wider = wider;
	dynParam->paramReadRequest = paramReadRequest;
	return dynParam;
}

// Dynamic SVGSwitch (see SVGSwitch in app.hpp and SVGSwitch.cpp)
struct DynamicSVGSwitch : SVGSwitch {
    int* mode;
    int oldMode;
	std::vector<std::shared_ptr<SVG>> framesAll;
	
    DynamicSVGSwitch();
	void addFrameAll(std::shared_ptr<SVG> svg);
    void step() override;
};

// Dynamic SVGKnob (see SVGKnob in app.hpp and SVGKnob.cpp)
struct DynamicSVGKnob : SVGKnob {
    int* mode;
    int oldMode;
	std::vector<std::shared_ptr<SVG>> framesAll;
	SVGWidget* effect;
	float orientationAngle;
	
    DynamicSVGKnob();
	void addFrameAll(std::shared_ptr<SVG> svg);
	void addEffect(std::shared_ptr<SVG> svg);// do this last
    void step() override;
};

} // namespace rack_plugin_Geodesics

using namespace rack_plugin_Geodesics;


#endif
