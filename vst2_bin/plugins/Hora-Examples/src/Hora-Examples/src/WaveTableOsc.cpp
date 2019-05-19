#include "Examples.hpp"
#include "CvToFreq.h" 
#include "FreqToRate.h" 
#include "stringMux.h" 
#include "floatToString.h" 
#include "clockedCounter.h" 
#include "waveTable.h" 

struct WaveTableOsc : Module {
    CvToFreq CvToFreq_CV_74;

    FreqToRate FreqToRate_toR82;

    stringMux stringMux_stM86;

    floatToString floatToString_flt81;

    clockedCounter clockedCounter_cCo62;

    waveTable waveTable_wav72;

    float pre57_out = 0;

    char screen_WaveTableOsc_lineMessage1[100] = "";
    char screen_WaveTableOsc_lineMessage2[100] = "";
    enum ParamIds {
           rot_91_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
          mod_73_INPUT,
          mod_96_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
          mod_8_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS 
    };
    WaveTableOsc() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step()override;
};

void WaveTableOsc::step() {
    float dou98_out = 6553;
    float dou75_out = 20;
    float mod_73_INPUT_signal = inputs[mod_73_INPUT].value;
    float dou64_out = 1;
    char str67_out[100] = "saw128.gwt";
    char str84_out[100] = "saw512.gwt";
    char str85_out[100] = "saw1024.gwt";
    char str89_out[100] = "sin512.gwt";
    char str90_out[100] = "sin128.gwt";
    float rot_91_PARAM_output = params[rot_91_PARAM].value;
    int toI92_out = int(rot_91_PARAM_output);
    float sam93_signal = engineGetSampleRate();
    float dou95_out = 5;
    float mod_96_INPUT_signal = inputs[mod_96_INPUT].value;
    CvToFreq_CV_74.setinput(mod_73_INPUT_signal);
    CvToFreq_CV_74.setbase(dou75_out);
    float CV_74_output = CvToFreq_CV_74.getoutput();

  float x__Op63_result = pre57_out-dou64_out;
    FreqToRate_toR82.setinput(CV_74_output);
    FreqToRate_toR82.setsampleRate(sam93_signal);
    FreqToRate_toR82.settableLength(pre57_out);
    float toR82_output = FreqToRate_toR82.getoutput();

    stringMux_stM86.setstr1(str67_out);
    stringMux_stM86.setstr2(str84_out);
    stringMux_stM86.setstr3(str85_out);
    stringMux_stM86.setstr4(str90_out);
    stringMux_stM86.setstr5(str89_out);
    stringMux_stM86.setindex(toI92_out);
    char *stM86_output = stringMux_stM86.getoutput();

    floatToString_flt81.setinput(CV_74_output);
    char *flt81_output = floatToString_flt81.getoutput();

    strcpy(screen_WaveTableOsc_lineMessage1, stM86_output);
    strcpy(screen_WaveTableOsc_lineMessage2, flt81_output);

    clockedCounter_cCo62.setlength(x__Op63_result);
    clockedCounter_cCo62.setrate(toR82_output);
    clockedCounter_cCo62.setreset(0);
    float cCo62_output = clockedCounter_cCo62.getoutput();

    waveTable_wav72.setindex(cCo62_output);
    waveTable_wav72.setreset(mod_96_INPUT_signal);
    waveTable_wav72.settableName(stM86_output);
    float wav72_output = waveTable_wav72.getoutput();
    float wav72_bufSize = waveTable_wav72.getbufSize();

    float x__Op68_result = wav72_output/dou98_out;
  float x__Op94_result = x__Op68_result-dou95_out;
    outputs[mod_8_OUTPUT].value = x__Op94_result;
    pre57_out = wav72_bufSize;


}
struct screen_WaveTableOsc : TransparentWidget {
    WaveTableOsc *module;
    std::shared_ptr<Font> font;
    screen_WaveTableOsc () {
    font = Font::load(assetPlugin(plugin, "res/LEDCounter7.ttf"));
    }
    void updateLine1(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, char* lineMessage1) {
    nvgFontSize(vg, 20);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, -2);
    nvgFillColor(vg, nvgTransRGBA(nvgRGB( 0, 150, 144), 0xff));
    nvgText(vg, pos.x, pos.y, lineMessage1, NULL);
    }
    void updateLine2(NVGcontext *vg, Vec pos, NVGcolor DMDtextColor, int xOffsetValue, char* lineMessage2) {
    nvgFontSize(vg, 20);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, -2);
    nvgFillColor(vg, nvgTransRGBA(nvgRGB( 0, 150, 144), 0xff));
    nvgText(vg, pos.x, pos.y, lineMessage2, NULL);
    }
    void draw(NVGcontext *vg) override {
    updateLine1(vg, Vec( 5, 5), nvgRGB(0x08, 0x08, 0x08), module->screen_WaveTableOsc_lineMessage1);
    updateLine2(vg, Vec(  5, (20+20/2)), nvgRGB(0x08, 0x08, 0x08), 20, module->screen_WaveTableOsc_lineMessage2);
    }
};
struct WaveTableOscWidget : ModuleWidget {
   WaveTableOscWidget(WaveTableOsc *module);
};

WaveTableOscWidget::WaveTableOscWidget(WaveTableOsc *module) : ModuleWidget(module) {
   box.size = Vec(120, 380);
   {
       SVGPanel *panel = new SVGPanel();
       panel->box.size = box.size;
       panel->setBackground(SVG::load(assetPlugin(plugin, "res/wave.svg")));
       addChild(panel);
    }
    {
        screen_WaveTableOsc *display_screen= new screen_WaveTableOsc();
        display_screen->module = module;
        display_screen->box.pos = Vec(12.3675, 118.624);
        display_screen->box.size = Vec(234, 234);
        addChild(display_screen);
    }
    addInput(Port::create<jack>(Vec(12.4868,242), Port::INPUT, module, WaveTableOsc::mod_73_INPUT));
    addOutput(Port::create<jack>(Vec(41.9474,289.908), Port::OUTPUT, module, WaveTableOsc::mod_8_OUTPUT));
    addParam(ParamWidget::create<mediumRotarysnap_snap>(Vec(41,43.1518), module, WaveTableOsc::rot_91_PARAM, 0, 4, 0));
    addInput(Port::create<jack>(Vec(71.9312,242.122), Port::INPUT, module, WaveTableOsc::mod_96_INPUT));
}

RACK_PLUGIN_MODEL_INIT(Hora_Examples, WaveTableOsc) {
   Model *modelWaveTableOsc = Model::create<WaveTableOsc,WaveTableOscWidget>("Hora", "hora-examplesWaveTable", "wavetableOSC", OSCILLATOR_TAG);
   return modelWaveTableOsc;
}
