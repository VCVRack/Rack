#ifndef WIDGET_MULTISCOPE_HPP
#define WIDGET_MULTISCOPE_HPP

#include "global_pre.hpp"
#include "Features.hpp"
#include "global_ui.hpp"

#if USE_NEW_SCOPE


#include "rack.hpp"
using namespace rack;

#include <string.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "dsp/digital.hpp"
#include "Module_multiScope.hpp"

/// TODO: Widget/Module wide invert setting



struct TSScopeModuleResizeHandle : Widget {
	float minWidth;
	bool right = false;
	float dragX;
	Rect originalBox;

	TS_PadSwitch* displayToggleBtn = NULL;
	ColorValueLight* displayToggleLED = NULL;
	TS_PadSwitch* colorDisplayToggleBtn = NULL;
	ColorValueLight* colorDisplayToggleLED = NULL;

	TSScopeModuleResizeHandle(float minWidth) {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
		this->minWidth = minWidth;
		return;
	}
	TSScopeModuleResizeHandle(float minWidth, SVGPanel* bgPanel) : TSScopeModuleResizeHandle(minWidth)
	{
		addChild(bgPanel);
		return;
	}
	TSScopeModuleResizeHandle(float minWidth, SVGPanel* bgPanel, TS_PadSwitch* displayBtn, ColorValueLight* displayLED) : TSScopeModuleResizeHandle(minWidth, bgPanel)
	{
		this->displayToggleBtn = displayBtn;
		this->displayToggleLED = displayLED;
		return;
	}
	// Set positions of controls relative to our position (even though we don't own these).
	void setChildPositions()
	{
		if (displayToggleBtn)
		{
			float margin = (this->box.size.x - displayToggleBtn->box.size.x) / 2.0;
			if (margin < 0)
				margin = 0;
			displayToggleBtn->box.pos.x = this->box.pos.x + margin;
		}
		if (displayToggleLED)
		{
			float margin = (this->box.size.x - displayToggleLED->box.size.x) / 2.0;
			if (margin < 0)
				margin = 0;
			displayToggleLED->box.pos.x = this->box.pos.x + margin;
		}
		if (colorDisplayToggleBtn)
		{
			float margin = (this->box.size.x - colorDisplayToggleBtn->box.size.x) / 2.0;
			if (margin < 0)
				margin = 0;
			colorDisplayToggleBtn->box.pos.x = this->box.pos.x + margin;
		}
		if (colorDisplayToggleLED)
		{
			float margin = (this->box.size.x - colorDisplayToggleLED->box.size.x) / 2.0;
			if (margin < 0)
				margin = 0;
			colorDisplayToggleLED->box.pos.x = this->box.pos.x + margin;
		}
		return;
	}


	void onMouseDown(EventMouseDown &e) override {
		if (e.button == 0) {
			e.consumed = true;
			e.target = this;
		}
	}
	void onDragStart(EventDragStart &e) override {
		dragX = rack::global_ui->app.gRackWidget->lastMousePos.x;
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();
		originalBox = m->box;
	}
	void onDragMove(EventDragMove &e) override {
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();

		float newDragX = rack::global_ui->app.gRackWidget->lastMousePos.x;
		float deltaX = newDragX - dragX;

		Rect newBox = originalBox;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}
		rack::global_ui->app.gRackWidget->requestModuleBox(m, newBox);
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScopeWidget
// Widget for the scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct multiScopeWidget : ModuleWidget {
	Panel* panel;
	TSScopeModuleResizeHandle* rightHandle;
	TransparentWidget* display[TROWA_SCOPE_NUM_WAVEFORMS];
	TS_Panel* screenContainer;
	TS_ColorSlider* colorSliders[3];
	TSScopeDisplay* scopeInfoDisplay;
	// Keep screw references to move them.
	ScrewBlack* rhsScrews[2];
	int inputAreaWidth;
	bool plugLightsEnabled = true;
	// Keep references to our ports to disable lights or change the colors.
	TS_Port* inputPorts[multiScope::NUM_INPUTS];
	// Keep references to our scale knobs [0: x, 1: y]
	TS_TinyBlackKnob* scaleKnobs[TROWA_SCOPE_NUM_WAVEFORMS][2];
	ColorValueLight* fillColorLEDs[TROWA_SCOPE_NUM_WAVEFORMS];

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// multiScopeWidget()
	// Instantiate a multiScope widget. v0.60 must have module as param.
	// @scopeModule : (IN) Pointer to the multiScope module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	multiScopeWidget(multiScope* scopeModule);
	void step() override;
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
	//Menu *createContextMenu() override;
};

#endif // use new scope

#endif // end if not defined
