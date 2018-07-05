#include "Features.hpp"

#if USE_NEW_SCOPE

#include <string.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "dsp/digital.hpp"
#include "Module_multiScope.hpp"
#include "Widget_multiScope.hpp"

using namespace trowaSoft;

#define SCREW_DIAMETER						 15
#define TROWA_WIDGET_TOP_BAR_HEIGHT	 		 15
#define	TROWA_SCOPE_INPUT_AREA_WIDTH		365  // 295 265
#define TROWA_SCOPE_MIN_SCOPE_AREA_WIDTH	(240 - RACK_GRID_WIDTH) // 240-RACK_GRID_WIDTH
#define TROWA_SCOPE_DISPLAY_AREA_WIDTH		400 
#define KNOB_X	0
#define KNOB_Y	1
#define HUE_IX		0
#define SAT_IX		1
#define VAL_IX		2





//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScopeWidget()
// Instantiate a multiScope widget.
// @scopeModule : (IN) Pointer to the multiScope module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiScopeWidget::multiScopeWidget(multiScope *scopeModule) : ModuleWidget(scopeModule)
{
	bool isPreview = scopeModule == NULL;

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

	//// Create scopeModule:
	//multiScope *scopeModule = new multiScope();
	//setModule(scopeModule);	
	
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
		screenContainer = new TS_Panel();
		screenContainer->backgroundColor = COLOR_BLACK;
		screenContainer->originalBackgroundColor = COLOR_BLACK;
		screenContainer->box.pos = Vec(inputAreaWidth - 1, 0);
		screenContainer->box.size = Vec(box.size.x - inputAreaWidth + 1, box.size.y);
		dynamic_cast<TS_Panel*>(screenContainer)->setBorderWidth(/*top*/ borderWidth, /*right*/ 0, /*bottom*/ borderWidth, /*left*/ 0);
		dynamic_cast<TS_Panel*>(screenContainer)->borderColor = borderColor;

		TS_Panel* screenBg = new TS_Panel();		
		screenBg->backgroundColor = (isPreview) ? COLOR_BLACK : scopeModule->plotBackgroundColor;
		screenBg->originalBackgroundColor = (isPreview) ? COLOR_BLACK : screenBg->backgroundColor;
		screenBg->box.pos = Vec(0, RACK_GRID_WIDTH);
		screenBg->box.size = Vec(screenContainer->box.size.x - RACK_GRID_WIDTH, screenContainer->box.size.y - 2 * RACK_GRID_WIDTH);
		screenContainer->addChild(screenBg);
		addChild(screenContainer);
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
		TS_PadSwitch* displayToggleBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(20, 20), scopeModule, multiScope::INFO_DISPLAY_TOGGLE_PARAM, 0, 1, 1));
		displayToggleBtn->box.size = tinyBtnSize;
		displayToggleBtn->value = 1.0;
		ColorValueLight* displayLED = TS_createColorValueLight<ColorValueLight>(displayToggleBtn->box.pos, scopeModule, multiScope::INFO_DISPLAY_TOGGLE_LED, tinyBtnSize, TROWA_SCOPE_INFO_DISPLAY_ON_COLOR);

		// Background Color
		TS_PadSwitch* colorDisplayToggleBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(20, 65), scopeModule, multiScope::BGCOLOR_DISPLAY_PARAM, 0, 1, 1));
		colorDisplayToggleBtn->box.size = tinyBtnSize;
		colorDisplayToggleBtn->value = 1.0;
		ColorValueLight* colorDisplayLED = TS_createColorValueLight<ColorValueLight>(colorDisplayToggleBtn->box.pos, scopeModule, multiScope::BGCOLOR_DISPLAY_LED, tinyBtnSize, TROWA_SCOPE_INFO_DISPLAY_ON_COLOR);

		TSScopeModuleResizeHandle* rightHandle = new TSScopeModuleResizeHandle(minimumWidgetWidth, svgpanel, displayToggleBtn, displayLED);
		rightHandle->colorDisplayToggleBtn = colorDisplayToggleBtn;
		rightHandle->colorDisplayToggleLED = colorDisplayLED;
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
#if ENABLE_BG_COLOR_PICKER
		addParam(colorDisplayToggleBtn);
		addChild(colorDisplayLED);
