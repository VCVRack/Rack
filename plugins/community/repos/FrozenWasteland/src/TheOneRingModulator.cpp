#include <string.h>
#include "FrozenWasteland.hpp"
#include "dsp/digital.hpp"

#define BUFFER_SIZE 512

namespace rack_plugin_FrozenWasteland {

struct TheOneRingModulator : Module {
	enum ParamIds {
		FORWARD_BIAS_PARAM,
		LINEAR_VOLTAGE_PARAM,
		SLOPE_PARAM,
		FORWARD_BIAS_ATTENUVERTER_PARAM,
		LINEAR_VOLTAGE_ATTENUVERTER_PARAM,
		SLOPE_ATTENUVERTER_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CARRIER_INPUT,
		SIGNAL_INPUT,
		FORWARD_BIAS_CV_INPUT,
		LINEAR_VOLTAGE_CV_INPUT,
		SLOPE_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		WET_OUTPUT,
		MIX_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT_1,
		BLINK_LIGHT_2,
		BLINK_LIGHT_3,
		BLINK_LIGHT_4,
		NUM_LIGHTS
	};


	float bufferX1[BUFFER_SIZE] = {};
	float bufferY1[BUFFER_SIZE] = {};
	float bufferX2[BUFFER_SIZE] = {};
	float bufferY2[BUFFER_SIZE] = {};
	int bufferIndex = 0;
	float frameIndex = 0;
	float deltaTime = powf(2.0, -8);

	float voltageBias = 0;
	float voltageLinear = 0.5;
	float h = 1; //Slope

	//SchmittTrigger resetTrigger;


	inline float diode_sim(float inVoltage )
	  {
	  	//Original
	  	//if( inVoltage < 0 ) return 0;
      	//	else return 0.2 * log( 1.0 + exp( 10 * ( inVoltage - 1 ) ) );
	  	
      	//Mine
	  	if( inVoltage <= voltageBias ) 
	  		return 0;
	    if( inVoltage <= voltageLinear) {
	    	return h * (inVoltage - voltageBias) * (inVoltage - voltageBias) / ((2.0 * voltageLinear) - (2.0 * voltageBias));
	    } else {
	    	return (h * inVoltage) - (h * voltageLinear) + (h * ((voltageLinear - voltageBias) * (voltageLinear - voltageBias) / ((2.0 * voltageLinear) - (2.0 * voltageBias))));
	    }	    
	  }

	TheOneRingModulator() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void TheOneRingModulator::step() {

	
    float vIn = inputs[ SIGNAL_INPUT ].value;
    float vC  = inputs[ CARRIER_INPUT ].value;
    float wd  = params[ MIX_PARAM ].value;

    voltageBias = clamp(params[FORWARD_BIAS_PARAM].value + (inputs[FORWARD_BIAS_CV_INPUT].value * params[FORWARD_BIAS_ATTENUVERTER_PARAM].value),0.0,10.0);
	voltageLinear = clamp(params[LINEAR_VOLTAGE_PARAM].value + (inputs[LINEAR_VOLTAGE_CV_INPUT].value * params[LINEAR_VOLTAGE_ATTENUVERTER_PARAM].value),voltageBias + 0.001f,10.0);
    h = clamp(params[SLOPE_PARAM].value + (inputs[SLOPE_CV_INPUT].value / 10.0 * params[SLOPE_ATTENUVERTER_PARAM].value),0.1f,1.0f);

    float A = 0.5 * vIn + vC;
    float B = vC - 0.5 * vIn;

    float dPA = diode_sim( A );
    float dMA = diode_sim( -A );
    float dPB = diode_sim( B );
    float dMB = diode_sim( -B );

    float res = dPA + dMA - dPB - dMB;
    //outputs[WET_OUTPUT].value = res;
    outputs[MIX_OUTPUT].value = wd * res + ( 1.0 - wd ) * vIn;
}





struct DiodeResponseDisplay : TransparentWidget {
	TheOneRingModulator *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	struct Stats {
		float vrms, vpp, vmin, vmax;
		void calculate(float *values) {
			vrms = 0.0;
			vmax = -INFINITY;
			vmin = INFINITY;
			for (int i = 0; i < BUFFER_SIZE; i++) {
				float v = values[i];
				vrms += v*v;
				vmax = fmaxf(vmax, v);
				vmin = fminf(vmin, v);
			}
			vrms = sqrtf(vrms / BUFFER_SIZE);
			vpp = vmax - vmin;
		}
	};
	Stats statsX, statsY;

	DiodeResponseDisplay() {
		font = Font::load(assetPlugin(plugin, "res/fonts/Sudo.ttf"));
	}

