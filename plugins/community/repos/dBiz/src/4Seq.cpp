///////////////////////////////////////////////////
//  dBiz FourSeq
// 
///////////////////////////////////////////////////

#include "dBiz.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_dBiz {

/////added fine out /////////////////////////////////////////////////
struct FourSeq : Module {
    enum ParamIds
    {   
        RESET_PARAM,
        STEPA_PARAM,
        STEPB_PARAM,
        GATEA_PARAM,
        GATEB_PARAM= GATEA_PARAM+4,
        SEQA_PARAM = GATEB_PARAM+4,
        SEQB_PARAM =SEQA_PARAM+4,
        NUM_PARAMS = SEQB_PARAM+4
    };
    enum InputIds
    {
        RESET_INPUT,
        CLKA_INPUT,
        CLKB_INPUT,
        CVA_INPUT,
        CVB_INPUT = CVA_INPUT +4,
        NUM_INPUTS = CVB_INPUT +4
    };
	enum OutputIds
	{
        SEQA_OUTPUT,
        SEQB_OUTPUT,
        GATEA_OUTPUT,
        GATEB_OUTPUT,
		NUM_OUTPUTS
	};

	enum LighIds
	{
        RESET_LIGHT,
        SEQA_LIGHT,
        SEQB_LIGHT = SEQA_LIGHT + 4,
        GATEA_LIGHT = SEQB_LIGHT+ 4,
        GATEB_LIGHT = GATEA_LIGHT + 4,
		NUM_LIGHTS = GATEB_LIGHT + 4
	};

    SchmittTrigger clk;
    SchmittTrigger clkb;
    SchmittTrigger reset_button;

    PulseGenerator gate1;
    PulseGenerator gate2;

    SchmittTrigger gate_a[4] = {};
    SchmittTrigger gate_b[4] = {};

    bool gateState_a[4] = {};
    bool gateState_b[4] = {};
    bool gateState[8] = {};

    bool pulse1; 
    bool pulse2; 

    int clk1C = 0;
    int clk2C = 0;

    int maxStepA = 0;
    int maxStepB = 0;  

    enum GateMode
    {
        TRIGGER,
        RETRIGGER,
        CONTINUOUS
    };
    GateMode gateMode = TRIGGER;


    FourSeq() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    json_t *toJson() override
    {
        json_t *rootJ = json_object();

        json_t *gatesJ = json_array();
        for (int i = 0; i < 8; i++)
        {
            json_t *gateJ = json_integer((int)gateState[i]);
            json_array_append_new(gatesJ, gateJ);
        }
        json_object_set_new(rootJ, "gate", gatesJ);

        json_t *gateModeJ = json_integer((int)gateMode);
        json_object_set_new(rootJ, "gateMode", gateModeJ);

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

        // gateMode
        json_t *gateModeJ = json_object_get(rootJ, "gateMode");
        if (gateModeJ)
            gateMode = (GateMode)json_integer_value(gateModeJ);
    }

    void reset() override
    {
        for (int i = 0; i < 8; i++)
        {
            gateState[i] = false;
        }
    }

};

    /////////////////////////////////////////////////////
    void FourSeq::step()
    {
        if (params[STEPA_PARAM].value == 0)  maxStepA = 3; 
        if (params[STEPA_PARAM].value == 1) { maxStepA = 2; lights[SEQA_LIGHT+3].value = 0.0;} 
        if (params[STEPA_PARAM].value == 2) {maxStepA = 1;  lights[SEQA_LIGHT+2].value = 0.0;}

        if (params[STEPB_PARAM].value == 0) maxStepB = 3;
        if (params[STEPB_PARAM].value == 1) {maxStepB = 2; lights[SEQB_LIGHT+3].value = 0.0;} 
        if (params[STEPB_PARAM].value == 2) {maxStepB = 1; lights[SEQB_LIGHT+2].value = 0.0;}


        if (reset_button.process(params[RESET_PARAM].value+inputs[RESET_INPUT].value))
        {
            reset();
            clk1C=0;
            lights[SEQA_LIGHT + clk1C].value = 1.0f;
            clk2C=0;
            lights[SEQB_LIGHT + clk2C].value = 1.0;
            lights[RESET_LIGHT].value=1.0;
        }

        if(lights[RESET_LIGHT].value>0)
        {
            lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / 0.1 / engineGetSampleRate();
        }


        if (inputs[CLKA_INPUT].active)
        {

            if (clk.process(inputs[CLKA_INPUT].value))
            {
                clk1C++;
                lights[SEQA_LIGHT + clk1C].value = 1.0f;
                gate1.trigger(1e-3);
            }

            if (clk1C > maxStepA)
            {
                clk1C = 0;
                lights[SEQA_LIGHT + clk1C].value = 1.0;
            }
            pulse1 = gate1.process(1.0f / engineGetSampleRate());

            if (lights[SEQA_LIGHT + clk1C].value > 0)
            {
                lights[SEQA_LIGHT + clk1C].value -= lights[SEQA_LIGHT + clk1C].value / 0.01 / engineGetSampleRate();
            }



        }
        else for (int i=0;i<4;i++){ lights[SEQA_LIGHT+i].value=0.0;}

        if (inputs[CLKB_INPUT].active)
        {
            if (clkb.process(inputs[CLKB_INPUT].value))
            {
                clk2C++;
                lights[SEQB_LIGHT + clk2C].value = 1.0f;
                gate2.trigger(1e-3);
            }

            if (clk2C > maxStepB)
            {
                clk2C = 0;
                lights[SEQB_LIGHT + clk2C].value = 1.0;
            }

            pulse2 = gate2.process(1.0f / engineGetSampleRate());

            if (lights[SEQB_LIGHT + clk2C].value > 0)
            {
                lights[SEQB_LIGHT + clk2C].value -= lights[SEQB_LIGHT + clk2C].value / 0.01 / engineGetSampleRate();
            }
        }
        else for (int i=0;i<4;i++){ lights[SEQB_LIGHT+i].value=0.0;}

        for (int i = 0; i < 4; i++)
        {
            gateState[i] = gateState_a[i];
            gateState[4 + i] = gateState_b[i];
        }

        if (gateState_a[clk1C])
        {
            outputs[SEQA_OUTPUT].value =clamp(inputs[CVA_INPUT+clk1C].value+params[SEQA_PARAM + clk1C].value,-3.0,3.0);
            outputs[GATEA_OUTPUT].value = pulse1 ? 10.0f : 0.0f;
        }
        if (gateState_b[clk2C])
        {
            outputs[SEQB_OUTPUT].value =clamp(inputs[CVB_INPUT+clk2C].value+params[SEQB_PARAM + clk2C].value,-3.0,3.0);
            outputs[GATEB_OUTPUT].value = pulse2 ? 10.0f : 0.0f;
        }

            for (int i = 0; i < 4; i++)
            {
                if (gate_a[i].process(params[GATEA_PARAM + i].value))
                {
                    gateState_a[i] = !gateState_a[i];
                }
                lights[GATEA_LIGHT + i].value = gateState_a[i] ? 1.0 : 0.0;

                if (gate_b[i].process(params[GATEB_PARAM + i].value))
                {
                    gateState_b[i] = !gateState_b[i];
                }
                lights[GATEB_LIGHT + i].value = gateState_b[i] ? 1.0 : 0.0;
            }



}

