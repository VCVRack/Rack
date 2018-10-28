///////////////////////////////////////////////////
//  dBiz Util2
// 
///////////////////////////////////////////////////

#include "dBiz.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_dBiz {

struct Util2 : Module {
    enum ParamIds
    {
        MODE_PARAM,
        VALUE_PARAM = MODE_PARAM + 4,
        BUTTON_PARAM = VALUE_PARAM +4,
        RANGE_PARAM = BUTTON_PARAM +4,
        GLIDE_PARAM= RANGE_PARAM + 2,
        RISE_PARAM = RANGE_PARAM + 2,
        FALL_PARAM =  RISE_PARAM + 2,
        NUM_PARAMS =  FALL_PARAM + 2
    };
    enum InputIds
    {
        BUTTON_INPUT,
        TRIG_INPUT=BUTTON_INPUT + 4,
        IN_INPUT = TRIG_INPUT + 2,
        NUM_INPUTS = IN_INPUT + 2
    };
    enum OutputIds
    {
        BUTTON_OUTPUT,
        EG_OUTPUT = BUTTON_OUTPUT + 4,
        OUT_OUTPUT = EG_OUTPUT + 2,
        NUM_OUTPUTS = OUT_OUTPUT + 2
    };

    enum LighIds
	{
        BUTTON_LIGHT,
	    NUM_LIGHTS = BUTTON_LIGHT + 4
	};

    float out[2]{};
    float outg[2]{};
    float eg_out[2]{};

    bool gate[2] = {};
    bool gateEg[2] = {};

    bool gateState[4] = {};
    bool pulse[4];

    SchmittTrigger trigger[2];
    SchmittTrigger btrigger[4];

    PulseGenerator buttonPulse[4];

    Util2() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    json_t *toJson() override
    {
        json_t *rootJ = json_object();

        json_t *gatesJ = json_array();
        for (int i = 0; i < 4; i++)
        {
            json_t *gateJ = json_integer((int)gateState[i]);
            json_array_append_new(gatesJ, gateJ);
        }
        json_object_set_new(rootJ, "gate", gatesJ);

        return rootJ;
    }

    void fromJson(json_t *rootJ) override
    {
        json_t *gatesJ = json_object_get(rootJ, "gates");
        if (gatesJ)
        {
            for (int i = 0; i < 8; i++)
            {
                json_t *gateJ = json_array_get(gatesJ, i);
                if (gateJ)
                    gateState[i] = !!json_integer_value(gateJ);
            }
        }
    }

    void reset() override
    {
        for (int i = 0; i < 4; i++)
        {
            gateState[i] = false;
        }
    }


};

/////////////////////////////////////////////////////

static float shapeDelta(float delta, float tau, float shape)
{
    float lin = sgn(delta) * 10.0 / tau;
    if (shape < 0.0)
    {
        float log = sgn(delta) * 40.0 / tau / (fabsf(delta) + 1.0);
        return crossfade(lin, log, -shape * 0.95);
    }
    else
    {
        float exp = M_E * delta / tau;
        return crossfade(lin, exp, shape * 0.90);
    }
}

