#include "dsp/digital.hpp"
#include "moDllz.hpp"
#include "dsp/filter.hpp"
/*
 * XBender
 */

namespace rack_plugin_moDllz {

struct XBender : Module {
	enum ParamIds {
		XBEND_PARAM,
		XBENDCVTRIM_PARAM,
		XBENDRANGE_PARAM,
		BEND_PARAM,
		BENDCVTRIM_PARAM,
		AXISXFADE_PARAM,
		AXISSLEW_PARAM,
		AXISTRNSUP_PARAM,
		AXISTRNSDWN_PARAM,
		AXISSHIFTTRIM_PARAM,
		AXISSELECT_PARAM = 10,
		AXISSELECTCV_PARAM = 18,
		SNAPAXIS_PARAM,
		YCENTER_PARAM,
		YZOOM_PARAM,
		AUTOZOOM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		XBENDCV_INPUT = 8,
		XBENDRANGE_INPUT,
		BENDCV_INPUT,
		AXISSELECT_INPUT,
		AXISEXT_INPUT,
		AXISXFADE_INPUT,
		AXISSHIFT_INPUT,
		AXISSLEW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		AXIS_OUTPUT = 8,
		NUM_OUTPUTS
	};
	enum LightIds {
		AXIS_LIGHT,
		AUTOZOOM_LIGHT = 8,
		SNAPAXIS_LIGHT,
		NUM_LIGHTS
	};
	
	float inAxis = 0.f;
	float axisSlew = 0.f;
	int axisTransParam = 0;
	float finalAxis = 0.f;
	float AxisShift =  0.f;
	float axisXfade;
	float selectedAxisF = 0.f;
	int selectedAxisI = 0;
	
	float dZoom = 1.f;
	float dCenter = 0.5f;
	int frameAutoZoom = 0;
	
	float slewchanged = 0.f;
	float XBenderKnobVal = 0.f;
	float XBenderKnobCV = 0.f;
	float xbend = 0.f;
	
	float testVal = 0.f;
	
	bool newZoom = false;
	
	SchmittTrigger axisTransUpTrigger;
	SchmittTrigger axisTransDwnTrigger;
	SchmittTrigger axisSelectTrigger[8];
	
	struct ioXBended {
		float inx = 0.0f;
		float xout = 0.0f;
		bool iactive = false;
	};
	
	ioXBended ioxbended[8];
	
	SlewLimiter slewlimiter;
	
	
	XBender() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	
	void step() override;
	void onReset() override {
		for (int ix = 0; ix < 8 ; ix++){
		outputs[OUT_OUTPUT + ix].value = inputs[IN_INPUT + ix].value;
		}
	}
	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "selectedAxisI", json_integer(selectedAxisI));
		json_object_set_new(rootJ, "axisTransParam", json_integer(axisTransParam));
		return rootJ;
	}
	
	void fromJson(json_t *rootJ) override {
		json_t *selectedAxisIJ = json_object_get(rootJ,("selectedAxisI"));
		selectedAxisI = json_integer_value(selectedAxisIJ);
		json_t *axisTransParamJ = json_object_get(rootJ,("axisTransParam"));
		axisTransParam = json_integer_value(axisTransParamJ);
	}
};

///////////////////////////////////////////
///////////////STEP //////////////////
/////////////////////////////////////////////

