#include "Southpole.hpp"

namespace rack_plugin_Southpole {

struct Abr : Module 
{
	enum ParamIds 
	{
        SWITCH1_PARAM,
        SWITCH2_PARAM,
        SWITCH3_PARAM,
        SWITCH4_PARAM,
        SWITCH5_PARAM,
        SWITCH6_PARAM,
        SWITCH7_PARAM,
        SWITCH8_PARAM,

		NUM_PARAMS
	};
	enum InputIds 
	{
        INA1_INPUT,
        INA2_INPUT,
        INA3_INPUT,
        INA4_INPUT,
        INA5_INPUT,
        INA6_INPUT,
        INA7_INPUT,
        INA8_INPUT,

        INB1_INPUT,
        INB2_INPUT,
        INB3_INPUT,
        INB4_INPUT,
        INB5_INPUT,
        INB6_INPUT,
        INB7_INPUT,
        INB8_INPUT,

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

        SUMA_OUTPUT,
        SUMB_OUTPUT,
        SUM_OUTPUT,

		NUM_OUTPUTS
    };
    enum LightIds 
	{
		NUM_LIGHTS
	};

    bool swState[8] = {};
    
	Abr() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) 
	{
		reset();
	}

    void step() override;

    void reset() override 
    {
        for (int i = 0; i < 8; i++) 
        {
            swState[i] = false;
		}
	}
    void randomize() override 
    {
        for (int i = 0; i < 8; i++) 
        {
            swState[i] = (randomUniform() < 0.5);
		}
    }
    
    json_t *toJson() override 
    {
		json_t *rootJ = json_object();
        json_t *swStatesJ = json_array();
        for (int i = 0; i < 8; i++) 
        {
			json_t *swStateJ = json_boolean(swState[i]);
            json_array_append_new(swStatesJ, swStateJ);
		}
        json_object_set_new(rootJ, "swStates", swStatesJ);
		return rootJ;
	}

    void fromJson(json_t *rootJ) override 
    {
        json_t *swStatesJ = json_object_get(rootJ, "swStates");
        if (swStatesJ) 
        {
            for (int i = 0; i < 8; i++) 
            {
				json_t *stateJ = json_array_get(swStatesJ, i);
				if (stateJ) {
					swState[i] = json_boolean_value(stateJ);
                }
			}
        }
	}
};


void Abr::step() 
{
    float outa = 0.;
    float outb = 0.;
    float out  = 0.;
    for(int i = 0; i < 8; i++)
    {
        swState[i] = params[SWITCH1_PARAM + i].value > 0.5;
        if ( !swState[i] ) {
            if(inputs[INA1_INPUT + i].active)
            {
                float ina = inputs[INA1_INPUT + i].value;
                outputs[OUT1_OUTPUT + i].value = ina;
                outa += ina;
                out += ina; 
            }
        } else {
            if(inputs[INB1_INPUT + i].active)
            {
                float inb = inputs[INB1_INPUT + i].value;
                outputs[OUT1_OUTPUT + i].value = inb;
                outb += inb;
                out += inb; 
            }
        } 
    }
    outputs[SUMA_OUTPUT].value = outa;
    outputs[SUMB_OUTPUT].value = outb;
    outputs[SUM_OUTPUT].value = out;
}

struct AbrWidget : ModuleWidget { 
    AbrWidget(Abr *module) : ModuleWidget(module) 
    {
        box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        {
            auto *panel = new SVGPanel();
            panel->box.size = box.size;
            panel->setBackground(SVG::load(assetPlugin(plugin, "res/Abr.svg")));
            addChild(panel);
        }

        const float x1 = 3.;
        const float x2 = 4.+20.;
        const float x3 = 5.+43.;
        const float x4 = 5.+63.;

        float yPos = 18.;

        for(int i = 0; i < 8; i++)
        {
            yPos += 32.;

            addInput(Port::create<sp_Port>(Vec(x1, yPos), Port::INPUT, module, Abr::INA1_INPUT + i));
            addParam(ParamWidget::create<sp_Switch>(Vec(x2+1, 3 + yPos), module, Abr::SWITCH1_PARAM + i, 0.0, 1.0, 0.0));
            addInput(Port::create<sp_Port>(Vec(x3, yPos), Port::INPUT, module, Abr::INB1_INPUT + i));
            addOutput(Port::create<sp_Port>(Vec(x4, yPos), Port::OUTPUT, module, Abr::OUT1_OUTPUT + i));
        }

        yPos += 48.;
        addOutput(Port::create<sp_Port>(Vec(x1, yPos), Port::OUTPUT, module, Abr::SUMA_OUTPUT));
        addOutput(Port::create<sp_Port>(Vec(x3, yPos), Port::OUTPUT, module, Abr::SUMB_OUTPUT));
        addOutput(Port::create<sp_Port>(Vec(x4, yPos), Port::OUTPUT, module, Abr::SUM_OUTPUT));
    }

};

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, Abr) {
   Model *modelAbr = Model::create<Abr,AbrWidget>("Southpole", "Abr", "Abr - A/B switch", SWITCH_TAG, MIXER_TAG);
   return modelAbr;
}