/////////////////////////////////////////////////////
void Util2::step() {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    for (int c = 0; c < 2; c++)
    {
        float in = inputs[IN_INPUT + c].value;
        float shape = 0.0 ; 
        float delta = in - out[c];

        bool rising = false;
        bool falling = false;

        if (delta > 0)
        {
            // Rise
            float riseCv = params[GLIDE_PARAM + c].value;
            float rise = 1e-1 * powf(2.0, riseCv * 10.0);
            out[c] += shapeDelta(delta, rise, shape) / engineGetSampleRate();
            rising = (in - out[c] > 1e-3);
            if (!rising)
            {
                gate[c] = false;
            }
        }
        else if (delta < 0)
        {
            // Fall
            float fallCv = params[GLIDE_PARAM + c].value;
            float fall = 1e-1 * powf(2.0, fallCv * 10.0);
            out[c] += shapeDelta(delta, fall, shape) / engineGetSampleRate();
            falling = (in - out[c] < -1e-3);
        }
        else
        {
            gate[c] = false;
        }

        if (!rising && !falling)
        {
            out[c] = in;
        }

        outputs[OUT_OUTPUT + c].value = out[c];
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    for (int c = 0; c < 2; c++)
    {
       float in = 0.0f; //inputs[IN_INPUT + c].value;
       if (trigger[c].process(inputs[TRIG_INPUT + c].value))
       {
           gateEg[c] = true;
       }
       if (gateEg[c])
       {
           in = 5.0;
       }

       float shape = 0.0;
       float delta = in - outg[c];

       float minTime;
       switch ((int)params[RANGE_PARAM + c].value)
       {
       case 0: minTime = 1e-1; break;
       case 1: minTime = 1e-2; break;
       default:minTime = 1e-3; break;
       }

        bool rising = false;
        bool falling = false;

        if (delta > 0)
        {
            // Rise
            float riseCv = params[RISE_PARAM + c].value;
            float rise = minTime * powf(2.0, riseCv * 10.0);
            outg[c] += shapeDelta(delta, rise, shape) / engineGetSampleRate();
            rising = (in - outg[c] > 1e-3);
            if (!rising)
            {
                gateEg[c] = false;
            }
        }
        else if (delta < 0)
        {
            // Fall
            float fallCv = params[FALL_PARAM + c].value;
            float fall = minTime * powf(2.0, fallCv * 10.0);
            outg[c] += shapeDelta(delta, fall, shape) / engineGetSampleRate();
            falling = (in - outg[c] < -1e-3);
        }
        else
        {
            gateEg[c] = false;
        }

        if (!rising && !falling)
        {
            outg[c] = in;
        }

        outputs[EG_OUTPUT + c].value = outg[c];
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////

    for (int i=0;i<4;i++)
    {
        if(params[MODE_PARAM+i].value==0)
        {
            if (btrigger[i].process(params[BUTTON_PARAM+i].value*10+inputs[BUTTON_INPUT+i].value))
            {
               // button[i] = true;
                lights[BUTTON_LIGHT + i].value = 1.0f;
                buttonPulse[i].trigger(1e-3);
            }
            if (lights[BUTTON_LIGHT + i].value>0)
            {
                lights[BUTTON_LIGHT + i].value -= lights[BUTTON_LIGHT + i].value / 0.02 / engineGetSampleRate();
            }

            pulse[i] = buttonPulse[i].process(1.0f / engineGetSampleRate());

            outputs[BUTTON_OUTPUT + i].value = pulse[i] ? 10.0f : 0.0f;
        }

        if (params[MODE_PARAM + i].value == 1)
        {
            if (btrigger[i].process(params[BUTTON_PARAM + i].value * 10 + inputs[BUTTON_INPUT + i].value))
            {
                gateState[i] = !gateState[i];
            }
            lights[BUTTON_LIGHT + i].value = gateState[i] ? 1.0 : 0.0;

            if (gateState[i])
            {
                outputs[BUTTON_OUTPUT + i].value = params[VALUE_PARAM + i].value;
            }
            else
            {
                outputs[BUTTON_OUTPUT + i].value = 0.0;
            }
        }    
    }

}

struct Util2Widget : ModuleWidget 
{
Util2Widget(Util2 *module) : ModuleWidget(module)
{
	box.size = Vec(15*10, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin,"res/Util2.svg")));
		addChild(panel);
    }

//Screw
  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
    
   int knob=33;
   int jack = 28;
   int si = 10;   

   //
   for (int i = 0; i < 2; i++)
   {
	    addParam(ParamWidget::create<SDKnob>(Vec(30 + knob, 20 + knob * i), module, Util2::GLIDE_PARAM + i, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<SDKnob>(Vec(40 , 91 + knob * i), module, Util2::RISE_PARAM + i, 0.0, 1.0, 0.0));
        addParam(ParamWidget::create<SDKnob>(Vec(40 + knob , 91 + knob * i), module, Util2::FALL_PARAM + i, 0.0, 1.0, 0.0));
        addInput(Port::create<PJ301MVAPort>(Vec(si, 23 + knob * i), Port::INPUT, module, Util2::IN_INPUT+i));
        addOutput(Port::create<PJ301MVAPort>(Vec(si + jack, 23 + knob * i), Port::OUTPUT, module, Util2::OUT_OUTPUT+i));

        addInput(Port::create<PJ301MVAPort>(Vec(si, 94 + knob * i), Port::INPUT, module, Util2::TRIG_INPUT + i));
        addOutput(Port::create<PJ301MVAPort>(Vec(40 + knob*2, 94 + knob * i), Port::OUTPUT, module, Util2::EG_OUTPUT + i));
        addParam(ParamWidget::create<MCKSSS>(Vec(43 + knob * 2.7, 95 + knob * i), module, Util2::RANGE_PARAM + i, 0.0, 2.0, 0.0));

        // addInput(Port::create<PJ301MVAPort>(Vec(si + 40, 22.5 + knob * i), Port::INPUT, module, Util2::SUB1_INPUT + i));
        // addInput(Port::create<PJ301MVAPort>(Vec(si + 40, 173.5 + knob * i), Port::INPUT, module, Util2::SUB2_INPUT + i));
    }
    for (int i=0;i<4;i++)
    {
    addParam(ParamWidget::create<LEDBezel>(Vec(si+5+knob * i,170), module, Util2::BUTTON_PARAM + i, 0.0, 1.0, 0.0));
    addChild(GrayModuleLightWidget::create<BigLight<OrangeLight>>(Vec(si +5+ 1 + knob * i, 171), module, Util2::BUTTON_LIGHT + i));
    addParam(ParamWidget::create<SDKnob>(Vec(si +2 + knob * i, 170 + jack), module, Util2::VALUE_PARAM + i, -10.0, 10.0, 0.0));
    addInput(Port::create<PJ301MVAPort>(Vec(si + 3.5 + knob * i, 175 + jack * 2), Port::INPUT, module, Util2::BUTTON_INPUT + i));
    addOutput(Port::create<PJ301MVAPort>(Vec(si + 3.5 + knob * i, 175 + jack * 3), Port::OUTPUT, module, Util2::BUTTON_OUTPUT + i));
    addParam(ParamWidget::create<SilverSwitch>(Vec(si + 2 + knob * i, 175 + jack*4), module, Util2::MODE_PARAM + i, 0.0, 1.0, 0.0));

    // addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 2, 310), Port::OUTPUT, module, Util2::CD_OUTPUT));
    // addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 3, 310), Port::OUTPUT, module, Util2::TRIG_OUTPUT));
    // addParam(ParamWidget::create<MCKSSS>(Vec(15 + jack * 4, 313), module, Util2::MODE_PARAM + 0, 0.0, 1.// 0, 0.0));

    // addInput(Port::create<PJ301MVAPort>(Vec(15, 310 + jack), Port::INPUT, module, Util2::CLOCKB_INPUT));
    // addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 1, 310 + jack), Port::OUTPUT, module, Util2::AB2_OUTPUT));
    // addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 2, 310 + jack), Port::OUTPUT, module, Util2::CD2_OUTPUT));
    // addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 3, 310 + jack), Port::OUTPUT, module, Util2::TRIGB_OUTPUT));
    }

}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, Util2) {
   Model *modelUtil2 = Model::create<Util2, Util2Widget>("dBiz", "Util2", "Util2", QUANTIZER_TAG);
   return modelUtil2;
}
