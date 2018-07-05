#include "Features.hpp"

#if !USE_NEW_SCOPE

#include <string.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "dsp/digital.hpp"
#include "Module_multiScope_Old.hpp"
#include "Widget_multiScope_Old.hpp"

#define SCREW_DIAMETER						 15
#define TROWA_WIDGET_TOP_BAR_HEIGHT	 		 15
#define	TROWA_SCOPE_INPUT_AREA_WIDTH		265  // 260
#define TROWA_SCOPE_MIN_SCOPE_AREA_WIDTH	(240 - RACK_GRID_WIDTH)
#define KNOB_X	0
#define KNOB_Y	1

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScopeWidget(void)
// Instantiate a multiScope widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiScopeWidget::multiScopeWidget(multiScope* scopeModule) : ModuleWidget(scopeModule)
{
	this->module = scopeModule;

	// Calculate Sizes:	
	this->inputAreaWidth = TROWA_SCOPE_INPUT_AREA_WIDTH;
	int borderSize = TROWA_WIDGET_TOP_BAR_HEIGHT; // Size of the top and bottom borders

	// Minimum widget width:
	int w = ((inputAreaWidth + TROWA_SCOPE_MIN_SCOPE_AREA_WIDTH) / RACK_GRID_WIDTH) + 1;
	if (w < 16)
		w = 16;
	int minimumWidgetWidth = w * RACK_GRID_WIDTH; // Smallest we want to ever become	
	
	// Scope default width and widget default width
	int scopeGraphHeight = RACK_GRID_HEIGHT - borderSize*2;
	float scopeGraphDefWidth = 16.0 * scopeGraphHeight / 16.0; // Try to make it 16x9 ...or 16x16 for perfect circle
	w = ((inputAreaWidth + scopeGraphDefWidth) / RACK_GRID_WIDTH) + 1;	
	box.size = Vec(RACK_GRID_WIDTH*w, RACK_GRID_HEIGHT);

	//// Create module:
	//multiScope *module = new multiScope();
	//setModule(module);	
	//
	// Border color for panels
	NVGcolor borderColor = nvgRGBAf(0.25, 0.25, 0.25, 1.0); // nvgRGBAf(0.25, 0.25, 0.25, 1.0);
	float borderWidth = 1;

	////////////////////////////////////
	// LHS Panel (with controls)
	////////////////////////////////////
	{
		TS_SVGPanel *svgpanel = new TS_SVGPanel(/*top*/ borderWidth, /*right*/ 0, /*bottom*/ borderWidth, /*left*/ borderWidth);
		svgpanel->borderColor = borderColor;
		svgpanel->box.size = Vec(inputAreaWidth, RACK_GRID_HEIGHT);
		svgpanel->setBackground(SVG::load(assetPlugin(plugin, "res/multiScope.svg")));
		addChild(svgpanel);
	}

	int controlDisplayHeight = 64; // 56
	int controlDisplayY = 24;
	////////////////////////////////////
	// Labels
	////////////////////////////////////
	{
		TSScopeLabelArea *area = new TSScopeLabelArea();
		area->box.pos = Vec(0, TROWA_SCOPE_CONTROL_START_Y - 14); // wAS 56_24 = 80, OLD CONTROL START WAS 94
		area->box.size = Vec(inputAreaWidth, box.size.y - 50);
		area->module = scopeModule;
		addChild(area);
	}

	////////////////////////////////////
	// Black background for Scope screen
	////////////////////////////////////	
	{
		screenBackground = new TS_Panel();
		screenBackground->backgroundColor = nvgRGB(0, 0, 0);
		screenBackground->box.pos = Vec(inputAreaWidth - 1, 0);
		screenBackground->box.size = Vec(box.size.x - inputAreaWidth + 1, box.size.y);
		dynamic_cast<TS_Panel*>(screenBackground)->setBorderWidth(/*top*/ borderWidth, /*right*/ 0, /*bottom*/ borderWidth, /*left*/ 0);
		dynamic_cast<TS_Panel*>(screenBackground)->borderColor = borderColor;
		addChild(screenBackground);
	}

	Vec tinyBtnSize = Vec(10, 10);

	////////////////////////////////////
	// RHS Resize Handle
	////////////////////////////////////
	{

		TS_SVGPanel *svgpanel = new TS_SVGPanel(/*top*/ borderWidth, /*right*/ borderWidth, /*bottom*/ borderWidth, /*left*/ 0);
		svgpanel->box.size = Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		svgpanel->borderColor = borderColor;
		svgpanel->setBackground(SVG::load(assetPlugin(plugin, "res/multiScope.svg")));

		//========== Single Controls ================
		// Turn Display On/Off (Default is On)
		TS_PadSwitch* displayToggleBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(20, 20), module, multiScope::INFO_DISPLAY_TOGGLE_PARAM, 0, 1, 1));
		displayToggleBtn->box.size = tinyBtnSize;
		displayToggleBtn->value = 1.0;
		ColorValueLight* displayLED = TS_createColorValueLight<ColorValueLight>(displayToggleBtn->box.pos, module, multiScope::INFO_DISPLAY_TOGGLE_LED, tinyBtnSize, TROWA_SCOPE_INFO_DISPLAY_ON_COLOR);
		TSScopeModuleResizeHandle* rightHandle = new TSScopeModuleResizeHandle(minimumWidgetWidth, svgpanel, displayToggleBtn, displayLED);
		rightHandle->right = true;
		rightHandle->box.pos = Vec(box.size.x - rightHandle->box.size.x, 0);
		rightHandle->setChildPositions(); // Adjust the positions of our children.

		TSScopeSideBarLabelArea* rhsLabels = new TSScopeSideBarLabelArea(rightHandle->box.size);
		rhsLabels->box.pos = Vec(0, 0);
		rightHandle->addChild(rhsLabels);

		this->rightHandle = rightHandle;
		addChild(this->rightHandle);	

		addParam(displayToggleBtn);		
		addChild(displayLED);
		scopeModule->lights[multiScope::INFO_DISPLAY_TOGGLE_LED].value = 1.0;
		scopeModule->infoDisplayOnTrigger.state = SchmittTrigger::HIGH;
	}

	////////////////////////////////////
	// Scope Display 
	////////////////////////////////////		
	int wIx = 0;
	int scopeGraphWidth = box.size.x - inputAreaWidth - 2 - this->rightHandle->box.size.x;
	for (wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{
		//info("Adding scope area %d.", wIx);
		multiScopeDisplay* scopeArea = new multiScopeDisplay();
		scopeArea->wIx = wIx;
		scopeArea->module = scopeModule;
		scopeArea->box.pos = Vec(inputAreaWidth + 1, borderSize);
		scopeArea->box.size = Vec(scopeGraphWidth, scopeGraphHeight);// box.size.y - borderSize * 2);
		addChild(scopeArea);
		this->display[wIx] = scopeArea;
	}
	
	////////////////////////////////////
	// Control Display
	////////////////////////////////////
	{
		TSScopeDisplay* sDisplay = new TSScopeDisplay();
		//sDisplay->box.pos = Vec(13, controlDisplayY);;
		//sDisplay->box.size = Vec(inputAreaWidth - 13 * 2, controlDisplayHeight);
		sDisplay->box.pos = Vec(screenBackground->box.pos.x + 15, controlDisplayY);;
		sDisplay->box.size = Vec(scopeGraphWidth - 13 * 2, controlDisplayHeight);
		sDisplay->module = scopeModule;
		sDisplay->visible = true; // By default show display
		sDisplay->originalWidth = sDisplay->box.size.x;
		scopeInfoDisplay = sDisplay;
		addChild(sDisplay);
	}

	////////////////////////////////////
	// Controls:
	////////////////////////////////////			
	//NVGcolor backColor = nvgRGBAf(0.2, 0.2, 0.2, /*alpha */ 0.7);
	//Vec ledSize = Vec(20, 20);

	const int xStart = TROWA_SCOPE_CONTROL_START_X;
	const int yStart = TROWA_SCOPE_CONTROL_START_Y; // 98
	int dx = TROWA_SCOPE_CONTROL_DX; // 35
	int dy = TROWA_SCOPE_CONTROL_DY; // 26
	int shapeSpacingY = TROWA_SCOPE_CONTROL_SHAPE_SPACING; // 8 An extra amount between shapes
	int knobOffset = 5;
	int tinyOffset = 5;
	int x, y = yStart;
	for (wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{
		x = xStart;
		int y2 = y + dy + knobOffset;
		int y3 = y2 + dy;

		// X Controls:
		inputPorts[multiScope::X_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), module, multiScope::X_INPUT + wIx, plugLightsEnabled, scopeModule->waveForms[wIx]->waveColor));
		addInput(inputPorts[multiScope::X_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), module, multiScope::X_POS_PARAM + wIx, TROWA_SCOPE_POS_KNOB_MIN, TROWA_SCOPE_POS_KNOB_MAX, TROWA_SCOPE_POS_X_KNOB_DEF));
		// Keep reference to the scale knobs for synching
		scaleKnobs[wIx][KNOB_X] = dynamic_cast<TS_TinyBlackKnob*>(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y3), module, multiScope::X_SCALE_PARAM + wIx, TROWA_SCOPE_SCALE_KNOB_MIN, TROWA_SCOPE_SCALE_KNOB_MAX, 1.0));
		addParam(scaleKnobs[wIx][KNOB_X]);

		// X-Y Scale Synchronization:
		TS_PadSwitch* linkXYScalesBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset + 23, y3 + tinyOffset), module, multiScope::LINK_XY_SCALE_PARAM + wIx, 0, 1, 0));
		linkXYScalesBtn->box.size = tinyBtnSize;
		addParam(linkXYScalesBtn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + 23, y3 + tinyOffset), module, multiScope::LINK_XY_SCALE_LED + wIx, tinyBtnSize, TROWA_SCOPE_LINK_XY_SCALE_ON_COLOR));
		scopeModule->waveForms[wIx]->lastXYScaleValue = 1.0;

		// Y Controls:
		x += dx;
		inputPorts[multiScope::Y_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), module, multiScope::Y_INPUT + wIx, plugLightsEnabled, scopeModule->waveForms[wIx]->waveColor));
		addInput(inputPorts[multiScope::Y_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), module, multiScope::Y_POS_PARAM + wIx, TROWA_SCOPE_POS_KNOB_MIN, TROWA_SCOPE_POS_KNOB_MAX, TROWA_SCOPE_POS_Y_KNOB_DEF));
		// Keep reference to the scale knobs for synching
		scaleKnobs[wIx][KNOB_Y] = dynamic_cast<TS_TinyBlackKnob*>(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y3), module, multiScope::Y_SCALE_PARAM + wIx, TROWA_SCOPE_SCALE_KNOB_MIN, TROWA_SCOPE_SCALE_KNOB_MAX, 1.0));
		addParam(scaleKnobs[wIx][KNOB_Y]);

		// Color Controls:
		x += dx;
		inputPorts[multiScope::COLOR_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), module, multiScope::COLOR_INPUT + wIx, plugLightsEnabled, scopeModule->waveForms[wIx]->waveColor));
		addInput(inputPorts[multiScope::COLOR_INPUT + wIx]);
		float knobHueVal = rescale(scopeModule->waveForms[wIx]->waveHue, 0, 1.0, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2 + TROWA_SCOPE_COLOR_KNOB_Y_OFFSET), module, multiScope::COLOR_PARAM + wIx, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, knobHueVal));
		scopeModule->params[multiScope::COLOR_PARAM + wIx].value = knobHueVal;
