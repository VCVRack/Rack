#include "Southpole.hpp"

namespace rack_plugin_Southpole {

struct But : Module 
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
        OUTA1_OUTPUT,
        OUTA2_OUTPUT,
        OUTA3_OUTPUT,
        OUTA4_OUTPUT,
        OUTA5_OUTPUT,
        OUTA6_OUTPUT,
        OUTA7_OUTPUT,
        OUTA8_OUTPUT,

        OUTB1_OUTPUT,
        OUTB2_OUTPUT,
        OUTB3_OUTPUT,
        OUTB4_OUTPUT,
        OUTB5_OUTPUT,
        OUTB6_OUTPUT,
        OUTB7_OUTPUT,
        OUTB8_OUTPUT,

        SUMA1_OUTPUT,
        SUMA2_OUTPUT,
        SUMB1_OUTPUT,
        SUMB2_OUTPUT,

      NUM_OUTPUTS
    };
    enum LightIds 
   {
        /*
        SWITCH1_LIGHT,
        SWITCH2_LIGHT,
        SWITCH3_LIGHT,
        SWITCH4_LIGHT,
        SWITCH5_LIGHT,
        SWITCH6_LIGHT,
        SWITCH7_LIGHT,
        SWITCH8_LIGHT,
        */
      NUM_LIGHTS
   };

    bool swState[8] = {};
    
   But() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) 
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


void But::step() 
{
    float outa = 0.;
    float outb = 0.;
    for(int i = 0; i < 8; i++)
    {
        swState[i] = params[SWITCH1_PARAM + i].value > 0.5;
        float in = 0.;
        if(inputs[IN1_INPUT + i].active)
        {
            in = inputs[IN1_INPUT + i].value;
        }
        if ( !swState[i] ) {
            outputs[OUTA1_OUTPUT + i].value = in;
            outa += in;
        } else {
           outputs[OUTB1_OUTPUT + i].value = in;
           outb += in; 
        } 
    //    lights[SWITCH1_LIGHT + i].setBrightness(swState[i] ? 0.9 : 0.0);
    }
    outputs[SUMA1_OUTPUT].value = outa;
    outputs[SUMA2_OUTPUT].value = outa;
    outputs[SUMB1_OUTPUT].value = outb;
    outputs[SUMB2_OUTPUT].value = outb;
}

struct ButWidget : ModuleWidget { 
 
    ButWidget(But *module)  : ModuleWidget(module) 
    {
        box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        {
            auto *panel = new SVGPanel();
            panel->box.size = box.size;
            panel->setBackground(SVG::load(assetPlugin(plugin, "res/But.svg")));
            addChild(panel);
        }

        const float x1 = 3.;
        const float x2 = 4.+20.;
        const float x3 = 5.+40.;
        const float x4 = 5.+63.;

        float yPos = 18.;

        for(int i = 0; i < 8; i++)
        {
            yPos += 32.;

            addInput(Port::create<sp_Port>(Vec(x1, yPos), Port::INPUT, module, But::IN1_INPUT + i));
            addOutput(Port::create<sp_Port>(Vec(x2, yPos), Port::OUTPUT, module, But::OUTA1_OUTPUT + i));
            addParam(ParamWidget::create<sp_Switch>(Vec(x3+1, 3 + yPos), module, But::SWITCH1_PARAM + i, 0.0, 1.0, 0.0));
            addOutput(Port::create<sp_Port>(Vec(x4, yPos), Port::OUTPUT, module, But::OUTB1_OUTPUT + i));
        }

        yPos += 48.;
        addOutput(Port::create<sp_Port>(Vec(x1, yPos), Port::OUTPUT, module, But::SUMA1_OUTPUT));
        addOutput(Port::create<sp_Port>(Vec(x2, yPos), Port::OUTPUT, module, But::SUMA2_OUTPUT));
        addOutput(Port::create<sp_Port>(Vec(x3+3, yPos), Port::OUTPUT, module, But::SUMB1_OUTPUT));
        addOutput(Port::create<sp_Port>(Vec(x4, yPos), Port::OUTPUT, module, But::SUMB2_OUTPUT));
    }

};

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, But) {
   Model *modelBut   = Model::create<But,ButWidget>(      "Southpole", "But",       "But - A/B buss", SWITCH_TAG, MIXER_TAG);
   return modelBut;
}