void XBender::step() {

	
	if(inputs[AXISSELECT_INPUT].active) {
		if (params[SNAPAXIS_PARAM].value > 0.5f){
			selectedAxisI = (clamp(static_cast<int>(inputs[AXISSELECT_INPUT].value * 0.7), 0, 7));
			selectedAxisF = static_cast<float>(selectedAxisI);
			inAxis = inputs[selectedAxisI].value;
		} else {
			selectedAxisF = clamp(inputs[AXISSELECT_INPUT].value * 0.7, 0.f, 7.f);
			selectedAxisI = static_cast<int>(selectedAxisF);
			float ax_float = selectedAxisF - static_cast<float>(selectedAxisI);
			inAxis = (inputs[selectedAxisI].value * (1.f - ax_float) + (inputs[selectedAxisI + 1].value * ax_float));
		}
	}else{
		 inAxis = inputs[selectedAxisI].value;
	}
	
	lights[SNAPAXIS_LIGHT].value = (params[SNAPAXIS_PARAM].value > 0.5f)? 1.f : 0.f;
	
	axisXfade = clamp((params[AXISXFADE_PARAM].value + inputs[AXISXFADE_INPUT].value/ 10.f), 0.f, 1.f);
	
	if (inputs[AXISEXT_INPUT].active){
		float axisext = inputs[AXISEXT_INPUT].value;
		axisSlew = crossfade(axisext, inAxis, axisXfade);
	}else axisSlew = inAxis;
	
	float slewsum = clamp((params[AXISSLEW_PARAM].value + inputs[AXISSLEW_INPUT].value * .1f), 0.f , 1.f);
	if (slewchanged != slewsum) {
		slewchanged = slewsum;
		float slewfloat = 1.0f/(5.0f + slewsum * engineGetSampleRate());
		slewlimiter.setRiseFall(slewfloat,slewfloat);
	}
	finalAxis = slewlimiter.process(axisSlew) + (inputs[AXISSHIFT_INPUT].value * params[AXISSHIFTTRIM_PARAM].value);
	AxisShift = (static_cast<float>(axisTransParam)/12.f);
	finalAxis += clamp(AxisShift, -12.f,12.f);
	
	float range = clamp((params[XBENDRANGE_PARAM].value + inputs[XBENDRANGE_INPUT].value/2.f), 1.f,5.f);
	/////////// MOTORIZED KNOB
	//float xbend = clamp((params[XBEND_PARAM].value + (inputs[XBENDCV_INPUT].value /5.f) * (params[XBENDCVTRIM_PARAM].value /24.f)),-1.f, 1.f);
	XBenderKnobCV = (params[XBENDCVTRIM_PARAM].value / 24.f) * (inputs[XBENDCV_INPUT].value / 5.f);
	
	XBenderKnobVal = params[XBEND_PARAM].value;
	
	xbend = clamp((XBenderKnobVal + XBenderKnobCV),-1.f,1.f);
	
	/////////////////////////
	float bend = clamp((params[BEND_PARAM].value + (inputs[BENDCV_INPUT].value /5.f) * (params[BENDCVTRIM_PARAM].value /60.f)),-1.f, 1.f);
	
	for (int i = 0; i < 8; i++){
		if (inputs[IN_INPUT + i].active) {
		
			if (axisSelectTrigger[i].process(params[AXISSELECT_PARAM + i].value)) {
				selectedAxisI = i;
				selectedAxisF = static_cast<float>(i); // float for display
			}
			ioxbended[i].iactive= true;
			lights[AXIS_LIGHT + i].value = (selectedAxisI == i)? 1.f : 0.01f;
			ioxbended[i].inx = inputs[IN_INPUT + i].value;
			float diff = (finalAxis - ioxbended[i].inx) * xbend * range;
			ioxbended[i].xout = clamp((ioxbended[i].inx + diff + bend * 6.f),-12.f,12.f);
			outputs[OUT_OUTPUT + i].value = ioxbended[i].xout;

		}else{
			lights[AXIS_LIGHT + i].value = 0;
			ioxbended[i].iactive=false;
		}
	} //for loop ix
  
	outputs[AXIS_OUTPUT].value = finalAxis;
	
	if (axisTransUpTrigger.process(params[AXISTRNSUP_PARAM].value))
			if (axisTransParam < 48) axisTransParam ++;
	if (axisTransDwnTrigger.process(params[AXISTRNSDWN_PARAM].value))
			if (axisTransParam > -48) axisTransParam --;
   
	bool autoZoom = (params[AUTOZOOM_PARAM].value > 0.f);
	if (autoZoom){
		frameAutoZoom ++;
		if (frameAutoZoom > 128) {
			frameAutoZoom = 0;
		float autoZoomMin = 12.f , autoZoomMax = -12.f;
			int active = 0;
		for (int i = 0; i < 8; i++){
			if (inputs[IN_INPUT + i].active) {
			active ++;
			if (ioxbended[i].inx < autoZoomMin)
				autoZoomMin = ioxbended[i].inx;
			if (ioxbended[i].xout < autoZoomMin)
				autoZoomMin = ioxbended[i].xout;
			if (ioxbended[i].inx > autoZoomMax)
				autoZoomMax = ioxbended[i].inx;
			if (ioxbended[i].xout > autoZoomMax)
				autoZoomMax = ioxbended[i].xout;
			}
		}
			if (finalAxis < autoZoomMin)
				autoZoomMin = finalAxis;
			if (finalAxis > autoZoomMax)
				autoZoomMax = finalAxis;
			float autoZ = 22.f / clamp((autoZoomMax - autoZoomMin),1.f,24.f);
			float autoCenter = 10.f * (autoZoomMin + (autoZoomMax - autoZoomMin) / 2.f);
			dZoom = clamp(autoZ, 1.f, 15.f);
			dCenter = clamp(autoCenter, -120.f, 120.f);
		}
		lights[AUTOZOOM_LIGHT].value = 10.f;
	}
	else {
		dCenter = params[XBender::YCENTER_PARAM].value;
		dZoom = params[XBender::YZOOM_PARAM].value;
		lights[AUTOZOOM_LIGHT].value = 0.f;
	}

	testVal = xbend;
}//closing STEP

