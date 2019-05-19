#include "Examples.hpp"
#include "intToString.h" 
#include "dsp/digital.hpp" 

struct miniSeq : Module {
    bool gat59_wasTrigged = false;

    bool gat33_wasTrigged = false;

    bool clo29_wasReset = false;

    float clo29_phase = 0;

    float arr50_inputs[4] = {0};

    float arr50[4] = {0};

    int cou28_currentCount = 0;

    SchmittTrigger clockTriggercou28;

    float arr23_inputs[4] = {0};

    float arr23[4] = {0};

    float arr34_inputs[4] = {0};

    float arr34[4] = {0};

    intToString intToString_int56;

    char screen_miniSeq_lineMessage1[100] = "";
    char screen_miniSeq_lineMessage2[100] = "";
    enum ParamIds {
           swi_51_PARAM,
           swi_38_PARAM,
           swi_37_PARAM,
           swi_36_PARAM,
           swi_35_PARAM,
           kno_30_PARAM,
           kno_27_PARAM,
           kno_26_PARAM,
           kno_25_PARAM,
           kno_24_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
          mod_44_INPUT,
          mod_32_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
          mod_43_OUTPUT,
          mod_31_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS 
    };
    miniSeq() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step()override;
};

void miniSeq::step() {
    float swi_51_PARAM_output = params[swi_51_PARAM].value;
    float mod_44_INPUT_signal = inputs[mod_44_INPUT].value;
    float swi_38_PARAM_output = params[swi_38_PARAM].value;
    float swi_37_PARAM_output = params[swi_37_PARAM].value;
    float swi_36_PARAM_output = params[swi_36_PARAM].value;
    float swi_35_PARAM_output = params[swi_35_PARAM].value;
    float mod_32_INPUT_signal = inputs[mod_32_INPUT].value;
    float kno_30_PARAM_output = params[kno_30_PARAM].value;
    float kno_27_PARAM_output = params[kno_27_PARAM].value;
    float kno_26_PARAM_output = params[kno_26_PARAM].value;
    float kno_25_PARAM_output = params[kno_25_PARAM].value;
    float kno_24_PARAM_output = params[kno_24_PARAM].value;
    char str54_out[100] = "";
    float dou58_out = 1;
    bool gat59_pulse = false;
    if(mod_44_INPUT_signal != 0 && gat59_wasTrigged == false)
    {
        gat59_wasTrigged = true;
        gat59_pulse = true;
   }    else if( mod_44_INPUT_signal == 0)
    {
    gat59_wasTrigged = false;
      }
    float gat59_out = gat59_pulse ? 10.0 : 0.0;

    int toI52_out = int(swi_51_PARAM_output);
    bool gat33_pulse = false;
    if(mod_32_INPUT_signal != 0 && gat33_wasTrigged == false)
    {
        gat33_wasTrigged = true;
        gat33_pulse = true;
   }    else if( mod_32_INPUT_signal == 0)
    {
    gat33_wasTrigged = false;
      }
    float gat33_out = gat33_pulse ? 10.0 : 0.0;

    bool clo29_nextStep = false;
    float clo29_clockTime = powf(2.0, kno_30_PARAM_output);
    clo29_phase += clo29_clockTime/engineGetSampleRate();
    if (gat59_out == 0 )
    {
        if (clo29_phase >= 1.0)
        {
       clo29_phase  -= 1.0;
        clo29_nextStep = true;
      clo29_wasReset = false;
        }
    }
        else if(clo29_wasReset == false) 
    {
      clo29_nextStep = true;
        clo29_wasReset = true;
        clo29_phase = 0;
    }
    float clo29_out = 0;
    if(clo29_nextStep == true)
    {
        clo29_out= 5.0;
    }
    else
    {
        clo29_out = 0.0;
    }

    arr50_inputs[0] = clo29_out;
     arr50_inputs[1] = gat33_out;
     arr50_inputs[2] = 0;
     arr50_inputs[3] = 0;
     for(int arr50_realsize = 0; arr50_realsize < 4;arr50_realsize ++)
    {
        if(arr50_realsize!=toI52_out)
        {
            arr50[arr50_realsize ] = arr50_inputs[arr50_realsize];
        }
        else
        {
             arr50[arr50_realsize ] = arr50_inputs[toI52_out];
        }
    }
    float arr50_out = arr50[toI52_out];

    if(clockTriggercou28.process(arr50_out))
    {
    cou28_currentCount++;
    }
    if(cou28_currentCount>3)
    {
    cou28_currentCount= 0;
    }
    if (gat59_out > 0.2 ) 
    {
     cou28_currentCount = 0;
   }
    int cou28_count = cou28_currentCount;

    arr23_inputs[0] = kno_24_PARAM_output;
     arr23_inputs[1] = kno_25_PARAM_output;
     arr23_inputs[2] = kno_27_PARAM_output;
     arr23_inputs[3] = kno_26_PARAM_output;
     for(int arr23_realsize = 0; arr23_realsize < 4;arr23_realsize ++)
    {
        if(arr23_realsize!=cou28_count)
        {
            arr23[arr23_realsize ] = arr23_inputs[arr23_realsize];
        }
        else
        {
             arr23[arr23_realsize ] = arr23_inputs[cou28_count];
        }
    }
    float arr23_out = arr23[cou28_count];

    float x__Op57_result = cou28_count+dou58_out;
    arr34_inputs[0] = swi_35_PARAM_output;
     arr34_inputs[1] = swi_36_PARAM_output;
     arr34_inputs[2] = swi_37_PARAM_output;
     arr34_inputs[3] = swi_38_PARAM_output;
     for(int arr34_realsize = 0; arr34_realsize < 4;arr34_realsize ++)
    {
        if(arr34_realsize!=cou28_count)
        {
            arr34[arr34_realsize ] = arr34_inputs[arr34_realsize];
        }
        else
        {
             arr34[arr34_realsize ] = arr34_inputs[cou28_count];
        }
    }
    float arr34_out = arr34[cou28_count];

    outputs[mod_31_OUTPUT].value = arr23_out;
    intToString_int56.setinput(x__Op57_result);
    char *int56_output = intToString_int56.getoutput();

    float x__Op42_result = 0;
  if (arr50_out != 0 && arr34_out != 0) 
{ 
   x__Op42_result = 1;
    }
    float x__Op41_result = 0;
  if (arr50_out != 0 && arr34_out != 0) 
{ 
   x__Op41_result = 1;
    }
    float x__Op40_result = 0;
  if (arr50_out != 0 && arr34_out != 0) 
{ 
   x__Op40_result = 1;
    }
    float x__Op39_result = 0;
  if (arr50_out != 0 && arr34_out != 0) 
{ 
   x__Op39_result = 1;
    }
    strcpy(screen_miniSeq_lineMessage1, int56_output);
    strcpy(screen_miniSeq_lineMessage2, str54_out);

    outputs[mod_43_OUTPUT].value = x__Op39_result + x__Op40_result + x__Op41_result + x__Op42_result;

}
struct screen_miniSeq : TransparentWidget {
    miniSeq *module;
    std::shared_ptr<Font> font;
    screen_miniSeq () {
    font = Font::load(assetPlugin(plugin, "res/LEDCounter7.ttf"));
    }
    void updateLine1(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, char* lineMessage1) {
    nvgFontSize(vg, 20);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, -2);
    nvgFillColor(vg, nvgTransRGBA(nvgRGB( 255, 255, 255), 0xff));
    nvgText(vg, pos.x, pos.y, lineMessage1, NULL);
    }
    void updateLine2(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, int xOffsetValue, char* lineMessage2) {
    nvgFontSize(vg, 20);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, -2);
    nvgFillColor(vg, nvgTransRGBA(nvgRGB( 255, 255, 255), 0xff));
    nvgText(vg, pos.x, pos.y, lineMessage2, NULL);
    }
    void draw(NVGcontext *vg) override {
    updateLine1(vg, Vec( 5, 5), nvgRGB(0x08, 0x08, 0x08), module->screen_miniSeq_lineMessage1);
    updateLine2(vg, Vec(  5, (20+20/2)), nvgRGB(0x08, 0x08, 0x08), 20, module->screen_miniSeq_lineMessage2);
    }
};
struct miniSeqWidget : ModuleWidget {
   miniSeqWidget(miniSeq *module);
};

