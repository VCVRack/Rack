#include "dsp/digital.hpp"
#include "util/math.hpp"
#include "qwelk.hpp"
#include "qwelk_common.h"


#define GWIDTH          4
#define GHEIGHT         8
#define GSIZE           (GWIDTH*GHEIGHT)
#define GMID            (GWIDTH/2+(GHEIGHT/2)*GWIDTH)
#define LIGHT_SIZE      10
#define DIR_BIT_SIZE    8

namespace rack_plugin_Qwelk {

struct ModuleNews : Module {
    enum ParamIds {
        PARAM_MODE,
        PARAM_GATEMODE,
        PARAM_ROUND,
        PARAM_CLAMP,
        PARAM_INTENSITY,
        PARAM_WRAP,
        PARAM_SMOOTH,
        PARAM_UNI_BI,
        PARAM_ORIGIN,
        NUM_PARAMS
    };
    enum InputIds {
        IN_NEWS,
        IN_INTENSITY,
        IN_WRAP,
        IN_HOLD,
        IN_ORIGIN,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT_CELL,
        NUM_OUTPUTS = OUT_CELL + GSIZE
    };
    enum LightIds {
        LIGHT_GRID,
        NUM_LIGHTS = LIGHT_GRID + GSIZE
    };

    float sample;
    SchmittTrigger trig_hold;
    byte grid[GSIZE] {};
    float buffer[GSIZE] {};