#endif
		if (!isPreview)
		{
			scopeModule->lights[multiScope::INFO_DISPLAY_TOGGLE_LED].value = 1.0;
			scopeModule->infoDisplayOnTrigger.state = SchmittTrigger::HIGH;
		}
	}

	////////////////////////////////////
	// Scope Display 
	////////////////////////////////////		
	int wIx = 0;
	int scopeGraphWidth = box.size.x - inputAreaWidth - this->rightHandle->box.size.x;
	for (wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{
		//info("Adding scope area %d.", wIx);
		multiScopeDisplay* scopeArea = new multiScopeDisplay();
		scopeArea->wIx = wIx;
		scopeArea->module = scopeModule;
		scopeArea->box.pos = Vec(inputAreaWidth - 1, borderSize);
		scopeArea->box.size = Vec(scopeGraphWidth, scopeGraphHeight);
		addChild(scopeArea);
		this->display[wIx] = scopeArea;
	}

	// Single Controls:
	// Color sliders
	const int sliderHeight = 20;
	const int margin = 10;
	int yslider = this->display[0]->box.pos.y + this->display[0]->box.size.y - 3 * (sliderHeight + margin) - margin;

	//info("Adding color sliders");
	for (int i = 0; i < 3; i++)
	{
		this->colorSliders[i] = new TS_ColorSlider(Vec(160, sliderHeight));
		this->colorSliders[i]->startColorHSL.h = (i == 0) ? 0.0 : 0.5;
		this->colorSliders[i]->startColorHSL.s = (i == 1) ? 0.0 : 1.0;
		this->colorSliders[i]->startColorHSL.lum = (i == 2) ? 0.0 : 0.5;
		this->colorSliders[i]->endColorHSL.h = (i == 0) ? 1.0 : 0.5;
		this->colorSliders[i]->endColorHSL.s = (i == 1) ? 1.0 : 1.0;
		this->colorSliders[i]->endColorHSL.lum = (i == 2) ? 1.0 : 0.5;

		//this->colorSliders[i]->endColor = endColors[i];
		this->colorSliders[i]->visible = false;
		this->colorSliders[i]->box.pos = Vec(inputAreaWidth + 21, yslider);
		addChild(this->colorSliders[i]);
		yslider += sliderHeight + margin;
	}

	
	////////////////////////////////////
	// Control Display
	////////////////////////////////////
	{
		TSScopeDisplay* sDisplay = new TSScopeDisplay();
		//sDisplay->box.pos = Vec(13, controlDisplayY);;
		//sDisplay->box.size = Vec(inputAreaWidth - 13 * 2, controlDisplayHeight);
		sDisplay->box.pos = Vec(screenContainer->box.pos.x + 15, controlDisplayY);;
		sDisplay->box.size = Vec(scopeGraphWidth - 13 * 2, controlDisplayHeight);
		sDisplay->module = scopeModule;
		sDisplay->visible = true; // By default show display
		float minWidth = TROWA_SCOPE_DISPLAY_AREA_WIDTH - 13 * 2;
		sDisplay->originalWidth = (sDisplay->box.size.x > minWidth) ? sDisplay->box.size.x : minWidth;
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
		NVGcolor defaultColor = COLOR_RED;
		float defaultHue = 0.0;
		NVGcolor defaultFillColor = COLOR_BLUE;
		float defaultFillHue = 0.67;
		if (!isPreview)
		{
			defaultColor = scopeModule->waveForms[wIx]->waveColor;
			defaultHue = scopeModule->waveForms[wIx]->waveHue;
			defaultFillColor = scopeModule->waveForms[wIx]->fillColor;
			defaultFillHue = scopeModule->waveForms[wIx]->fillHue;
		}

		// X Controls:
		inputPorts[multiScope::X_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::X_INPUT + wIx, !plugLightsEnabled, defaultColor));
		addInput(inputPorts[multiScope::X_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), scopeModule, multiScope::X_POS_PARAM + wIx, TROWA_SCOPE_POS_KNOB_MIN, TROWA_SCOPE_POS_KNOB_MAX, TROWA_SCOPE_POS_X_KNOB_DEF));
		// Keep reference to the scale knobs for synching
		scaleKnobs[wIx][KNOB_X] = dynamic_cast<TS_TinyBlackKnob*>(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y3), scopeModule, multiScope::X_SCALE_PARAM + wIx, TROWA_SCOPE_SCALE_KNOB_MIN, TROWA_SCOPE_SCALE_KNOB_MAX, 1.0));
		addParam(scaleKnobs[wIx][KNOB_X]);

		// X-Y Scale Synchronization:
		TS_PadSwitch* linkXYScalesBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset + 23, y3 + tinyOffset), scopeModule, multiScope::LINK_XY_SCALE_PARAM + wIx, 0, 1, 0));
		linkXYScalesBtn->box.size = tinyBtnSize;
		addParam(linkXYScalesBtn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + 23, y3 + tinyOffset), scopeModule, multiScope::LINK_XY_SCALE_LED + wIx, tinyBtnSize, TROWA_SCOPE_LINK_XY_SCALE_ON_COLOR));
		if (!isPreview)
			scopeModule->waveForms[wIx]->lastXYScaleValue = 1.0;

		// Y Controls:
		x += dx;
		inputPorts[multiScope::Y_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::Y_INPUT + wIx, !plugLightsEnabled, defaultColor));
		addInput(inputPorts[multiScope::Y_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), scopeModule, multiScope::Y_POS_PARAM + wIx, TROWA_SCOPE_POS_KNOB_MIN, TROWA_SCOPE_POS_KNOB_MAX, TROWA_SCOPE_POS_Y_KNOB_DEF));
		// Keep reference to the scale knobs for synching
		scaleKnobs[wIx][KNOB_Y] = dynamic_cast<TS_TinyBlackKnob*>(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y3), scopeModule, multiScope::Y_SCALE_PARAM + wIx, TROWA_SCOPE_SCALE_KNOB_MIN, TROWA_SCOPE_SCALE_KNOB_MAX, 1.0));
		addParam(scaleKnobs[wIx][KNOB_Y]);

		// Color Controls:
		x += dx;
		inputPorts[multiScope::COLOR_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::COLOR_INPUT + wIx, !plugLightsEnabled, defaultColor));
		addInput(inputPorts[multiScope::COLOR_INPUT + wIx]);
		float knobHueVal = rescale(defaultHue, 0, 1.0, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2 + TROWA_SCOPE_COLOR_KNOB_Y_OFFSET), scopeModule, multiScope::COLOR_PARAM + wIx, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, knobHueVal));
		if (!isPreview)
			scopeModule->params[multiScope::COLOR_PARAM + wIx].value = knobHueVal;

