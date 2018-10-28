


#include "Squinky.hpp"
#include "SQWidgets.h"
#include "WidgetComposite.h"

#include <sstream>

#ifdef _CHB
#include "CHB.h"

/**
 */
struct CHBModule : Module
{
public:
    CHBModule();
    /**
     *
     *
     * Overrides of Module functions
     */
    void step() override;

    CHB<WidgetComposite> chb;
private:
};

CHBModule::CHBModule()
    : Module(chb.NUM_PARAMS,
    chb.NUM_INPUTS,
    chb.NUM_OUTPUTS,
    chb.NUM_LIGHTS),
    chb(this)
{
}

void CHBModule::step()
{
    chb.step();
}

////////////////////
// module widget
////////////////////

struct CHBWidget : ModuleWidget
{
    friend struct CHBEconomyItem;
    CHBWidget(CHBModule *);

    /**
     * Helper to add a text label to this widget
     */
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addHarmonics(CHBModule *module);
    void addVCOKnobs(CHBModule *module);
    void addOtherKnobs(CHBModule *module);
    void addMisc(CHBModule *module);
    void addBottomJacks(CHBModule *module);
    void resetMe(CHBModule *module);
private:
    bool fake;
    const float defaultGainParam = .63;

    const int numHarmonics;
    CHBModule* const module;
    std::vector<ParamWidget* > harmonicParams;
    std::vector<float> harmonicParamMemory;
    ParamWidget* gainParam=nullptr;
};

/**
 * Global coordinate contstants
 */
const float colHarmonicsJacks = 21;
const float rowFirstHarmonicJackY = 47;
const float harmonicJackSpacing = 32;
const float harmonicTrimDeltax = 27.5;

// columns of knobs
const float col1 = 95;
const float col2 = 150;
const float col3 = 214;

// rows of knobs
const float row1 = 75;
const float row2 = 131;
const float row3 = 228;
const float row4 = 287;
const float row5 = 332;

const float labelAboveKnob = 33;
const float labelAboveJack = 30;

inline void CHBWidget::addHarmonics(CHBModule *module)
{
    for (int i = 0; i < numHarmonics; ++i) {
        const float row = rowFirstHarmonicJackY + i * harmonicJackSpacing;
        addInput(createInputCentered<PJ301MPort>(
            Vec(colHarmonicsJacks, row),
            module,
            module->chb.H0_INPUT + i));

        const float defaultValue = (i == 0) ? 1 : 0;
        auto p = createParamCentered<Trimpot>(
            Vec(colHarmonicsJacks + harmonicTrimDeltax, row),
            module,
            module->chb.PARAM_H0 + i,
            0.0f, 1.0f, defaultValue);
        addParam(p);
        harmonicParams.push_back(p);
    }
}

inline void CHBWidget::addVCOKnobs(CHBModule *module)
{
    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(col2, row1),
        module,
        module->chb.PARAM_OCTAVE,
        -5.0f, 4.0f, 0.f));
    addLabel(Vec(col2 - 27, row1 - labelAboveKnob), "Octave");

    addParam(createParamCentered<Blue30Knob>(
        Vec(col3, row1),
        module,
        module->chb.PARAM_TUNE,
        -7.0f, 7.0f, 0));
    addLabel(Vec(col3 - 22, row1 - labelAboveKnob), "Tune");

    addParam(createParamCentered<Blue30Knob>(
        Vec(col2, row2),
        module,
        module->chb.PARAM_PITCH_MOD_TRIM,
        0, 1.0f, 0.0f));
    addLabel(Vec(col2 - 20, row2 - labelAboveKnob), "Mod");

    addParam(createParamCentered<Blue30Knob>(
        Vec(col3, row2),
        module,
        module->chb.PARAM_LINEAR_FM_TRIM,
        0, 1.0f, 0.0f));
    addLabel(Vec(col3 - 18, row2 - labelAboveKnob), "LFM");
}

inline void CHBWidget::addOtherKnobs(CHBModule *module)
{
    // gain

    gainParam = createParamCentered<Blue30Knob>(
        Vec(col1, row2),
        module,
        module->chb.PARAM_EXTGAIN,
        -5.0f, 5.0f, defaultGainParam);
    addParam(gainParam);

    addLabel(Vec(col1 - 22, row2 - labelAboveKnob), "Gain");

    addParam(createParamCentered<Trimpot>(
        Vec(col1, row2+30),
        module,
        module->chb.PARAM_EXTGAIN_TRIM,
        0, 1, 0));


    // slope
    addParam(createParamCentered<Blue30Knob>(
        Vec(185, 188),
        module,
        module->chb.PARAM_SLOPE,
        -5, 5, 5));
    addLabel(Vec(185 - 23, 188 - labelAboveKnob), "Slope");

    //even
    addParam(createParamCentered<Blue30Knob>(
        Vec(col2, row3),
        module,
        module->chb.PARAM_MAG_EVEN,
        0, 1, 1));
    addLabel(Vec(col2 - 21.5, row3 - labelAboveKnob), "Even");

    //odd
    addParam(createParamCentered<Blue30Knob>(
        Vec(col3, row3),
        module,
        module->chb.PARAM_MAG_ODD,
        0, 1, 1));
    addLabel(Vec(col3 - 20, row3 - labelAboveKnob), "Odd");
}

