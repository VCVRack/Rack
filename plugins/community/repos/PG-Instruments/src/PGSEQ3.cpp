#include "dsp/digital.hpp"
#include "Template.hpp"

namespace rack_plugin_PG_Instruments {

#define NUM_SEQ_STEPS 16

struct PGSEQ3 : Module 
{
	enum ParamIds 
    {
		CLOCK_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		STEPS_PARAM,
		ENUMS(ROW1_PARAM, NUM_SEQ_STEPS),
		ENUMS(ROW2_PARAM, NUM_SEQ_STEPS),
		ENUMS(ROW3_PARAM, NUM_SEQ_STEPS),
		ENUMS(GATE_PARAM, NUM_SEQ_STEPS),
		NUM_PARAMS
	};
	
    enum InputIds 
    {
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		NUM_INPUTS
	};

	enum OutputIds 
    {
		GATES_OUTPUT,
		ROW1_OUTPUT,
		ROW2_OUTPUT,
		ROW3_OUTPUT,
		ENUMS(GATE_OUTPUT, NUM_SEQ_STEPS),
		NUM_OUTPUTS
	};

	enum LightIds 
    {
		RUNNING_LIGHT,
		RESET_LIGHT,
		GATES_LIGHT,
		ENUMS(ROW_LIGHTS, 3),
		ENUMS(GATE_LIGHTS, NUM_SEQ_STEPS),
		NUM_LIGHTS
	};

	bool running = true;
	SchmittTrigger clockTrigger;
	SchmittTrigger runningTrigger;
	SchmittTrigger resetTrigger;
	SchmittTrigger gateTriggers[NUM_SEQ_STEPS];
	/** Phase of internal LFO */
	float phase = 0.f;
	int index = 0;
	bool gates[NUM_SEQ_STEPS] = {};

	PGSEQ3() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) 
    {
		onReset();
	}

	void onReset() override 
    {
		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
			gates[i] = true;
		}
	}

	void onRandomize() override 
    {
		for (int i = 0; i < NUM_SEQ_STEPS; i++)
        {
			gates[i] = (randomUniform() > 0.5f);
		}
	}

	json_t *toJson() override 
    {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// gates
		json_t *gatesJ = json_array();
		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
			json_array_insert_new(gatesJ, i, json_integer((int) gates[i]));
		}
		json_object_set_new(rootJ, "gates", gatesJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override 
    {
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// gates
		json_t *gatesJ = json_object_get(rootJ, "gates");
		if (gatesJ) 
        {
			for (int i = 0; i < NUM_SEQ_STEPS; i++) 
            {
				json_t *gateJ = json_array_get(gatesJ, i);
				if (gateJ)
					gates[i] = !!json_integer_value(gateJ);
			}
		}
	}

	void setIndex(int index) 
    {
		int numSteps = (int) clamp(roundf(params[STEPS_PARAM].value + inputs[STEPS_INPUT].value), 1.0f, (float)NUM_SEQ_STEPS);
		phase = 0.f;
		this->index = index;
		if (this->index >= numSteps)
			this->index = 0;
	}

	void step() override 
    {
		// Run
		if (runningTrigger.process(params[RUN_PARAM].value))
        {
			running = !running;
		}

		bool gateIn = false;
		if (running) 
        {
			if (inputs[EXT_CLOCK_INPUT].active) 
            {
				// External clock
				if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value)) 
                {
					setIndex(index + 1);
				}
				gateIn = clockTrigger.isHigh();
			}
			else
            {
				// Internal clock
				float clockTime = powf(2.0f, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
				phase += clockTime * engineGetSampleTime();
				if (phase >= 1.0f) 
                {
					setIndex(index + 1);
				}
				gateIn = (phase < 0.5f);
			}
		}

		// Reset
		if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value)) 
        {
			setIndex(0);
		}

		// Gate buttons
		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
			if (gateTriggers[i].process(params[GATE_PARAM + i].value)) 
            {
				gates[i] = !gates[i];
			}
			outputs[GATE_OUTPUT + i].value = (running && gateIn && i == index && gates[i]) ? 10.0f : 0.0f;
			lights[GATE_LIGHTS + i].setBrightnessSmooth((gateIn && i == index) ? (gates[i] ? 1.f : 0.33) : (gates[i] ? 0.66 : 0.0));
		}

		// Outputs
		outputs[ROW1_OUTPUT].value = params[ROW1_PARAM + index].value;
		outputs[ROW2_OUTPUT].value = params[ROW2_PARAM + index].value;
		outputs[ROW3_OUTPUT].value = params[ROW3_PARAM + index].value;
		outputs[GATES_OUTPUT].value = (gateIn && gates[index]) ? 10.0f : 0.0f;
		lights[RUNNING_LIGHT].value = (running);
		lights[RESET_LIGHT].setBrightnessSmooth(resetTrigger.isHigh());
		lights[GATES_LIGHT].setBrightnessSmooth(gateIn);
		lights[ROW_LIGHTS].value = outputs[ROW1_OUTPUT].value / 10.0f;
		lights[ROW_LIGHTS + 1].value = outputs[ROW2_OUTPUT].value / 10.0f;
		lights[ROW_LIGHTS + 2].value = outputs[ROW3_OUTPUT].value / 10.0f;
	}
};