///Bend Realtime Background Display
struct BenderDisplayBackG : TransparentWidget {
	BenderDisplayBackG() {
	}
	float *pyCenter ;
	float *pyZoom ;
	void draw(NVGcontext* vg)
	{
		const float dispHeight = 228.f;
		const float dispCenter = dispHeight / 2.f;
		float yZoom = *pyZoom;
		float yCenter =  *pyCenter * yZoom + dispCenter;
		float keyw = 10.f * yZoom /12.f;
		// crop drawing to display
		nvgScissor(vg, 0.f, 0.f, 152.f, dispHeight);
		nvgBeginPath(vg);
		nvgFillColor(vg, nvgRGB(0x2a, 0x2a, 0x2a));
		nvgRect(vg, 20.f, yCenter - 120.f * yZoom, 110.f, 20.f * yZoom);
		nvgRect(vg, 20.f, yCenter + 100.f * yZoom, 110.f, 20.f * yZoom);
		nvgFill(vg);
		
		nvgBeginPath(vg);
		if (yZoom > 2.5f) {
			nvgFillColor(vg, nvgRGB(0x0, 0x0, 0x0));
			nvgRect(vg, 0.f, 0.f, 20.f, dispHeight);
			nvgFill(vg);
			nvgBeginPath(vg);
			nvgFillColor(vg, nvgRGB(0x2f, 0x2f, 0x2f));
		} else {
			nvgFillColor(vg, nvgRGB(0x1a, 0x1a, 0x1a));
		}
		nvgRect(vg, 20.f, yCenter - 50.f * yZoom, 110.f, 100.f * yZoom );
		nvgFill(vg);
		for (int i = 0; i < 11; i++){
			if (yZoom < 2.5f){
			   // 1V lines
				nvgBeginPath(vg);
				nvgStrokeColor(vg, nvgRGB(0x2d,0x2d,0x2d));
				nvgMoveTo(vg, 20.f, yCenter - 10.f * yZoom * i);
				nvgLineTo(vg, 130.f,yCenter - 10.f * yZoom * i);
				nvgMoveTo(vg, 20.f, yCenter + 10.f * yZoom * i);
				nvgLineTo(vg, 130.f,yCenter + 10.f * yZoom * i);
				nvgStroke(vg);
			}else if (i < 5){
				// keyboard
				float keyPos = yCenter + 10.f * yZoom * i;
				// C's highlight
				nvgBeginPath(vg);
				nvgFillColor(vg, nvgRGB(0x44, 0x44, 0x44));
				nvgRect(vg, 20.f, yCenter - 10.f * yZoom * i - keyw * 0.5f, 110.f, keyw);
				nvgFill(vg);
				/// over center
				nvgBeginPath(vg);
				nvgFillColor(vg, nvgRGB(0x0, 0x0, 0x0));
				nvgRect(vg, 20.f, keyPos + keyw * 1.5f, 110.f, keyw);
				nvgRect(vg, 20.f, keyPos + keyw * 3.5f , 110.f, keyw);
				nvgRect(vg, 20.f, keyPos + keyw * 5.5f, 110.f, keyw);
				nvgRect(vg, 20.f, keyPos + keyw * 8.5f, 110.f, keyw);
				nvgRect(vg, 20.f, keyPos + keyw * 10.5f, 110.f, keyw);
				nvgFill(vg);
				/// C's highlight
				nvgBeginPath(vg);
				nvgFillColor(vg, nvgRGB(0x44, 0x44, 0x44));
				nvgRect(vg, 20.f, yCenter + 10.f * yZoom * i - keyw * 0.5f, 110.f, keyw);
				nvgFill(vg);
				/// under center
				keyPos = yCenter - 10.f * yZoom * i;
				nvgBeginPath(vg);
				nvgFillColor(vg, nvgRGB(0x0, 0x0, 0x0));
				nvgRect(vg, 20.f, keyPos - keyw * 1.5f, 110.f, keyw);
				nvgRect(vg, 20.f, keyPos - keyw * 3.5f , 110.f, keyw);
				nvgRect(vg, 20.f, keyPos - keyw * 6.5f, 110.f, keyw);
				nvgRect(vg, 20.f, keyPos - keyw * 8.5f, 110.f, keyw);
				nvgRect(vg, 20.f, keyPos - keyw * 10.5f, 110.f, keyw);
				nvgFill(vg);
			}
		}
		// center 0v...
		nvgBeginPath(vg);
		if (yZoom < 2.5f){
			nvgStrokeColor(vg,nvgRGBA(0x80, 0x00, 0x00 ,0x77));
			nvgStrokeWidth(vg,1.f);
			nvgMoveTo(vg, 20.f, yCenter);
			nvgLineTo(vg, 130.f, yCenter);
			nvgStroke(vg);
		 }//... center C
		 else {
			 nvgFillColor(vg, nvgRGBA(0x80, 0x00, 0x00 ,0x77));
			 nvgRect(vg, 20.f, yCenter - keyw * 0.5f, 110.f, keyw);
			 nvgFill(vg);
		}
	}
};