#if TROWA_SCOPE_USE_COLOR_LIGHTS
		//scopeModule->waveForms[wIx]->waveLight = TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset, y3), module, multiScope::COLOR_LED + wIx, ledSize, scopeModule->waveForms[wIx]->waveColor, backColor);
		scopeModule->waveForms[wIx]->waveLight = TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), module, multiScope::COLOR_LED + wIx, tinyBtnSize, scopeModule->waveForms[wIx]->waveColor, backColor);
		addChild(scopeModule->waveForms[wIx]->waveLight);
#endif

		// Opacity:
		x += dx;
		inputPorts[multiScope::OPACITY_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), module, multiScope::OPACITY_INPUT + wIx, plugLightsEnabled, scopeModule->waveForms[wIx]->waveColor));
		addInput(inputPorts[multiScope::OPACITY_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), module, multiScope::OPACITY_PARAM + wIx, TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY, TROWA_SCOPE_MAX_OPACITY));
		// Pen On:
		inputPorts[multiScope::PEN_ON_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y3 - knobOffset), module, multiScope::PEN_ON_INPUT + wIx, plugLightsEnabled, scopeModule->waveForms[wIx]->waveColor));
		addInput(inputPorts[multiScope::PEN_ON_INPUT + wIx]);

		// Rotation Controls:
		x += dx;
		inputPorts[multiScope::ROTATION_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), module, multiScope::ROTATION_INPUT + wIx, plugLightsEnabled, scopeModule->waveForms[wIx]->waveColor));
		addInput(inputPorts[multiScope::ROTATION_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), module, multiScope::ROTATION_PARAM + wIx, TROWA_SCOPE_ROT_KNOB_MIN, TROWA_SCOPE_ROT_KNOB_MAX, 0));
		TS_PadSwitch* rotModeBtn = dynamic_cast<TS_PadSwitch*>( ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), module, multiScope::ROTATION_MODE_PARAM + wIx, 0, 1, 0) );
		rotModeBtn->box.size = tinyBtnSize;
		addParam(rotModeBtn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), module, multiScope::ROT_LED + wIx, tinyBtnSize, TROWA_SCOPE_ABS_ROT_ON_COLOR));
		if (scopeModule->waveForms[wIx]->rotMode)
		{
			rotModeBtn->value = 1.0;
			scopeModule->params[multiScope::ROTATION_MODE_PARAM + wIx].value = 1.0;
			scopeModule->waveForms[wIx]->rotModeTrigger.state = SchmittTrigger::HIGH;
			scopeModule->lights[multiScope::ROT_LED + wIx].value = 1.0;
		}

		// Time Controls:
		x += dx;
		inputPorts[multiScope::TIME_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), module, multiScope::TIME_INPUT + wIx, plugLightsEnabled, scopeModule->waveForms[wIx]->waveColor));
		addInput(inputPorts[multiScope::TIME_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), module, multiScope::TIME_PARAM + wIx, TROWA_SCOPE_TIME_KNOB_MIN, TROWA_SCOPE_TIME_KNOB_MAX, TROWA_SCOPE_TIME_KNOB_DEF));
		//TS_PadSwitch* mirrorBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset, y3), module, multiScope::MIRROR_X_PARAM + wIx, 0, 1, 0));
		//mirrorBtn->box.size = tinyBtnSize;
		//addParam(mirrorBtn);
		//mirrorBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset, y3 + tinyBtnSize.y + 2), module, multiScope::MIRROR_Y_PARAM + wIx, 0, 1, 0));
		//mirrorBtn->box.size = tinyBtnSize;
		//addParam(mirrorBtn);
		TS_PadSwitch* lissajousBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), module, multiScope::LISSAJOUS_PARAM + wIx, 0, 1, 1));
		lissajousBtn->box.size = tinyBtnSize;
		lissajousBtn->value = 1.0;
		addParam(lissajousBtn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), module, multiScope::LISSAJOUS_LED + wIx, tinyBtnSize, TROWA_SCOPE_LISSAJOUS_ON_COLOR));
		scopeModule->params[multiScope::LISSAJOUS_PARAM + wIx].value = 1.0;
		scopeModule->waveForms[wIx]->lissajousTrigger.state = SchmittTrigger::HIGH;
		scopeModule->lights[multiScope::LISSAJOUS_LED + wIx].value = 1.0;

		y = y3 + dy + shapeSpacingY; // Extra space between shapes / waveforms		
	} // end loop

	// Screws:
	addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(0, box.size.y - SCREW_DIAMETER)));

	rhsScrews[0] = dynamic_cast<ScrewBlack*>(Widget::create<ScrewBlack>(Vec(box.size.x - SCREW_DIAMETER, 0)));
	addChild(rhsScrews[0]);
	rhsScrews[1] = dynamic_cast<ScrewBlack*>(Widget::create<ScrewBlack>(Vec(box.size.x - SCREW_DIAMETER, box.size.y - SCREW_DIAMETER)));
	addChild(rhsScrews[1]);
	if (scopeModule) {
		scopeModule->firstLoad = true;
		scopeModule->initialized = true;
	}
	return;
} // end multiScopeWidget()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step(void)
// Resize.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiScopeWidget::step() {
	multiScope* scopeModule = dynamic_cast<multiScope*>(module);
	
	if (scopeModule->initialized)
	{
		if (scopeModule->infoDisplayOnTrigger.process(scopeModule->params[multiScope::INFO_DISPLAY_TOGGLE_PARAM].value)) {
			scopeInfoDisplay->visible = !scopeInfoDisplay->visible;
		}
		scopeModule->lights[multiScope::INFO_DISPLAY_TOGGLE_LED].value = (scopeInfoDisplay->visible) ? 1.0 : 0.0;

		// Resizing ///////////////////////////////
		float width = box.size.x - inputAreaWidth - 2  - this->rightHandle->box.size.x;
		screenBackground->box.size.x = box.size.x - inputAreaWidth + 1;
		if (width - 15 < scopeInfoDisplay->box.size.x)
		{
			scopeInfoDisplay->box.size.x = width - 15;
		}
		else if (width - 15 > scopeInfoDisplay->box.size.x)
		{
			scopeInfoDisplay->box.size.x = (width - 15 > scopeInfoDisplay->originalWidth) ? scopeInfoDisplay->originalWidth : width - 15;
		}
		//float height = box.size.y - TROWA_WIDGET_TOP_BAR_HEIGHT * 2;
		for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
		{
			display[wIx]->box.size.x = width;
			// Change light colors on plugs
			if (plugLightsEnabled)
			{
				//if (scopeModule->waveForms[wIx]->colorChanged) // When reloading from save, this doesn't work :(
				{
					inputPorts[multiScope::PEN_ON_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->waveColor);
					inputPorts[multiScope::OPACITY_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->waveColor);
					inputPorts[multiScope::COLOR_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->waveColor);
					inputPorts[multiScope::ROTATION_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->waveColor);
					inputPorts[multiScope::TIME_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->waveColor);
					inputPorts[multiScope::X_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->waveColor);
					inputPorts[multiScope::Y_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->waveColor);
				}
			}
			// Adjusting Knobs ///////////////////////////////////////////////
			// Link X, Y scales
			int srcIx = -1;
			if (scopeModule->waveForms[wIx]->linkXYScalesTrigger.process(scopeModule->params[multiScope::LINK_XY_SCALE_PARAM + wIx].value))
			{
				scopeModule->waveForms[wIx]->linkXYScales = !scopeModule->waveForms[wIx]->linkXYScales;
				if (scopeModule->waveForms[wIx]->linkXYScales)
				{
					// Initial set up - make Y match X
					srcIx = KNOB_X;
				}
			}
			else if (scopeModule->waveForms[wIx]->linkXYScales)
			{
				// Check if any one changed.
				if (scopeModule->params[multiScope::X_SCALE_PARAM + wIx].value != scopeModule->waveForms[wIx]->lastXYScaleValue)
					srcIx = KNOB_X;
				else if (scopeModule->params[multiScope::Y_SCALE_PARAM + wIx].value != scopeModule->waveForms[wIx]->lastXYScaleValue)
					srcIx = KNOB_Y;
			}
			if (srcIx > -1)
			{
				int destIx = (srcIx == KNOB_X) ? KNOB_Y : KNOB_X;
				// Adjust the other knob
				float val = scaleKnobs[wIx][srcIx]->value;
				//info("Changing SRC: %d; DEST: %d to value of %f.", srcIx, destIx, val);
				scaleKnobs[wIx][destIx]->value = val;
				scaleKnobs[wIx][destIx]->dirty = true; // Set to dirty.
				// Change the value on thhe module param too?
				scopeModule->params[multiScope::X_SCALE_PARAM + wIx].value = val;
				scopeModule->params[multiScope::Y_SCALE_PARAM + wIx].value = val;
				scopeModule->waveForms[wIx]->lastXYScaleValue = val;
			}
			scopeModule->lights[multiScope::LINK_XY_SCALE_LED + wIx].value = (scopeModule->waveForms[wIx]->linkXYScales) ? 1.0 : 0;
			// end adjust Knobs for XY Scale Link //////////////////////////
		}
		for (int i = 0; i < 2; i++)
		{
			rhsScrews[i]->box.pos.x = box.size.x - SCREW_DIAMETER; // subtract screw diameter
		}
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;		
		rightHandle->setChildPositions(); // Move the items that are artificially "in" this bar (really belong to multiScopeWidget, but should be rendered on top of rightHandle)
		ModuleWidget::step();
	}
	return;
} // end step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// toJson(void)
// Save to json.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
json_t *multiScopeWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();
	json_object_set_new(rootJ, "width", json_real(box.size.x));
	json_object_set_new(rootJ, "showInfoDisplay", json_integer(scopeInfoDisplay->visible));

	return rootJ;
} // end toJson()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// fromJson()
// Load from json object.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiScopeWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);
	json_t *widthJ = json_object_get(rootJ, "width");
	if (widthJ)
		box.size.x = json_number_value(widthJ);
	json_t* showInfoJ = json_object_get(rootJ, "showInfoDisplay");
	if (showInfoJ)
		scopeInfoDisplay->visible = (bool)json_integer_value(showInfoJ);
} // end fromJson()


#endif // use new scope