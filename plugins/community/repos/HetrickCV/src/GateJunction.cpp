#include "HetrickCV.hpp"

namespace rack_plugin_HetrickCV {

#ifdef USE_VST2
#define plugin "HetrickCV"
#endif // USE_VST2

struct GateJunction : Module
{
	enum ParamIds
	{
        MUTE1_PARAM,
        MUTE2_PARAM,
        MUTE3_PARAM,
        MUTE4_PARAM,
        MUTE5_PARAM,
        MUTE6_PARAM,
        MUTE7_PARAM,
        MUTE8_PARAM,

        INV1_PARAM,
        INV2_PARAM,
        INV3_PARAM,
        INV4_PARAM,
        INV5_PARAM,
        INV6_PARAM,
        INV7_PARAM,
        INV8_PARAM,

		NUM_PARAMS
	};
	enum InputIds
	{
        IN1_INPUT,
        IN2_INPUT,
        IN3_INPUT,
        IN4_INPUT,
        IN5_INPUT,
        IN6_INPUT,
        IN7_INPUT,
        IN8_INPUT,

		NUM_INPUTS
	};
	enum OutputIds
	{
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        OUT3_OUTPUT,
        OUT4_OUTPUT,
        OUT5_OUTPUT,
        OUT6_OUTPUT,
        OUT7_OUTPUT,
        OUT8_OUTPUT,
		NUM_OUTPUTS
    };
    enum LightIds
	{
        MUTE1_LIGHT,
        MUTE2_LIGHT,
        MUTE3_LIGHT,
        MUTE4_LIGHT,
        MUTE5_LIGHT,
        MUTE6_LIGHT,
        MUTE7_LIGHT,
        MUTE8_LIGHT,

        INV1_LIGHT,
        INV2_LIGHT,
        INV3_LIGHT,
        INV4_LIGHT,
        INV5_LIGHT,
        INV6_LIGHT,
        INV7_LIGHT,
        INV8_LIGHT,

        OUT1_LIGHT,
        OUT2_LIGHT,
        OUT3_LIGHT,
        OUT4_LIGHT,
        OUT5_LIGHT,
        OUT6_LIGHT,
        OUT7_LIGHT,
        OUT8_LIGHT,

		NUM_LIGHTS
	};

    float ins[8] = {};
    float outs[8] = {};

    bool muteState[8] = {};
    SchmittTrigger muteTrigger[8];

    bool invState[8] = {};
	SchmittTrigger invTrigger[8];

	GateJunction() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		reset();
	}

    void step() override;

    void reset() override
    {
        for (int i = 0; i < 8; i++)
        {
            muteState[i] = false;
            invState[i] = false;
		}
	}
    void randomize() override
    {
        for (int i = 0; i < 8; i++)
        {
            muteState[i] = (randomf() < 0.5);
            invState[i] = (randomf() < 0.5);
		}
    }

    json_t *toJson() override
    {
		json_t *rootJ = json_object();
		// states
        json_t *muteStatesJ = json_array();
        json_t *invStatesJ = json_array();
        for (int i = 0; i < 8; i++)
        {
			json_t *muteStateJ = json_boolean(muteState[i]);
            json_array_append_new(muteStatesJ, muteStateJ);
            json_t *invStateJ = json_boolean(invState[i]);
			json_array_append_new(invStatesJ, invStateJ);
		}
        json_object_set_new(rootJ, "muteStates", muteStatesJ);
        json_object_set_new(rootJ, "invStates", invStatesJ);
		return rootJ;
	}
    void fromJson(json_t *rootJ) override
    {
		// states
        json_t *muteStatesJ = json_object_get(rootJ, "muteStates");
        json_t *invStatesJ = json_object_get(rootJ, "invStates");
        if (muteStatesJ)
        {
            for (int i = 0; i < 8; i++)
            {
				json_t *stateJ = json_array_get(muteStatesJ, i);
				if (stateJ)
					muteState[i] = json_boolean_value(stateJ);
			}
        }
        if (invStatesJ)
        {
            for (int i = 0; i < 8; i++)
            {
				json_t *stateJ = json_array_get(invStatesJ, i);
				if (stateJ)
					invState[i] = json_boolean_value(stateJ);
			}
		}
	}

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void GateJunction::step()
{
    ins[0] = (inputs[IN1_INPUT].value >= 1.0f) ? 5.0f : 0.0f;

    for(int i = 1; i < 8; i++)
    {
        const int thisInput = IN1_INPUT + i;
        if(inputs[thisInput].active)
        {
            ins[i] = (inputs[thisInput].value >= 1.0f) ? 5.0f : 0.0f;
        }
        else
        {
            ins[i] = ins[thisInput - 1];
        }
    }

    for(int i = 0; i < 8; i++)
    {
        if (muteTrigger[i].process(params[MUTE1_PARAM + i].value)) muteState[i] ^= true;
        if (invTrigger[i].process(params[INV1_PARAM + i].value)) invState[i] ^= true;

        if(invState[i]) ins[i] = 5.0f - ins[i];
        if(muteState[i]) ins[i] = 0.0f;

        outputs[OUT1_OUTPUT + i].value = ins[i];
        lights[OUT1_LIGHT + i].value = ins[i];

        lights[MUTE1_LIGHT + i].setBrightness(muteState[i] ? 0.9 : 0.0);
        lights[INV1_LIGHT + i].setBrightness(invState[i] ? 0.9 : 0.0);
    }
}

template <typename BASE>
struct MuteLight : BASE {
	MuteLight() {
		this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

struct GateJunctionWidget : ModuleWidget { GateJunctionWidget(GateJunction *module); };

GateJunctionWidget::GateJunctionWidget(GateJunction *module) : ModuleWidget(module)
{
	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		auto *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/GateJunction.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    //////PARAMS//////

    const int inXPos = 10;
    const int outXPos = 145;
    //const int inLightX = 50;
    const int outLightX = 120;
    for(int i = 0; i < 8; i++)
    {
        const int yPos = 50 + (40 * i);
        const int lightY = 59 + (40 * i);

        //////INPUTS//////
        addInput(Port::create<PJ301MPort>(Vec(inXPos, yPos), Port::INPUT, module, GateJunction::IN1_INPUT + i));

        //////OUTPUTS//////
        addOutput(Port::create<PJ301MPort>(Vec(outXPos, yPos), Port::OUTPUT, module, GateJunction::OUT1_OUTPUT + i));

        //////BLINKENLIGHTS//////
        addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(outLightX, lightY), module, GateJunction::OUT1_LIGHT + i));

        addParam(ParamWidget::create<LEDBezel>(Vec(50, 1 + yPos), module, GateJunction::MUTE1_PARAM + i, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<LEDBezel>(Vec(85, 1 + yPos), module, GateJunction::INV1_PARAM + i, 0.0, 1.0, 0.0));

        addChild(ModuleLightWidget::create<MuteLight<RedLight>>(Vec(52.2, 3 + yPos), module, GateJunction::MUTE1_LIGHT + i));
        addChild(ModuleLightWidget::create<MuteLight<BlueLight>>(Vec(87.2, 3 + yPos), module, GateJunction::INV1_LIGHT + i));
    }
}

} // namespace rack_plugin_HetrickCV

using namespace rack_plugin_HetrickCV;

RACK_PLUGIN_MODEL_INIT(HetrickCV, GateJunction) {
   Model *modelGateJunction = Model::create<GateJunction, GateJunctionWidget>("HetrickCV", "Gate Junction", "Gate Junction", SWITCH_TAG, LOGIC_TAG);
   return modelGateJunction;
}