    ModuleNews() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    
    inline void set(int i, bool gatemode)
    {
        if (gatemode)
            grid[i] ^= 1;
        else
            grid[i] += 1;
    }
    inline void set(int x, int y, bool gatemode)
    {
        set(x + y * GWIDTH, gatemode);
    }
};

void ModuleNews::step()
{
    bool    mode        = params[PARAM_MODE].value > 0.0;
    bool    gatemode    = params[PARAM_GATEMODE].value > 0.0;
    bool    round       = params[PARAM_ROUND].value == 0.0;
    bool    clamp       = params[PARAM_CLAMP].value == 0.0;
    bool    bi          = params[PARAM_UNI_BI].value == 0.0;
    byte    intensity   = (byte)(floor(params[PARAM_INTENSITY].value));
    int     wrap        = floor(params[PARAM_WRAP].value);
    int     origin      = floor(params[PARAM_ORIGIN].value);
    float   smooth      = params[PARAM_SMOOTH].value;

    float in_origin     = inputs[IN_ORIGIN].value / 10.0;
    float in_intensity  = (inputs[IN_INTENSITY].value / 10.0) * 255.0;
    float in_wrap       = (inputs[IN_WRAP].value / 5.0) * 31.0;

    
    intensity = minb(intensity + (byte)in_intensity, 255.0);
    wrap = ::clampi(wrap + (int)in_wrap, -31, 31);

    // are we doing s&h?
    if (trig_hold.process(inputs[IN_HOLD].value))
        sample = inputs[IN_NEWS].value;

    // read the news, or if s&h is active just the held sample
    float news = (inputs[IN_HOLD].active) ? sample : inputs[IN_NEWS].value;

    // if round switch is down, round off the  input signal to an integer
    if (round)
        news = ceil(news);

    // wrap the bits around, e.g. wrap = 2, 1001 ->  0110 / wrap = -3,  1001 -> 0011
    uint32_t bits = *(reinterpret_cast<uint32_t *>(&news));
    if (wrap > 0)
        bits = (bits << wrap) | (bits >> (32 - wrap));
    else if (wrap < 0) {
        wrap = -wrap;
        bits = (bits >> wrap) | (bits << (32 - wrap));
    }
    
    news = *((float *)&bits);

    // extract the key out the bits which represent the input signal
    uint32_t key = *(reinterpret_cast<uint32_t *>(&news));

    // reset grid
    for (int i = 0; i < GSIZE; ++i)
        grid[i] = 0;

    // determine origin
    /*origin = min(origin + floor(in_origin * GSIZE), GSIZE );*/ // v0.6 breakage
    origin = min(origin + (int)floor(in_origin * GSIZE), GSIZE );
    int cy = origin / GWIDTH,
        cx = origin % GWIDTH;

    // extract N-E-W-S steps
    int nort = (key >> 24) & 0xFF,
        east = (key >> 16) & 0xFF,
        sout = (key >>  8) & 0xFF,
        west = (key      ) & 0xFF;

    // begin plotting, or 'the walk'
    int w = 0,
       ic = (mode ? 1 : DIR_BIT_SIZE),
     cond = 0;
    while (w++ < ic) {
        cond = mode ? (nort) : (((nort >> w) & 1) == 1);
        while (cond-- > 0) {
            cy = (cy - 1) >= 0 ? cy - 1 : GHEIGHT - 1;
            set(cx, cy, gatemode);
        }
        cond = (mode ? (east) : (((east >> w) & 1) == 1));
        while (cond-- > 0) {
            cx = (cx + 1) < GWIDTH ? cx + 1 : 0;
            set(cx, cy, gatemode);
        }
        cond = (mode ? (sout) : (((sout >> w) & 1) == 1));
        while (cond-- > 0) {
            cy = (cy + 1) < GHEIGHT ? cy + 1 : 0;
            set(cx, cy, gatemode);
        }
        cond = (mode ? (west) : (((west >> w) & 1) == 1));
        while (cond-- > 0) {
            cx = (cx - 1) >= 0 ? cx - 1 : GWIDTH - 1;
            set(cx, cy, gatemode);
        }
    }

    // output
    for (int i = 0; i < GSIZE; ++i) {
        byte r = grid[i] * intensity;
        if (clamp && ((int)grid[i] * (int)intensity) > 0xFF)
            r = 0xFF;

        float v = gatemode
                  ? (grid[i] ? 1 : 0)
                  : ((byte)r / 255.0);

        buffer[i] = slew(buffer[i], v, smooth, 0.1, 100000.0, 0.5);

        outputs[OUT_CELL + i].value = 10.0 * buffer[i] - (bi ? 5.0 : 0.0);
        lights[LIGHT_GRID + i].setBrightness(buffer[i] * 0.9);
    }
}

//TODO: move to common
template <typename _BASE>
struct CellLight : _BASE {
    CellLight()
    {
        this->box.size = mm2px(Vec(LIGHT_SIZE, LIGHT_SIZE));
    }
};

struct WidgetNews : ModuleWidget {
    WidgetNews(ModuleNews *module);
};

WidgetNews::WidgetNews(ModuleNews *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/NEWS.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));


    addInput(Port::create<PJ301MPort>(Vec(9 , 30), Port::INPUT, module, ModuleNews::IN_HOLD));
    addInput(Port::create<PJ301MPort>(Vec(9 , 65), Port::INPUT, module, ModuleNews::IN_NEWS));
    
    addInput(Port::create<PJ301MPort>(Vec(39 , 65), Port::INPUT, module, ModuleNews::IN_ORIGIN));
    addParam(ParamWidget::create<TinyKnob>(Vec(41.6, 32.5), module, ModuleNews::PARAM_ORIGIN, 0.0, GSIZE, GMID));
    addInput(Port::create<PJ301MPort>(Vec(69 , 65), Port::INPUT, module, ModuleNews::IN_INTENSITY));
    addParam(ParamWidget::create<TinyKnob>(Vec(71.1 , 32.5), module, ModuleNews::PARAM_INTENSITY, 1.0, 256.0, 1.0));
    addInput(Port::create<PJ301MPort>(Vec(99, 65), Port::INPUT, module, ModuleNews::IN_WRAP));
    addParam(ParamWidget::create<TinyKnob>(Vec(101, 32.5), module, ModuleNews::PARAM_WRAP, -31.0, 32.0, 0.0));

    const float out_ytop = 92.5;
    for (int y = 0; y < GHEIGHT; ++y)
        for (int x = 0; x < GWIDTH; ++x) {
            int i = x + y * GWIDTH;
            addChild(ModuleLightWidget::create<CellLight<GreenLight>>(Vec(7 + x * 30 - 0.2, out_ytop + y * 30 - 0.1), module, ModuleNews::LIGHT_GRID + i));
            addOutput(Port::create<PJ301MPort>(Vec(7 + x * 30 + 2, out_ytop + y * 30 + 2), Port::OUTPUT, module, ModuleNews::OUT_CELL + i));
        }

    const float bottom_row = 345;
    addParam(ParamWidget::create<CKSS>(Vec(5        , bottom_row), module, ModuleNews::PARAM_UNI_BI, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<CKSS>(Vec(25       , bottom_row), module, ModuleNews::PARAM_MODE, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<CKSS>(Vec(45       , bottom_row), module, ModuleNews::PARAM_GATEMODE, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<CKSS>(Vec(65       , bottom_row), module, ModuleNews::PARAM_ROUND, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<CKSS>(Vec(85       , bottom_row), module, ModuleNews::PARAM_CLAMP, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<TinyKnob>(Vec(110  , bottom_row), module, ModuleNews::PARAM_SMOOTH, 0.0, 1.0, 0.0));
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, News) {
   Model *modelNews = Model::create<ModuleNews, WidgetNews>(
      TOSTRING(SLUG), "NEWS", "NEWS", SEQUENCER_TAG);
   return modelNews;
}