///Bend Realtime Display
struct BenderDisplay : TransparentWidget {
	BenderDisplay() {
	}
	XBender::ioXBended *ioxB;
	
	float *pAxis =  NULL;
	float *pyCenter ;
	float *pyZoom ;
	float *pAxisIx;
	float *pAxisXfade;
	void draw(NVGcontext* vg)
	{
		const float dispHeight = 228.f;
		const float dispCenter = dispHeight / 2.f;
		float yZoom = *pyZoom;
		float yCenter =  *pyCenter * yZoom + dispCenter;
		float AxisIx = *pAxisIx;
		float keyw = 10.f * yZoom /12.f;
		// crop drawing to display
		nvgScissor(vg, 0.f, 0.f, 152.f, dispHeight);
		float Axis = *pAxis;
		///// Bend Lines
		const float yfirst = 10.5f;
		const float ystep = 26.f;
		for (int i = 0; i < 8 ; i++){
			//nowactive =;
			if (ioxB[i].iactive){
				float yport = yfirst + i * ystep;
				float yi =  yZoom * ioxB[i].inx * -10.f + yCenter ;
				float yo =  yZoom * ioxB[i].xout * -10.f + yCenter ;
				nvgBeginPath(vg);
				nvgStrokeWidth(vg,1.f);
				nvgStrokeColor(vg,nvgRGBA(0xff, 0xff, 0xff,0x80));
				nvgMoveTo(vg, 0.f, yport);
				nvgLineTo(vg, 20.f, yi);
				nvgLineTo(vg, 130.f, yo);
				nvgLineTo(vg, 150.f, yport);
				nvgStroke(vg);
				if (yZoom > 2.5f){
					nvgBeginPath(vg);
					nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff,0x60));
					nvgRoundedRect(vg, 15.f, yi - keyw * 0.5f, 10.f, keyw, yZoom / 3.f);
					nvgFill(vg);
					nvgBeginPath(vg);
					nvgRoundedRect(vg, 125.f, yo - keyw * 0.5f, 10.f, keyw, yZoom / 3.f);
					nvgFill(vg);
				}
			}
		}
		/// Axis Line
		NVGcolor extColor = nvgRGBA(0xee, 0xee, 0x00, (1.f - *pAxisXfade) * 0xff);
		NVGcolor intColor = nvgRGBA(0xee, 0x00, 0x00, *pAxisXfade * 0xff);
		NVGcolor axisColor = nvgRGB(0xee, (1.f - *pAxisXfade) * 0xee, 0x00);
	
		Axis = yZoom * Axis * -10.f + yCenter;
		nvgStrokeWidth(vg,1.f);
	//ext
		nvgBeginPath(vg);
		nvgStrokeColor(vg,extColor);
		nvgMoveTo(vg, 0.f, 228.f);
		nvgLineTo(vg, 20.f, Axis);
		nvgStroke(vg);
	// int
		nvgBeginPath(vg);
		nvgStrokeColor(vg,intColor);
		nvgMoveTo(vg, 0.f, yfirst + AxisIx * ystep);
		nvgLineTo(vg, 20.f, Axis);
		nvgStroke(vg);
	// axis
		nvgBeginPath(vg);
		nvgStrokeColor(vg,axisColor);
		nvgMoveTo(vg, 20.f, Axis);
		nvgLineTo(vg, 130.f, Axis);
		nvgLineTo(vg, 150.f, 222.f);
		nvgStroke(vg);
	}
};