struct PGSEQ3Widget : ModuleWidget 
{
	PGSEQ3Widget(PGSEQ3 *module) : ModuleWidget(module) 
    {
		setPanel(SVG::load(assetPlugin(plugin, "res/PGSEQ3.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

		addParam(ParamWidget::create<RoundBlackKnob>(Vec(18, 56), module, PGSEQ3::CLOCK_PARAM, -2.0f, 6.0f, 2.0f));
		addParam(ParamWidget::create<LEDButton>(Vec(60, 61-1), module, PGSEQ3::RUN_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(64.4f, 64.4f), module, PGSEQ3::RUNNING_LIGHT));
		addParam(ParamWidget::create<LEDButton>(Vec(99, 61-1), module, PGSEQ3::RESET_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(103.4f, 64.4f), module, PGSEQ3::RESET_LIGHT));
		addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(132, 56), module, PGSEQ3::STEPS_PARAM, 1.0f, (float)NUM_SEQ_STEPS, (float)NUM_SEQ_STEPS));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(179.4f, 64.4f), module, PGSEQ3::GATES_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(218.4f, 64.4f), module, PGSEQ3::ROW_LIGHTS));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(256.4f, 64.4f), module, PGSEQ3::ROW_LIGHTS + 1));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(295.4f, 64.4f), module, PGSEQ3::ROW_LIGHTS + 2));

		static const float portX[NUM_SEQ_STEPS] = 
        {
            20, 58, 96, 135, 
            173, 212, 250, 288,
            327, 366, 404, 442,
            480, 519, 557, 596
        };
        
		addInput(Port::create<PJ301MPort>(Vec(portX[0]-1, 98), Port::INPUT, module, PGSEQ3::CLOCK_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX[1]-1, 98), Port::INPUT, module, PGSEQ3::EXT_CLOCK_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX[2]-1, 98), Port::INPUT, module, PGSEQ3::RESET_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX[3]-1, 98), Port::INPUT, module, PGSEQ3::STEPS_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(portX[4]-1, 98), Port::OUTPUT, module, PGSEQ3::GATES_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(portX[5]-1, 98), Port::OUTPUT, module, PGSEQ3::ROW1_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(portX[6]-1, 98), Port::OUTPUT, module, PGSEQ3::ROW2_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(portX[7]-1, 98), Port::OUTPUT, module, PGSEQ3::ROW3_OUTPUT));

		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
			addParam(ParamWidget::create<RoundBlackKnob>(Vec(portX[i]-2, 157), module, PGSEQ3::ROW1_PARAM + i, 0.0f, 10.0f, 1.0f));
			addParam(ParamWidget::create<RoundBlackKnob>(Vec(portX[i]-2, 198), module, PGSEQ3::ROW2_PARAM + i, 0.0f, 10.0f, 0.0f));
			addParam(ParamWidget::create<RoundBlackKnob>(Vec(portX[i]-2, 240), module, PGSEQ3::ROW3_PARAM + i, 0.0f, 10.0f, 0.0f));
			addParam(ParamWidget::create<LEDButton>(Vec(portX[i]+2, 278-1), module, PGSEQ3::GATE_PARAM + i, 0.0f, 1.0f, 0.0f));
			addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(portX[i]+6.4f, 281.4f), module, PGSEQ3::GATE_LIGHTS + i));
			addOutput(Port::create<PJ301MPort>(Vec(portX[i]-1, 307), Port::OUTPUT, module, PGSEQ3::GATE_OUTPUT + i));
		}
	}
};

} // namespace rack_plugin_PG_Instruments

using namespace rack_plugin_PG_Instruments;

RACK_PLUGIN_MODEL_INIT(PG_Instruments, PGSEQ3) {
   Model *modelPGSEQ3 = Model::create<PGSEQ3, PGSEQ3Widget>("PG-Instruments", "PGSEQ3", "PG-SEQ-3", SEQUENCER_TAG);
   return modelPGSEQ3;
}