	void drawWaveform(NVGcontext *vg, float vB, float vL, float h) {
		nvgStrokeWidth(vg, 2);
		nvgBeginPath(vg);
		nvgMoveTo(vg, 10, 122);
		nvgLineTo(vg, 10 + vB/10.0*127, 122);
		//Draw response as a bunch of small lines for now until I can convert to bezier	
		for (float inX=vB+.1;inX<=vL;inX+=.1) {
			float nonLinearY = h * (inX - vB) * (inX - vB) / ((2.0 * vL) - (2.0 * vB));
			nvgLineTo(vg, 10 + inX/10*127.0,10+(1-nonLinearY/10.0)*112.0);
		}

		float voltLinearConstant = 0 - (h * vL) + (h * ((vL - vB) * (vL - vB) / ((2.0 * vL) - (2.0 * vB))));
		for (float inX=vL+.1;inX<=10;inX+=.1) {		
			float linearY = (h * inX) + voltLinearConstant;
			nvgLineTo(vg, 10 + inX/10*127.0,10+(1-linearY/10.0)*112.0);
		}
		//nvgLineTo(vg, 137, 12 + (1-h) * 122);
		nvgStroke(vg);
	}

	

	void drawStats(NVGcontext *vg, Vec pos, const char *title, Stats *stats) {
		nvgFontSize(vg, 13);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
		nvgText(vg, pos.x + 6, pos.y + 11, title, NULL);

		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		char text[128];
		snprintf(text, sizeof(text), "pp % 06.2f  max % 06.2f  min % 06.2f", stats->vpp, stats->vmax, stats->vmin);
		nvgText(vg, pos.x + 22, pos.y + 11, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0x20, 0xff));	
		drawWaveform(vg, module->voltageBias, module->voltageLinear, module->h);
	}
};

struct TheOneRingModulatorWidget : ModuleWidget {
	TheOneRingModulatorWidget(TheOneRingModulator *module);
};

TheOneRingModulatorWidget::TheOneRingModulatorWidget(TheOneRingModulator *module) : ModuleWidget(module) {
	box.size = Vec(15*10, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TheOneRingModulator.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH - 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH-12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH + 12, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	{
		DiodeResponseDisplay *display = new DiodeResponseDisplay();
		display->module = module;
		display->box.pos = Vec(0, 35);
		display->box.size = Vec(box.size.x-10, 90);
		addChild(display);
	}

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 190), module, TheOneRingModulator::FORWARD_BIAS_PARAM, 0.0, 10.0, 0.0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(12, 254), module, TheOneRingModulator::FORWARD_BIAS_ATTENUVERTER_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(60, 190), module, TheOneRingModulator::LINEAR_VOLTAGE_PARAM, 0.0, 10.0, 0.5));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(62, 254), module, TheOneRingModulator::LINEAR_VOLTAGE_ATTENUVERTER_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(110, 190), module, TheOneRingModulator::SLOPE_PARAM, 0.1, 1.0, 1.0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(112, 254), module, TheOneRingModulator::SLOPE_ATTENUVERTER_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(90, 325), module, TheOneRingModulator::MIX_PARAM, -0.0, 1.0, 0.5));

	addInput(Port::create<PJ301MPort>(Vec(14, 330), Port::INPUT, module, TheOneRingModulator::CARRIER_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(50, 330), Port::INPUT, module, TheOneRingModulator::SIGNAL_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(13, 225), Port::INPUT, module, TheOneRingModulator::FORWARD_BIAS_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(63, 225), Port::INPUT, module, TheOneRingModulator::LINEAR_VOLTAGE_CV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(113, 225), Port::INPUT, module, TheOneRingModulator::SLOPE_CV_INPUT));

	//addOutput(Port::create<PJ301MPort>(Vec(51, 330), Port::OUTPUT, module, TheOneRingModulator::WET_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(122, 330), Port::OUTPUT, module, TheOneRingModulator::MIX_OUTPUT));
	
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(21, 59), module, LissajousLFO::BLINK_LIGHT_1));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(41, 59), module, LissajousLFO::BLINK_LIGHT_2));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(61, 59), module, LissajousLFO::BLINK_LIGHT_3));
	//addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(81, 59), module, LissajousLFO::BLINK_LIGHT_4));
}

} // namespace rack_plugin_FrozenWasteland

using namespace rack_plugin_FrozenWasteland;

RACK_PLUGIN_MODEL_INIT(FrozenWasteland, TheOneRingModulator) {
   Model *modelTheOneRingModulator = Model::create<TheOneRingModulator, TheOneRingModulatorWidget>("Frozen Wasteland", "TheOneRingModulator", "The One Ring Modulator", RING_MODULATOR_TAG);
   return modelTheOneRingModulator;
}
