#include "Examples.hpp"
#include "clockedCounter.h" 
#include "ControlledBuffer.h" 
#include "dsp/digital.hpp" 

struct buf : Module {
    bool gat76_wasTrigged = false;

    int cou75_currentCount = 0;

    SchmittTrigger clockTriggercou75;

    float arr80_inputs[3] = {0};

    float arr80[3] = {0};

    bool gat66_wasTrigged = false;

    clockedCounter clockedCounter_cCo62;

    ControlledBuffer ControlledBuffer_cBu14;

    float pre57_out = 0;

    enum ParamIds {
           swi_81_PARAM,
           swi_33_PARAM,
           kno_25_PARAM,
           but_16_PARAM,
           but_5_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
          mod_74_INPUT,
          mod_69_INPUT,
          mod_68_INPUT,
          mod_67_INPUT,
          mod_3_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
          mod_8_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
          led_73_LIGHT,
          led_72_LIGHT,
        NUM_LIGHTS
    };
    buf();
    void step()override;
};

buf::buf() {
    params.resize(NUM_PARAMS);
    lights.resize(NUM_LIGHTS);
    inputs.resize(NUM_INPUTS);
    outputs.resize(NUM_OUTPUTS);
}
void buf::step() {
    float swi_81_PARAM_output = params[swi_81_PARAM].value;
    float mod_74_INPUT_signal = inputs[mod_74_INPUT].value;
    float dou71_out = 5;
    float mod_69_INPUT_signal = inputs[mod_69_INPUT].value;
    float mod_68_INPUT_signal = inputs[mod_68_INPUT].value;
    float mod_67_INPUT_signal = inputs[mod_67_INPUT].value;
    float swi_33_PARAM_output = params[swi_33_PARAM].value;
    float kno_25_PARAM_output = params[kno_25_PARAM].value;
    float but_16_PARAM_output = params[but_16_PARAM].value;
    float mod_3_INPUT_signal = inputs[mod_3_INPUT].value;
    float but_5_PARAM_output = params[but_5_PARAM].value;
    float dou64_out = 1;
    float dou85_out = 0;
    int toI82_out = int(swi_81_PARAM_output);
    bool gat76_pulse = false;
    if(mod_74_INPUT_signal != 0 && gat76_wasTrigged == false)
    {
        gat76_wasTrigged = true;
        gat76_pulse = true;
   }    else if( mod_74_INPUT_signal == 0)
    {
    gat76_wasTrigged = false;
      }
    float gat76_out = gat76_pulse ? 10.0 : 0.0;

    if(clockTriggercou75.process(gat76_out))
    {
    cou75_currentCount++;
    }
    if(cou75_currentCount>1)
    {
    cou75_currentCount= 0;
    }
    if (0 > 0.2 ) 
    {
     cou75_currentCount = 0;
   }
    int cou75_count = cou75_currentCount;

lights[led_72_LIGHT].value = but_5_PARAM_output + mod_67_INPUT_signal;
    float x__Op70_result = mod_69_INPUT_signal/dou71_out;
  float x__Op63_result = pre57_out-dou64_out;
    float x__Op84_result = 0;
  if (mod_68_INPUT_signal > dou85_out) 
   { 
   x__Op84_result = 1;
   }
    arr80_inputs[0] = cou75_count;
     arr80_inputs[1] = swi_33_PARAM_output;
     arr80_inputs[2] = 0;
     arr80_inputs[3] = 0;
     for(int arr80_realsize = 0; arr80_realsize < 3;arr80_realsize ++)
    {
        if(arr80_realsize!=toI82_out)
        {
            arr80[arr80_realsize ] = arr80_inputs[arr80_realsize];
        }
        else
        {
             arr80[arr80_realsize ] = arr80_inputs[toI82_out];
        }
    }
    float arr80_out = arr80[toI82_out];

lights[led_73_LIGHT].value = arr80_out;
    bool gat66_pulse = false;
    if(x__Op84_result + but_16_PARAM_output != 0 && gat66_wasTrigged == false)
    {
        gat66_wasTrigged = true;
        gat66_pulse = true;
   }    else if( x__Op84_result + but_16_PARAM_output == 0)
    {
    gat66_wasTrigged = false;
      }
    float gat66_out = gat66_pulse ? 10.0 : 0.0;

    float x__Op24_result = kno_25_PARAM_output + x__Op70_result*arr80_out;
    clockedCounter_cCo62.setlength(x__Op63_result);
    clockedCounter_cCo62.setrate(x__Op24_result);
    clockedCounter_cCo62.setreset(gat66_out);
    float cCo62_output = clockedCounter_cCo62.getoutput();

    int toI54_out = int(cCo62_output);
    ControlledBuffer_cBu14.setinput(mod_3_INPUT_signal);
    ControlledBuffer_cBu14.setrec(but_5_PARAM_output + mod_67_INPUT_signal);
    ControlledBuffer_cBu14.setread(arr80_out);
    ControlledBuffer_cBu14.setindex(toI54_out);
    ControlledBuffer_cBu14.setreset(gat66_out);
    float cBu14_output = ControlledBuffer_cBu14.getoutput();
    float cBu14_bufSize = ControlledBuffer_cBu14.getbufSize();

    outputs[mod_8_OUTPUT].value = cBu14_output;
    pre57_out = cBu14_bufSize;


}
struct bufWidget : ModuleWidget {
   bufWidget(buf *module);
};

bufWidget::bufWidget(buf *module) : ModuleWidget(module) {
   box.size = Vec(120, 380);
   {
       SVGPanel *panel = new SVGPanel();
       panel->box.size = box.size;
       panel->setBackground(SVG::load(assetPlugin(plugin, "res/recorder.svg")));
       addChild(panel);
    }
    addParam(ParamWidget::create<switch_0>(Vec(22.0143,103.699), module, buf::swi_81_PARAM, 0, 1, 0));
    addInput(Port::create<jack>(Vec(46.6637,115.553), Port::INPUT, module, buf::mod_74_INPUT));
    addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(33.9625,146.15), module, buf::led_73_LIGHT));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(31.2876,63.1705), module, buf::led_72_LIGHT));
    addInput(Port::create<jack>(Vec(71.823,177.781), Port::INPUT, module, buf::mod_69_INPUT));
    addInput(Port::create<jack>(Vec(72.1072,244.442), Port::INPUT, module, buf::mod_68_INPUT));
    addInput(Port::create<jack>(Vec(71.7332,27.7387), Port::INPUT, module, buf::mod_67_INPUT));
    addParam(ParamWidget::create<switch_0>(Vec(53.9451,87.6101), module, buf::swi_33_PARAM, 0, 1, 0));
    addParam(ParamWidget::create<mediumKnob>(Vec(16.53,173.204), module, buf::kno_25_PARAM, 0.01, 2, 1));
    addParam(ParamWidget::create<button>(Vec(24.2008,250.621), module, buf::but_16_PARAM, 0, 1, 0));
    addInput(Port::create<jack>(Vec(18.8916,316.217), Port::INPUT, module, buf::mod_3_INPUT));
    addParam(ParamWidget::create<button>(Vec(23.9387,33.7277), module, buf::but_5_PARAM, 0, 1, 0));
    addOutput(Port::create<jack>(Vec(71.952,316.325), Port::OUTPUT, module, buf::mod_8_OUTPUT));
}

RACK_PLUGIN_MODEL_INIT(Hora_Examples, buf) {
   Model *modelbuf = Model::create<buf,bufWidget>("Hora", "hora-examplesBuf", "buffer", OSCILLATOR_TAG);
   return modelbuf;
}