// Transp Display
struct AxisTranspDisplay : TransparentWidget {
	AxisTranspDisplay(){
		font = Font::load(FONT_FILE);
	}
	float mdfontSize = 11.f;
	std::string s;
	std::shared_ptr<Font> font;
	int *pAxisTransP = 0;
	int AxisTransP = 128;
	void draw(NVGcontext* vg) {
		if (AxisTransP != *pAxisTransP) {
			AxisTransP = *pAxisTransP;
			s = std::to_string(*pAxisTransP);
		}
			nvgFontSize(vg, mdfontSize);
			nvgFontFaceId(vg, font->handle);
			nvgTextAlign(vg, NVG_ALIGN_CENTER);
			nvgFillColor(vg, nvgRGB(0xFF,0xFF,0xFF));
			nvgTextBox(vg, 0.f, 14.0f,box.size.x, s.c_str(), NULL);
		}
};


struct RangeSelector: moDllzSmSelector{
	RangeSelector(){
		minAngle = -0.4*M_PI;
		maxAngle = 0.4*M_PI;
	}
};

struct xbendKnob : SVGKnob {
//	 XBender *module;
//	float *ptoKnobCV = 0;
//	float *pxbend = 0;
//
//	float toKnobCV = 0.f;
//	float KnobVal = 0.f;

	xbendKnob() {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/xbendKnob.svg")));
		shadow->opacity = 0.f;
	}
//	void step() {
//		if (KnobVal != *pxbend){
//			KnobVal = *pxbend;
//			toKnobCV = *ptoKnobCV;
//			value = KnobVal;
//			dirty = true;
//		}
//		if (dirty) {
//			float angle;
//			if (isfinite(minValue) && isfinite(maxValue)) {
//				angle = rescale(value, minValue, maxValue, minAngle, maxAngle);
//			}
//			else {
//				angle = rescale(value, -1.0, 1.0, minAngle, maxAngle);
//				angle = fmodf(angle, 2*M_PI);
//			}
//			tw->identity();
//			// Rotate SVG
//			Vec center = sw->box.getCenter();
//			tw->translate(center);
//			tw->rotate(angle);
//			tw->translate(center.neg());
//			// Redraw
//			module->XBenderKnobVal = value - toKnobCV;
//		}
//		FramebufferWidget::step();
//	}
//
//	void onChange(EventChange &e) {
//		dirty = true;
//		Knob::onChange(e);
//	}
//	void onMouseUp(EventMouseUp &e){
//
//	}
};

struct zTTrim : SVGKnob {
	zTTrim() {
		minAngle = 0;
		maxAngle = 1.75*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/zTTrim.svg")));
		shadow->opacity = 0.f;
	}
};

struct cTTrim : SVGKnob {
	cTTrim() {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
		snap = true;
		setSVG(SVG::load(assetPlugin(plugin, "res/cTTrim.svg")));
		shadow->opacity = 0.f;
	}
};

struct autoZoom : SVGSwitch, ToggleSwitch {
	autoZoom() {
		addFrame(SVG::load(assetPlugin(plugin, "res/autoButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/autoButton.svg")));
	}
};

struct snapAxisButton : SVGSwitch, ToggleSwitch {
  snapAxisButton() {
	  addFrame(SVG::load(assetPlugin(plugin, "res/snapButton.svg")));
	  addFrame(SVG::load(assetPlugin(plugin, "res/snapButton.svg")));
  }
};