miniSeqWidget::miniSeqWidget(miniSeq *module) : ModuleWidget(module) {
   box.size = Vec(120, 380);
   {
       SVGPanel *panel = new SVGPanel();
       panel->box.size = box.size;
       panel->setBackground(SVG::load(assetPlugin(plugin, "res/sequencer.svg")));
       addChild(panel);
    }
    addParam(ParamWidget::create<switch_0>(Vec(85,271), module, miniSeq::swi_51_PARAM, 0, 1, 0));
    addInput(Port::create<jack>(Vec(46,333), Port::INPUT, module, miniSeq::mod_44_INPUT));
    addOutput(Port::create<jack>(Vec(89,333), Port::OUTPUT, module, miniSeq::mod_43_OUTPUT));
    addParam(ParamWidget::create<switch_0>(Vec(90.0526,195.434), module, miniSeq::swi_38_PARAM, 0, 1, 0));
    addParam(ParamWidget::create<switch_0>(Vec(90.05,147.71), module, miniSeq::swi_37_PARAM, 0, 1, 0));
    addParam(ParamWidget::create<switch_0>(Vec(90.05,93), module, miniSeq::swi_36_PARAM, 0, 1, 0));
    addParam(ParamWidget::create<switch_0>(Vec(90.05,46), module, miniSeq::swi_35_PARAM, 0, 1, 0));
    addInput(Port::create<jack>(Vec(46.9211,263), Port::INPUT, module, miniSeq::mod_32_INPUT));
    addOutput(Port::create<jack>(Vec(1,333), Port::OUTPUT, module, miniSeq::mod_31_OUTPUT));
    addParam(ParamWidget::create<mediumKnob>(Vec(3,259), module, miniSeq::kno_30_PARAM, 0.01, 10, 5));
    addParam(ParamWidget::create<mediumKnob>(Vec(30.8947,137.026), module, miniSeq::kno_27_PARAM, 0, 10, 5));
    addParam(ParamWidget::create<mediumKnob>(Vec(32.9211,188.053), module, miniSeq::kno_26_PARAM, 0, 10, 5));
    addParam(ParamWidget::create<mediumKnob>(Vec(30.9474,80.9737), module, miniSeq::kno_25_PARAM, 0, 10, 5));
    addParam(ParamWidget::create<mediumKnob>(Vec(29.4342,34), module, miniSeq::kno_24_PARAM, 0, 10, 5));
    {
        screen_miniSeq *display_screen= new screen_miniSeq();
        display_screen->module = module;
        display_screen->box.pos = Vec(12.2632, 241.266);
        display_screen->box.size = Vec(234, 234);
        addChild(display_screen);
    }
}

RACK_PLUGIN_MODEL_INIT(Hora_Examples, miniSeq) {
   Model *modelminiSeq = Model::create<miniSeq,miniSeqWidget>("Hora", "Hora-ExamplesSeq", "miniSequencer", SEQUENCER_TAG);
   return modelminiSeq;
}