struct FourSeqWidget : ModuleWidget 
{
   FourSeqWidget(FourSeq *module) : ModuleWidget(module)
{
	box.size = Vec(15*8, 380);

	{
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin,"res/FourSeq.svg")));
		   addChild(panel);
    }

//Screw
  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
    
   int knob=35;
   int jack = 27;

   for (int i = 0; i < 4; i++)
   {
       addParam(ParamWidget::create<SDKnob>(Vec(70, 28 + knob * i), module, FourSeq::SEQA_PARAM + i, -3.0,3.0, 0.0));
       addParam(ParamWidget::create<LEDBezel>(Vec(15, 31 + knob * i), module, FourSeq::GATEA_PARAM + i, 0.0, 1.0, 0.0));
       addChild(GrayModuleLightWidget::create<BigLight<OrangeLight>>(Vec(16, 32 + knob * i),module , FourSeq::GATEA_LIGHT+i));


       addParam(ParamWidget::create<SDKnob>(Vec(70, 172 + knob * i), module, FourSeq::SEQB_PARAM + i, -3.0, 3.0, 0.0));
       addParam(ParamWidget::create<LEDBezel>(Vec(15, 175 + knob * i), module, FourSeq::GATEB_PARAM + i, 0.0, 1.0, 0.0));
       addChild(GrayModuleLightWidget::create<BigLight<OrangeLight>>(Vec(16, 176 + knob * i), module, FourSeq::GATEB_LIGHT + i));

       addInput(Port::create<PJ301MVAPort>(Vec(40, 30.5 + knob * i), Port::INPUT, module,  FourSeq::CVA_INPUT + i));
       addInput(Port::create<PJ301MVAPort>(Vec(40, 173.5 + knob * i), Port::INPUT, module, FourSeq::CVB_INPUT + i));

       addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(105, 38 + knob * i), module, FourSeq::SEQA_LIGHT + i));
       addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(105, 180 + knob * i), module, FourSeq::SEQB_LIGHT + i));
   }
   addInput(Port::create<PJ301MVAPort>(Vec(14, 170 + knob * 4), Port::INPUT, module, FourSeq::CLKA_INPUT));
   addInput(Port::create<PJ301MVAPort>(Vec(14, 197 + knob * 4), Port::INPUT, module, FourSeq::CLKB_INPUT));

   addOutput(Port::create<PJ301MVAPort>(Vec(14+jack, 170 + knob * 4), Port::OUTPUT, module, FourSeq::SEQA_OUTPUT));
   addOutput(Port::create<PJ301MVAPort>(Vec(14+jack, 197 + knob *4), Port::OUTPUT, module, FourSeq::SEQB_OUTPUT));

   addOutput(Port::create<PJ301MVAPort>(Vec(14 + jack*2, 170 + knob * 4), Port::OUTPUT, module, FourSeq::GATEA_OUTPUT));
   addOutput(Port::create<PJ301MVAPort>(Vec(14 + jack*2, 197 + knob * 4), Port::OUTPUT, module, FourSeq::GATEB_OUTPUT));

   addParam(ParamWidget::create<MCKSSS>(Vec(14 + jack * 3, 172 + knob * 4), module, FourSeq::STEPA_PARAM, 0.0, 2.0, 0.0));
   addParam(ParamWidget::create<MCKSSS>(Vec(14 + jack * 3, 199 + knob * 4), module, FourSeq::STEPB_PARAM, 0.0, 2.0, 0.0));

   addParam(ParamWidget::create<LEDBezel>(Vec(35+jack, 4), module, FourSeq::RESET_PARAM, 0.0, 1.0, 0.0));
   addChild(GrayModuleLightWidget::create<BigLight<OrangeLight>>(Vec(36+jack,5), module, FourSeq::RESET_LIGHT));
   addInput(Port::create<PJ301MVAPort>(Vec(35, 4), Port::INPUT, module, FourSeq::RESET_INPUT));
}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, FourSeq) {
   Model *modelFourSeq = Model::create<FourSeq, FourSeqWidget>("dBiz", "FourSeq", "FourSeq", SEQUENCER_TAG);
   return modelFourSeq;
}