void CHBWidget::addMisc(CHBModule *module)
{
    auto sw = new SQPush();
    Vec pos(col1, row1);
    sw->center(pos);
    sw->onClick([this, module]() {
        this->resetMe(module);
    });

    addChild(sw);
    addLabel(Vec(col1 - 25, row1 - labelAboveKnob), "Preset");

    const float switchY = 219;
    addParam(createParamCentered<CKSS>(
        Vec(col1, switchY),
        module,
        module->chb.PARAM_FOLD,
        0.0f, 1.0f, 0.0f));
    auto l = addLabel(Vec(col1 - 18, 219 - 30), "Fold");
    l->fontSize = 11;
    l = addLabel(Vec(col1 - 17, 219 + 10), "Clip");
    l->fontSize = 11;

 //  Vec(col1, 165),
    addChild(createLightCentered<SmallLight<GreenRedLight>>(
        Vec(col1-16, switchY),
        module,
        module->chb.GAIN_GREEN_LIGHT));
}

static const char* labels[] = {
    "V/Oct",
    "Mod",
    "LFM",
    "Slope",
    "Ext In",
    "Gain",
    "EG",
    "Out",
};
static const int offsets[] = {
    -1,
    1,
    2,
    -1,
    -1,
    1,
    5,
    2
};

static const int ids[] = {
    CHB<WidgetComposite>::CV_INPUT,
    CHB<WidgetComposite>::PITCH_MOD_INPUT,
    CHB<WidgetComposite>::LINEAR_FM_INPUT,
    CHB<WidgetComposite>::SLOPE_INPUT,
    CHB<WidgetComposite>::AUDIO_INPUT,
    CHB<WidgetComposite>::GAIN_INPUT,
    CHB<WidgetComposite>::ENV_INPUT,
    CHB<WidgetComposite>::MIX_OUTPUT
};

void CHBWidget::addBottomJacks(CHBModule *module)
{
    const int deltaX = .5f + ((col3 - col1) / 3.0);
    for (int jackRow = 0; jackRow < 2; ++jackRow) {
        for (int jackCol = 0; jackCol < 4; ++jackCol) {
            const Vec pos(col1 + deltaX * jackCol,
                jackRow == 0 ? row4 : row5);
            const int index = jackCol + 4 * jackRow;

            auto color = COLOR_BLACK;
            if (index == 7) {
                color = COLOR_WHITE;
            }

            const int id = ids[index];
            if (index == 7) {
                addOutput(createOutputCentered<PJ301MPort>(
                    pos,
                    module,
                    id));
            } else {
                addInput(createInputCentered<PJ301MPort>(
                    pos,
                    module,
                    id));
            }
            auto l = addLabel(Vec(pos.x - 20 + offsets[index], pos.y - labelAboveJack),
                labels[index],
                color);
            l->fontSize = 11;
           // printf("def font size %f\n", l->fontSize);
        }
    }
}

void CHBWidget::resetMe(CHBModule *module)
{
    bool isOnlyFundamental = true;
    bool isAll = true;
    bool havePreset = !harmonicParamMemory.empty();
    const float val0 = harmonicParams[0]->value;
    if (val0 < .99) {
        isOnlyFundamental = false;
        isAll = false;
    }

    for (int i = 1; i < numHarmonics; ++i) {
        const float value = harmonicParams[i]->value;
        if (value < .9) {
            isAll = false;
        }

        if (value > .1) {
            isOnlyFundamental = false;
        }
    }

    if (!isOnlyFundamental && !isAll) {
        // take snapshot
        if (harmonicParamMemory.empty()) {
            harmonicParamMemory.resize(numHarmonics);
        }
        for (int i = 0; i < numHarmonics; ++i) {
            harmonicParamMemory[i] = harmonicParams[i]->value;
        }
    }

    // fundamental -> all
    if (isOnlyFundamental) {
        for (int i = 0; i < numHarmonics; ++i) {
            harmonicParams[i]->setValue(1);
        }
    }
    // all -> preset, if any
    else if (isAll && havePreset) {
        for (int i = 0; i < numHarmonics; ++i) {
            harmonicParams[i]->setValue(harmonicParamMemory[i]);
        }
    }
    // preset -> fund. if no preset all -> fund
    else {
        for (int i = 0; i < numHarmonics; ++i) {
            harmonicParams[i]->setValue((i == 0) ? 1 : 0);
        }
    }

    gainParam->setValue(defaultGainParam);
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
CHBWidget::CHBWidget(CHBModule *module) :
    ModuleWidget(module),
    numHarmonics(module->chb.numHarmonics),
    module(module)
{
    box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/chb_panel.svg")));
        addChild(panel);
    }

    addHarmonics(module);
    addVCOKnobs(module);
    addOtherKnobs(module);
    addMisc(module);
    addBottomJacks(module);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH))); 
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, CHB) {
   Model *modelCHBModule = Model::create<CHBModule,
                                         CHBWidget>("Squinky Labs",
                                                    "squinkylabs-CHB",
                                                    "Chebyshev: Waveshaper VCO", EFFECT_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG);
   return modelCHBModule;
}
#endif