#if TROWA_SCOPE_USE_COLOR_LIGHTS
		if (!isPreview)
		{
			scopeModule->waveForms[wIx]->waveLight = TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), scopeModule, multiScope::COLOR_LED + wIx, tinyBtnSize, scopeModule->waveForms[wIx]->waveColor, backColor);
			addChild(scopeModule->waveForms[wIx]->waveLight);
		}
#endif

		// Opacity:
		x += dx;
		inputPorts[multiScope::OPACITY_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::OPACITY_INPUT + wIx, !plugLightsEnabled, defaultColor));
		addInput(inputPorts[multiScope::OPACITY_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), scopeModule, multiScope::OPACITY_PARAM + wIx, TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY, TROWA_SCOPE_MAX_OPACITY));
		// Pen On:
		inputPorts[multiScope::PEN_ON_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y3 - knobOffset), scopeModule, multiScope::PEN_ON_INPUT + wIx, !plugLightsEnabled, defaultColor));
		addInput(inputPorts[multiScope::PEN_ON_INPUT + wIx]);

		// Fill Color Controls:
		x += dx;
		inputPorts[multiScope::FILL_COLOR_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::FILL_COLOR_INPUT + wIx, !plugLightsEnabled, defaultFillColor));
		addInput(inputPorts[multiScope::FILL_COLOR_INPUT + wIx]);
		knobHueVal = rescale(defaultFillHue, 0, 1.0, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2 + TROWA_SCOPE_COLOR_KNOB_Y_OFFSET), scopeModule, multiScope::FILL_COLOR_PARAM + wIx, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, knobHueVal));
		if (!isPreview)
			scopeModule->params[multiScope::FILL_COLOR_PARAM + wIx].value = knobHueVal;
		// Fill On toggle
		// close to knob: Vec(x + knobOffset + tinyOffset, y3 - 7)
		TS_PadSwitch* fillBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset + tinyOffset, y3 - 7), scopeModule, multiScope::FILL_ON_PARAM + wIx, 0, 1, 0));
		fillBtn->box.size = tinyBtnSize;
		addParam(fillBtn);
		this->fillColorLEDs[wIx] = TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + tinyOffset, y3 - 7), scopeModule, multiScope::FILL_ON_LED + wIx, tinyBtnSize, defaultFillColor);
		addChild(fillColorLEDs[wIx]);

		// Fill Opacity:
		x += dx;
		inputPorts[multiScope::FILL_OPACITY_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::FILL_OPACITY_INPUT + wIx, !plugLightsEnabled, defaultFillColor));
		addInput(inputPorts[multiScope::FILL_OPACITY_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), scopeModule, multiScope::FILL_OPACITY_PARAM + wIx, TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY, TROWA_SCOPE_MAX_OPACITY));


		// Rotation Controls:
		x += dx;
		inputPorts[multiScope::ROTATION_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::ROTATION_INPUT + wIx, !plugLightsEnabled, defaultColor));
		addInput(inputPorts[multiScope::ROTATION_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), scopeModule, multiScope::ROTATION_PARAM + wIx, TROWA_SCOPE_ROT_KNOB_MIN, TROWA_SCOPE_ROT_KNOB_MAX, 0));
		TS_PadSwitch* rotModeBtn = dynamic_cast<TS_PadSwitch*>( ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), scopeModule, multiScope::ROTATION_MODE_PARAM + wIx, 0, 1, 0) );
		rotModeBtn->box.size = tinyBtnSize;
		addParam(rotModeBtn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), scopeModule, multiScope::ROT_LED + wIx, tinyBtnSize, TROWA_SCOPE_ABS_ROT_ON_COLOR));
		if (!isPreview && scopeModule->waveForms[wIx]->rotMode)
		{
			rotModeBtn->value = 1.0;
			scopeModule->params[multiScope::ROTATION_MODE_PARAM + wIx].value = 1.0;
			scopeModule->waveForms[wIx]->rotModeTrigger.state = SchmittTrigger::HIGH;
			scopeModule->lights[multiScope::ROT_LED + wIx].value = 1.0;
		}

		// Time Controls:
		x += dx;
		inputPorts[multiScope::TIME_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::TIME_INPUT + wIx, !plugLightsEnabled, defaultColor));
		addInput(inputPorts[multiScope::TIME_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), scopeModule, multiScope::TIME_PARAM + wIx, TROWA_SCOPE_TIME_KNOB_MIN, TROWA_SCOPE_TIME_KNOB_MAX, TROWA_SCOPE_TIME_KNOB_DEF));
		TS_PadSwitch* lissajousBtn = dynamic_cast<TS_PadSwitch*>(ParamWidget::create<TS_PadSwitch>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), scopeModule, multiScope::LISSAJOUS_PARAM + wIx, 0, 1, 1));
		lissajousBtn->box.size = tinyBtnSize;
		lissajousBtn->value = 1.0;
		addParam(lissajousBtn);
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + knobOffset + tinyOffset, y3 + tinyOffset), scopeModule, multiScope::LISSAJOUS_LED + wIx, tinyBtnSize, TROWA_SCOPE_LISSAJOUS_ON_COLOR));
		if (!isPreview)
		{
			scopeModule->params[multiScope::LISSAJOUS_PARAM + wIx].value = 1.0;
			scopeModule->waveForms[wIx]->lissajousTrigger.state = SchmittTrigger::HIGH;
			scopeModule->lights[multiScope::LISSAJOUS_LED + wIx].value = 1.0;
		}

		// Thickness:
		x += dx;
		inputPorts[multiScope::THICKNESS_INPUT + wIx] = dynamic_cast<TS_Port*>(TS_createInput<TS_Port>(Vec(x, y), scopeModule, multiScope::THICKNESS_INPUT + wIx, !plugLightsEnabled, defaultColor));
		addInput(inputPorts[multiScope::THICKNESS_INPUT + wIx]);
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y2), scopeModule, multiScope::THICKNESS_PARAM + wIx, TROWA_SCOPE_THICKNESS_MIN, TROWA_SCOPE_THICKNESS_MAX, TROWA_SCOPE_THICKNESS_DEF));
		// Effect Controls:
		//x += dx;
		addParam(ParamWidget::create<TS_TinyBlackKnob>(Vec(x + knobOffset, y3), scopeModule, multiScope::EFFECT_PARAM + wIx, TROWA_SCOPE_EFFECT_KNOB_MIN, TROWA_SCOPE_EFFECT_KNOB_MAX, TROWA_SCOPE_EFFECT_KNOB_DEF));


		y = y3 + dy + shapeSpacingY; // Extra space between shapes / waveforms		
	} // end loop

	// Screws:
	addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(0, box.size.y - SCREW_DIAMETER)));

	rhsScrews[0] = dynamic_cast<ScrewBlack*>(Widget::create<ScrewBlack>(Vec(box.size.x - SCREW_DIAMETER, 0)));
	addChild(rhsScrews[0]);
	rhsScrews[1] = dynamic_cast<ScrewBlack*>(Widget::create<ScrewBlack>(Vec(box.size.x - SCREW_DIAMETER, box.size.y - SCREW_DIAMETER)));
	addChild(rhsScrews[1]);
	
	if (!isPreview)
	{
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

	static NVGcolor* lastEditColorPtr;
	
	if (scopeModule->initialized)
	{
		// Display toggle
		if (scopeModule->infoDisplayOnTrigger.process(scopeModule->params[multiScope::INFO_DISPLAY_TOGGLE_PARAM].value)) {
			scopeInfoDisplay->visible = !scopeInfoDisplay->visible;
		}
		scopeModule->lights[multiScope::INFO_DISPLAY_TOGGLE_LED].value = (scopeInfoDisplay->visible) ? 1.0 : 0.0;

#if ENABLE_BG_COLOR_PICKER
		// Color Picker (On-Screen)
		for (int i = 0; i < 3; i++)
		{
			colorSliders[i]->visible = scopeModule->showColorPicker;
		}
		if (scopeModule->showColorPicker)
		{
			if (lastEditColorPtr == NULL || lastEditColorPtr != scopeModule->editColorPointer)
			{
				if (scopeModule->editColorPointer != NULL)
				{
					TSColorHSL selectedColor;
					trowaSoft::TSColorToHSL(*(scopeModule->editColorPointer), &selectedColor);
					// Load new color:
					for (int i = 0; i < 3; i++)
					{
						colorSliders[i]->value = clamp(selectedColor.hsl[i], 0.0f, 1.0f); // first slider is for hue, second is for sat, last is for value
						for (int j = i + 1; j < 3; j++)
						{
							colorSliders[j]->setComponent(i, selectedColor.hsl[i]);
						}
					}
				}
			} // end if
			else
			{
				// Read in color
				TSColorHSL selectedColor;
				for (int i = 0; i < 3; i++)
				{
					selectedColor.hsl[i] = colorSliders[i]->value;
					for (int j = i + 1; j < 3; j++)
					{
						colorSliders[j]->setComponent(i, selectedColor.hsl[i]);
					}
				}
				// Set our color
				if (scopeModule->editColorPointer != NULL)
				{
					*(scopeModule->editColorPointer) = nvgHSL(selectedColor.h, selectedColor.s, selectedColor.lum);
				}
			} // end else
			lastEditColorPtr = scopeModule->editColorPointer;
		} // end if we're showing the color picker
#endif


		// Resizing ///////////////////////////////
		float width = box.size.x - inputAreaWidth  - this->rightHandle->box.size.x;
		screenContainer->box.size.x = box.size.x - inputAreaWidth;
		TS_Panel* screenBg = screenContainer->getFirstDescendantOfType<TS_Panel>();
		if (screenBg)
		{
			screenBg->box.size.x = screenContainer->box.size.x - RACK_GRID_WIDTH;

			// Set background color of plot area:
			// Negative Image (Invert Color)
			if (scopeModule->negativeImage)
			{
				screenBg->backgroundColor = ColorInvertToNegative(scopeModule->plotBackgroundColor);
			}
			else
			{
				screenBg->backgroundColor = scopeModule->plotBackgroundColor;
			}
		}


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
					inputPorts[multiScope::THICKNESS_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->waveColor);

					inputPorts[multiScope::FILL_COLOR_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->fillColor);
					inputPorts[multiScope::FILL_OPACITY_INPUT + wIx]->setLightColor(scopeModule->waveForms[wIx]->fillColor);
					if (scopeModule->waveForms[wIx]->doFill)
					{
						fillColorLEDs[wIx]->setColor(scopeModule->waveForms[wIx]->fillColor);
					}
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
