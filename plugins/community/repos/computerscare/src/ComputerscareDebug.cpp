#include "Computerscare.hpp"
#include "dsp/digital.hpp"
#include "dsp/filter.hpp"

#include <string>
#include <sstream>
#include <iomanip>

namespace rack_plugin_computerscare {

#define NUM_LINES 16

struct ComputerscareDebug : Module {
	enum ParamIds {
		PITCH_PARAM,
		MANUAL_TRIGGER,
		MANUAL_CLEAR_TRIGGER,
		NUM_PARAMS
	};
	enum InputIds {
		VAL_INPUT,
		TRG_INPUT,
		CLR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SINE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	std::string defaultStrValue = "0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n";
	std::string strValue = "0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n0.000000\n";

	float logLines[NUM_LINES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	int lineCounter = 0;

	SchmittTrigger clockTrigger;
	SchmittTrigger clearTrigger;
	SchmittTrigger manualClockTrigger;
  	SchmittTrigger manualClearTrigger;

	ComputerscareDebug() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void ComputerscareDebug::step() {
	std::string thisVal;
	if (clockTrigger.process(inputs[TRG_INPUT].value / 2.f) || manualClockTrigger.process(params[MANUAL_TRIGGER].value)) {
		 for( unsigned int a = NUM_LINES-1; a > 0; a = a - 1 )
		 {
		 	logLines[a] = logLines[a-1];
		 }
		logLines[0] = inputs[VAL_INPUT].value;

		thisVal = std::to_string(logLines[0]).substr(0,10);
		for( unsigned int a = 1; a < NUM_LINES; a = a + 1 )
		 {
		 	thisVal =  thisVal + "\n" + std::to_string(logLines[a]).substr(0,10);
		 }

		strValue = thisVal;
	}
	if(clearTrigger.process(inputs[CLR_INPUT].value / 2.f) || manualClearTrigger.process(params[MANUAL_CLEAR_TRIGGER].value)) {
		for( unsigned int a = 0; a < NUM_LINES; a++ )
		 {
		 	logLines[a] = 0;
		 }
		strValue = defaultStrValue;
	}


}

////////////////////////////////////
struct StringDisplayWidget3 : TransparentWidget {

  std::string *value;
  std::shared_ptr<Font> font;

  StringDisplayWidget3() {
    font = Font::load(assetPlugin(plugin, "res/Oswald-Regular.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
    // Background
    NVGcolor backgroundColor = nvgRGB(0x10, 0x00, 0x10);
    NVGcolor StrokeColor = nvgRGB(0xC0, 0xC7, 0xDE);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, -1.0, -1.0, box.size.x+2, box.size.y+2, 4.0);
    nvgFillColor(vg, StrokeColor);
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);    
    
    // text 
    nvgFontSize(vg, 15);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;   
    to_display << std::setw(8) << *value;

    Vec textPos = Vec(6.0f, 12.0f);   
    NVGcolor textColor = nvgRGB(0xC0, 0xE7, 0xDE);
    nvgFillColor(vg, textColor);
 	nvgTextBox(vg, textPos.x, textPos.y,80,to_display.str().c_str(), NULL);

  }
};


struct ComputerscareDebugWidget : ModuleWidget {

	ComputerscareDebugWidget(ComputerscareDebug *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/ComputerscareDebugPanel.svg")));

		addInput(Port::create<InPort>(Vec(3, 330), Port::INPUT, module, ComputerscareDebug::TRG_INPUT));
		addInput(Port::create<InPort>(Vec(33, 330), Port::INPUT, module, ComputerscareDebug::VAL_INPUT));
		addInput(Port::create<InPort>(Vec(63, 330), Port::INPUT, module, ComputerscareDebug::CLR_INPUT));
	
		addParam(ParamWidget::create<LEDButton>(Vec(6, 290), module, ComputerscareDebug::MANUAL_TRIGGER, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<LEDButton>(Vec(66, 290), module, ComputerscareDebug::MANUAL_CLEAR_TRIGGER, 0.0, 1.0, 0.0));

		StringDisplayWidget3 *display = new StringDisplayWidget3();
		  display->box.pos = Vec(1,24);
		  display->box.size = Vec(88, 250);
		  display->value = &module->strValue;
		  addChild(display);

	}
};

} // namespace rack_plugin_computerscare

using namespace rack_plugin_computerscare;

RACK_PLUGIN_MODEL_INIT(computerscare, ComputerscareDebug) {
   Model *modelComputerscareDebug = Model::create<ComputerscareDebug, ComputerscareDebugWidget>("computerscare", "computerscare-debug", "Debug", UTILITY_TAG);
   return modelComputerscareDebug;
}
