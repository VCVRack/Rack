#include "Bidoo.hpp"
#include "dsp/digital.hpp"
#include "BidooComponents.hpp"
#include <vector>
#include "cmath"

using namespace std;

namespace rack_plugin_Bidoo {

struct CHUTE : Module {
	enum ParamIds {
		ALTITUDE_PARAM,
		GRAVITY_PARAM,
		COR_PARAM,
		RUN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		ALTITUDE_INPUT,
		GRAVITY_INPUT,
		COR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		GATE_OUTPUT,
		PITCH_OUTPUT,
		PITCHSTEP_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	bool running = false;
	float phase = 0.0f;
	float altitude = 0.0f;
	float altitudeInit = 0.0f;
	float minAlt = 0.0f;
	float speed = 0.0f;
	bool desc = false;

	SchmittTrigger playTrigger;
	SchmittTrigger gateTypeTrigger;

	CHUTE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }

	void step() override;

};


void CHUTE::step() {

	// Running
	if (playTrigger.process(params[RUN_PARAM].value + inputs[TRIG_INPUT].value)) {
		running = true;
		desc = true;
		altitude = params[ALTITUDE_PARAM].value + inputs[ALTITUDE_INPUT].value;
		altitudeInit = altitude;
		minAlt = altitude;
		speed = 0.0f;
	}

	// Altitude calculation
	if (running) {
		if (minAlt<0.0001f) {
			running = false;
			altitude = 0.0f;
			minAlt = 0.0f;
		}
		else
		{
			phase = 1.0f / engineGetSampleRate();
			if (desc) {
				speed += (params[GRAVITY_PARAM].value + inputs[GRAVITY_INPUT].value)*phase;
				altitude = altitude - (speed * phase);
				if (altitude <= 0.0f) {
					desc=false;
					speed = speed * (params[COR_PARAM].value + + inputs[COR_INPUT].value);
					altitude = 0.0f;
				}
			}
			else {
				speed = speed - (params[GRAVITY_PARAM].value + inputs[GRAVITY_INPUT].value)*phase;
				if (speed<=0.0f) {
					speed = 0.0f;
					desc=true;
					minAlt=min(minAlt,altitude);
				}
				else {
					altitude = altitude + (speed * phase);
				}
			}
		}
	}

	//Calculate output
	outputs[GATE_OUTPUT].value = running ? desc ? 10.0f : 0.0f : 0.0f;
	outputs[PITCH_OUTPUT].value = running ? 10.0f * altitude/ altitudeInit : 0.0f;
	outputs[PITCHSTEP_OUTPUT].value = running ? 10.0f * minAlt/ altitudeInit : 0.0f;
}

struct CHUTEDisplay : TransparentWidget {
	CHUTE *module;
	int frame = 0;
	shared_ptr<Font> font;

	CHUTEDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		frame = 0;
		nvgFontSize(vg, 18.0f);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2.0f);
		nvgFillColor(vg, nvgRGBA(0x00, 0x00, 0x00, 0xff));

		float altRatio = clamp(module->altitude / module->altitudeInit, 0.0f, 1.0f);
		int pos = roundl(box.size.y + altRatio * (9.0f - box.size.y));

		nvgText(vg, 6.0f, pos, "☻", NULL);
	}
};

struct CHUTEWidget : ModuleWidget {
	CHUTEWidget(CHUTE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CHUTE.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			CHUTEDisplay *display = new CHUTEDisplay();
			display->module = module;
			display->box.pos = Vec(110.0f, 30.0f);
			display->box.size = Vec(40.0f, 180.0f);
			addChild(display);
		}

		static const float portX[2] = {20.0f, 60.0f};
		static const float portY[3] = {52.0f, 116.0f, 178.0f};
	 	addInput(Port::create<PJ301MPort>(Vec(portX[0], portY[0]),Port::INPUT, module, CHUTE::ALTITUDE_INPUT));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX[1]-1, portY[0]-2.0f), module, CHUTE::ALTITUDE_PARAM, 0.01f, 3.0f, 1.0f));
		addInput(Port::create<PJ301MPort>(Vec(portX[0], portY[1]),Port::INPUT, module, CHUTE::GRAVITY_INPUT));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX[1]-1, portY[1]-2.0f), module, CHUTE::GRAVITY_PARAM, 1.622f, 11.15f, 9.798f)); // between the Moon and Neptune
		addInput(Port::create<PJ301MPort>(Vec(portX[0], portY[2]),Port::INPUT, module, CHUTE::COR_INPUT));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX[1]-1, portY[2]-2.0f), module, CHUTE::COR_PARAM, 0.0f, 1.0f, 0.69f)); // 0 inelastic, 1 perfect elastic, 0.69 glass

		addParam(ParamWidget::create<BlueCKD6>(Vec(51.0f, 269.0f), module, CHUTE::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addInput(Port::create<PJ301MPort>(Vec(11.0f, 270.0f),Port::INPUT,  module, CHUTE::TRIG_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(11.0f, 320.0f),Port::OUTPUT, module, CHUTE::GATE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(54.0f, 320.0f),Port::OUTPUT, module, CHUTE::PITCH_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(96.0f, 320.0f),Port::OUTPUT, module, CHUTE::PITCHSTEP_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, CHUTE) {
   Model *modelCHUTE = Model::create<CHUTE, CHUTEWidget>("Bidoo", "ChUTE", "ChUTE trigger", SEQUENCER_TAG);
   return modelCHUTE;
}