struct XBenderWidget : ModuleWidget {
	
		XBenderWidget(XBender *module): ModuleWidget(module){
		setPanel(SVG::load(assetPlugin(plugin, "res/XBender.svg")));
		float xPos;
		float yPos;
		float xyStep = 26.f ;
		//Screws
		addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(0, 365)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 365)));
		
		xPos = 73.f;
		yPos = 22.f;
		{
			BenderDisplayBackG *benderDisplayBG = new BenderDisplayBackG();
			benderDisplayBG->box.pos = Vec(xPos, yPos);
			benderDisplayBG->box.size = {152.f, 228.f};
			benderDisplayBG->pyCenter = &(module->dCenter);
			benderDisplayBG->pyZoom = &(module->dZoom);
			addChild(benderDisplayBG);
		}
		
		{
			BenderDisplay *benderDisplay = new BenderDisplay();
			benderDisplay->box.pos = Vec(xPos, yPos);
			benderDisplay->box.size = {152.f, 228.f};
			benderDisplay->ioxB = &(module->ioxbended[0]);
			benderDisplay->pAxis = &(module->finalAxis);
			benderDisplay->pAxisIx = &(module->selectedAxisF);
			benderDisplay->pyCenter = &(module->dCenter);
			benderDisplay->pyZoom = &(module->dZoom);
			benderDisplay->pAxisXfade = &(module->axisXfade);
			addChild(benderDisplay);
		}
		
		xPos = 170.f;
		yPos = 252.f;
		/// View Center Zoom
		addParam(ParamWidget::create<cTTrim>(Vec(xPos,yPos), module, XBender::YCENTER_PARAM, -120.f, 120.f, 0.f));
		xPos = 203;
		addParam(ParamWidget::create<zTTrim>(Vec(xPos,yPos), module, XBender::YZOOM_PARAM, 1.f, 15.f, 1.f));
		yPos += 1.5f;
		xPos = 125.f;
		addParam(ParamWidget::create<autoZoom>(Vec(xPos, yPos ), module, XBender::AUTOZOOM_PARAM, 0.0f, 1.0f, 1.0f));
		addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(xPos + 17.f, yPos + 4.f), module, XBender::AUTOZOOM_LIGHT));

		/// IN  - Axis select  - OUTS
		xPos = 12.f;
		yPos = 22.f;
		for (int i = 0; i < 8; i++){
			// IN leds  OUT
			addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, XBender::IN_INPUT + i));
			addParam(ParamWidget::create<moDllzRoundButton>(Vec(xPos + 37.f, yPos + 5.f), module, XBender::AXISSELECT_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(xPos + 42.5f, yPos + 10.5f), module, XBender::AXIS_LIGHT + i));
			addOutput(Port::create<moDllzPort>(Vec(234.8f, yPos), Port::OUTPUT, module, XBender::OUT_OUTPUT + i));
			yPos += xyStep;
		}
		yPos = 248.f;
		//// AXIS out >>>> on the Right
		addOutput(Port::create<moDllzPort>(Vec(234.8f, yPos), Port::OUTPUT, module, XBender::AXIS_OUTPUT));
		
		yPos = 249.f;
		/// AXIS select in
		addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, XBender::AXISSELECT_INPUT));
		addParam(ParamWidget::create<snapAxisButton>(Vec(xPos + 22.5f, yPos + 2.f ), module, XBender::SNAPAXIS_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(xPos + 40.f, yPos + 4.f), module, XBender::SNAPAXIS_LIGHT));

		/// AXIS EXT - XFADE
		yPos += 25.f;
		addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, XBender::AXISXFADE_INPUT));
		addParam(ParamWidget::create<moDllzKnob26>(Vec(36.f,280.f), module, XBender::AXISXFADE_PARAM, 0.f, 1.f, 1.f));
		yPos += 25.f;
		addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, XBender::AXISEXT_INPUT));
			
		/// AXIS Slew
		yPos += 25.f;
		addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, XBender::AXISSLEW_INPUT));
		addParam(ParamWidget::create<moDllzKnob26>(Vec(34.f, 324.f), module, XBender::AXISSLEW_PARAM, 0.f, 1.f, 0.f));
 
		//AXIS Mod Shift
		addInput(Port::create<moDllzPort>(Vec(xPos + 53.f,yPos), Port::INPUT, module, XBender::AXISSHIFT_INPUT));
		addParam(ParamWidget::create<moDllzKnob26>(Vec(87.f, 324.f), module, XBender::AXISSHIFTTRIM_PARAM, 0.0f, 1.0f, 0.f));

		//AXIS Transp
		xPos = 73.5f;
		yPos = 278.5f;
		addParam(ParamWidget::create<moDllzPulseUp>(Vec(xPos + 31.f,yPos - 1.f), module, XBender::AXISTRNSUP_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<moDllzPulseDwn>(Vec(xPos + 31.f ,yPos + 10.f), module, XBender::AXISTRNSDWN_PARAM, 0.0f, 1.0f, 0.0f));
		{
			AxisTranspDisplay *axisTranspDisplay = new AxisTranspDisplay();
			axisTranspDisplay->box.pos = Vec(xPos, yPos);
			axisTranspDisplay->box.size = {30.f, 20};
			axisTranspDisplay->pAxisTransP = &(module->axisTransParam);
			addChild(axisTranspDisplay);
		}
		/// Knobs
		
		//XBEND
		xPos = 124.f;
		yPos = 272.f;
		
		addParam(ParamWidget::create<xbendKnob>(Vec(xPos, yPos), module, XBender::XBEND_PARAM, -1.f, 1.f, 0.f));
//			{   xbendKnob *xbendK = new xbendKnob();
//				xbendK->box.pos = Vec(xPos, yPos);
//				xbendK->box.size = {50.f, 50.f};
//				xbendK->minValue = -1.f;
//				xbendK->maxValue = 1.f;
//				xbendK->defaultValue = 0.f;
//				xbendK->module = module;
//				xbendK->ptoKnobCV = &(module->XBenderKnobCV);
//				xbendK->pxbend = &(module->xbend);
//				addChild(xbendK);
//			}
		xPos = 127.5f;
		yPos = 328.f;
		addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, XBender::XBENDCV_INPUT));
		addParam(ParamWidget::create<TTrimSnap>(Vec(xPos + 26.5f,yPos + 7.f), module, XBender::XBENDCVTRIM_PARAM, 0.f, 24.f, 24.f));
			
		xPos = 127.5f;
		yPos = 328.f;
		addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, XBender::XBENDCV_INPUT));
		addParam(ParamWidget::create<TTrimSnap>(Vec(xPos + 26.5f,yPos + 7.f), module, XBender::XBENDCVTRIM_PARAM, 0.f, 24.f, 24.f));
		//XBEND RANGE
		xPos = 181.f;
		yPos = 288.f;
		addParam(ParamWidget::create<RangeSelector>(Vec(xPos, yPos), module, XBender::XBENDRANGE_PARAM, 1.f, 5.f, 1.f));
		xPos = 187.5f;
		yPos = 328.f;
		addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, XBender::XBENDRANGE_INPUT));
		//BEND
		xPos = 219.f;
		yPos = 288.f;
		addParam(ParamWidget::create<moDllzKnobM>(Vec(xPos, yPos), module, XBender::BEND_PARAM, -1.f, 1.f, 0.f));
		xPos = 218.5f;
		yPos = 328.f;
		addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, XBender::BENDCV_INPUT));
		addParam(ParamWidget::create<TTrimSnap>(Vec(xPos + 26.5f,yPos + 7.f), module, XBender::BENDCVTRIM_PARAM, 0.f, 60.f, 12.f));
//		{
//			testDisplay *mDisplay = new testDisplay();
//			mDisplay->box.pos = Vec(0.0f, 360.0f);
//			mDisplay->box.size = {165.0f, 20.0f};
//			mDisplay->valP = &(module->testVal);
//			addChild(mDisplay);
//		}
			
	}
};

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.

} // namespace rack_plugin_moDllz

using namespace rack_plugin_moDllz;

RACK_PLUGIN_MODEL_INIT(moDllz, XBender) {
   Model *modelXBender = Model::create<XBender, XBenderWidget>("moDllz", "XBender", "Poly X Bender",MULTIPLE_TAG ,EFFECT_TAG);
   return modelXBender;
}
