#include "ctrl/SemitoneDisplay.h"
#include "Squinky.hpp"

#ifdef _CHB
#include "IComposite.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqMenuItem.h"
#include "WidgetComposite.h"
#include <sstream>

#include "CHB.h"
#include "IMWidgets.hpp"

using Comp = CHB<WidgetComposite>;

/**
 */
struct CHBModule : Module
{
public:
    CHBModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    CHB<WidgetComposite> chb;
private:
};

#ifdef __V1
CHBModule::CHBModule() : chb(this)
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
}
#else
CHBModule::CHBModule()
    : Module(chb.NUM_PARAMS,
    chb.NUM_INPUTS,
    chb.NUM_OUTPUTS,
    chb.NUM_LIGHTS),
    chb(this)
{
}
#endif

void CHBModule::step()
{
    chb.step();
}

void CHBModule::onSampleRateChange()
{
    chb.onSampleRateChange();
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
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void step() override
    {
        ModuleWidget::step();
        semitoneDisplay.step();
    }
private:
    void addHarmonics(CHBModule *module,  std::shared_ptr<IComposite>);
    void addRow1(CHBModule *module, std::shared_ptr<IComposite>);
    void addRow2(CHBModule *module, std::shared_ptr<IComposite>);
    void addRow3(CHBModule *module, std::shared_ptr<IComposite>);
    void addRow4(CHBModule *module, std::shared_ptr<IComposite>);

    void addBottomJacks(CHBModule *module);
    void resetMe(CHBModule *module);
#ifdef __V1
    void appendContextMenu(Menu *menu) override;
#else
    Menu* createContextMenu() override;
#endif
    
    // TODO: stil used?
    // This is the gain which when run throught all the lookup tables
    // gives a gain of 1.
    const float defaultGainParam = .63108f;

    int numHarmonics = 10;
    CHBModule* const module;
    std::vector<ParamWidget* > harmonicParams;
    std::vector<float> harmonicParamMemory;
    ParamWidget* gainParam = nullptr;

    SemitoneDisplay semitoneDisplay;
};

#ifdef __V1
inline void CHBWidget::appendContextMenu(Menu *menu) 
{
    ManualMenuItem* manual = new ManualMenuItem(
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/chebyshev.md");
    menu->addChild(manual);
}
#else
inline Menu* CHBWidget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();
    ManualMenuItem* manual = new ManualMenuItem(
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/chebyshev.md");
    theMenu->addChild(manual);
    return theMenu;
}
#endif


/**
 * Global coordinate constants
 */
const float colHarmonicsJacks = 21;
const float rowFirstHarmonicJackY = 47;
const float harmonicJackSpacing = 32;
const float harmonicTrimDeltax = 27.5;

// columns of knobs
const float col1 = 95;
const float col2 = 150;
const float col3 = 214;
const float col4 = 268;

// rows of knobs
const float row1 = 75;
const float row2 = 131;
const float row3 = 201;
const float row4 = 237;
const float row5 = 287;
const float row6 = 332;

const float labelAboveKnob = 33;
const float labelAboveJack = 30;

inline void CHBWidget::addHarmonics(CHBModule *module, std::shared_ptr<IComposite> icomp)
{
    for (int i = 0; i < numHarmonics; ++i) {
        const float row = rowFirstHarmonicJackY + i * harmonicJackSpacing;
        addInput(createInputCentered<PJ301MPort>(
            Vec(colHarmonicsJacks, row),
            module,
            module->chb.H0_INPUT + i));

       // const float defaultValue = (i == 0) ? 1 : 0;
        auto p = SqHelper::createParamCentered<Trimpot>(
            icomp,
            Vec(colHarmonicsJacks + harmonicTrimDeltax, row),
            module,
            Comp::PARAM_H0 + i);
        addParam(p);
        harmonicParams.push_back(p);
    }
}

void CHBWidget::addRow1(CHBModule *module, std::shared_ptr<IComposite> icomp)
{
    const float row = row1;

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col1, row),
        module,
        CHB<WidgetComposite>::PARAM_RISE));
    addLabel(Vec(col1 - 20, row - labelAboveKnob), "Rise");

    addParam(SqHelper::createParamCentered<Blue30SnapKnob>(
        icomp,
        Vec(col2, row),
        module,
        CHB<WidgetComposite>::PARAM_OCTAVE));
    semitoneDisplay.setOctLabel(
        addLabel(Vec(col2 - 22, row1 - labelAboveKnob), "Octave"),
        CHB<WidgetComposite>::PARAM_OCTAVE);

    addParam(SqHelper::createParamCentered<Blue30SnapKnob>(
        icomp,
        Vec(col3, row),
        module,
        CHB<WidgetComposite>::PARAM_SEMIS));
    semitoneDisplay.setSemiLabel(
        addLabel(Vec(col3 - 26, row - labelAboveKnob), "Semi"),
        CHB<WidgetComposite>::PARAM_SEMIS);

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col4, row1),
        module,
        CHB<WidgetComposite>::PARAM_TUNE));
    addLabel(Vec(col4 - 22, row1 - labelAboveKnob), "Tune");
}

void CHBWidget::addRow2(CHBModule *module,  std::shared_ptr<IComposite> icomp)
{
    const float row = row2;

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col1, row),
        module,
        CHB<WidgetComposite>::PARAM_FALL));
    addLabel(Vec(col1 - 18, row - labelAboveKnob), "Fall");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col3, row),
        module,
        CHB<WidgetComposite>::PARAM_PITCH_MOD_TRIM));
    addLabel(Vec(col3 - 20, row - labelAboveKnob), "Mod");

    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col4, row),
        module,
        CHB<WidgetComposite>::PARAM_LINEAR_FM_TRIM));
    addLabel(Vec(col4 - 20, row - labelAboveKnob), "LFM");

    addParam(SqHelper::createParamCentered<CKSS>(
        icomp,
        Vec(col2, row),
        module,
        CHB<WidgetComposite>::PARAM_FOLD));
    auto l = addLabel(Vec(col2 - 18, row - 30), "Fold");
    l->fontSize = 11;
    l = addLabel(Vec(col2 - 17, row + 10), "Clip");
    l->fontSize = 11;

    addChild(createLightCentered<SmallLight<GreenRedLight>>(
        Vec(col2 - 16, row),
        module,
        CHB<WidgetComposite>::GAIN_GREEN_LIGHT));
}

void CHBWidget::addRow3(CHBModule *module,  std::shared_ptr<IComposite> icomp)
{
    const float row = row3;

    gainParam = SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col1, row),
        module,
        CHB<WidgetComposite>::PARAM_EXTGAIN);
    addParam(gainParam);
    addLabel(Vec(col1 - 21, row - labelAboveKnob), "Gain");

    //even
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col2, row),
        module,
        CHB<WidgetComposite>::PARAM_MAG_EVEN));
    addLabel(Vec(col2 - 21.5, row - labelAboveKnob), "Even");

    // slope
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col3, row),
        module,
        CHB<WidgetComposite>::PARAM_SLOPE));
    addLabel(Vec(col3 - 23, row - labelAboveKnob), "Slope");

    //odd
    addParam(SqHelper::createParamCentered<Blue30Knob>(
        icomp,
        Vec(col4, row),
        module,
        CHB<WidgetComposite>::PARAM_MAG_ODD));
    addLabel(Vec(col4 - 19, row - labelAboveKnob), "Odd");

}

void CHBWidget::addRow4(CHBModule *module,  std::shared_ptr<IComposite> icomp)
{
    float row = row4;

    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(col1, row),
        module,
        CHB<WidgetComposite>::PARAM_EXTGAIN_TRIM));

    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(col2, row),
        module,
        CHB<WidgetComposite>::PARAM_EVEN_TRIM));

    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(col3, row),
        module,
        CHB<WidgetComposite>::PARAM_SLOPE_TRIM));

    addParam(SqHelper::createParamCentered<Trimpot>(
        icomp,
        Vec(col4, row),
        module,
        CHB<WidgetComposite>::PARAM_ODD_TRIM));
}

static const char* labels[] = {
    "V/Oct",
    "Mod",
    "LFM",
    "Even",
    "Slope",
    "Odd",
    "Ext In",
    "Gain",
    "EG",
    "Rise",
    "Fall",
    "Out",
    nullptr,
};
static const int offsets[] = {
    -1,
    2,
    2,
    0,
    0,      // slope
    2,      // odd
    -2,     // ext gain
    1,          // gain
    5,
    2,          // rise
    4,
    3
};

static const int ids[] = {
    // top row
    CHB<WidgetComposite>::CV_INPUT,
    CHB<WidgetComposite>::PITCH_MOD_INPUT,
    CHB<WidgetComposite>::LINEAR_FM_INPUT,
    CHB<WidgetComposite>::EVEN_INPUT,
    CHB<WidgetComposite>::SLOPE_INPUT,
    CHB<WidgetComposite>::ODD_INPUT,
    //bottom row
    CHB<WidgetComposite>::AUDIO_INPUT,
    CHB<WidgetComposite>::GAIN_INPUT,
    CHB<WidgetComposite>::ENV_INPUT,
    CHB<WidgetComposite>::RISE_INPUT,
    CHB<WidgetComposite>::FALL_INPUT,
    CHB<WidgetComposite>::MIX_OUTPUT
};

void CHBWidget::addBottomJacks(CHBModule *module)
{
    const float jackCol1 = 93;
    const int numCol = 6;
    const float deltaX = 36;
    for (int jackRow = 0; jackRow < 2; ++jackRow) {
        for (int jackCol = 0; jackCol < numCol; ++jackCol) {
            const Vec pos(jackCol1 + deltaX * jackCol,
                jackRow == 0 ? row5 : row6);
            const int index = jackCol + numCol * jackRow;

            auto color = SqHelper::COLOR_BLACK;
            if (index == 11) {
                color = SqHelper::COLOR_WHITE;
            }

            const int id = ids[index];
            if (index == 11) {
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
        }
    }
}

void CHBWidget::resetMe(CHBModule *module)
{
    bool isOnlyFundamental = true;
    bool isAll = true;
    bool havePreset = !harmonicParamMemory.empty();
//    const float val0 = harmonicParams[0]->value;
    const float val0 = SqHelper::getValue(harmonicParams[0]);
    if (val0 < .99) {
        isOnlyFundamental = false;
        isAll = false;
    }

    for (int i = 1; i < numHarmonics; ++i) {
        const float value = SqHelper::getValue(harmonicParams[i]);
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
            harmonicParamMemory[i] = SqHelper::getValue(harmonicParams[i]);
        }
    }

    // fundamental -> all
    if (isOnlyFundamental) {
        for (int i = 0; i < numHarmonics; ++i) {
            //harmonicParams[i]->setValue(1);
            SqHelper::setValue(harmonicParams[i], 1.f);
        }
    }
    // all -> preset, if any
    else if (isAll && havePreset) {
        for (int i = 0; i < numHarmonics; ++i) {
            SqHelper::setValue(harmonicParams[i], harmonicParamMemory[i]);
        }
    }
    // preset -> fund. if no preset all -> fund
    else {
        for (int i = 0; i < numHarmonics; ++i) {
            SqHelper::setValue(harmonicParams[i], (i == 0) ? 1 : 0);
        }
    }

    SqHelper::setValue(gainParam, defaultGainParam);
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
CHBWidget::CHBWidget(CHBModule *module) :
  //  numHarmonics(module->chb.numHarmonics),
    module(module),
    semitoneDisplay(module)
{
    printf("entering ctor of chb\n"); fflush(stdout);
    if (module) {
        numHarmonics = module->chb.numHarmonics;
    }
    setModule(module);
#else
CHBWidget::CHBWidget(CHBModule *module) :
    ModuleWidget(module),
    numHarmonics(module->chb.numHarmonics),
    module(module),
    semitoneDisplay(module)
{
#endif
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    box.size = Vec(20 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(SqHelper::assetPlugin(plugin, "res/chb_panel.svg")));
        addChild(panel);

        auto border = new PanelBorderWidget();
        border->box = box;
        addChild(border);
    }

    addHarmonics(module, icomp);
    addRow1(module, icomp);
    addRow2(module, icomp);
    addRow3(module, icomp);
    addRow4(module, icomp);

    auto sw = new SQPush(
        "res/preset-button-up.svg",
        "res/preset-button-down.svg");
    Vec pos(64, 360);
    sw->center(pos);
    sw->onClick([this, module]() {
        this->resetMe(module);
    });

    addChild(sw);
    addBottomJacks(module);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, CHB) {
#ifdef __V1
   Model *modelCHBModule = createModel<CHBModule, CHBWidget>(
      "cheby");
#else
   Model *modelCHBModule = Model::create<CHBModule,
                                         CHBWidget>("Squinky Labs",
                                                    "squinkylabs-CHB2",
                                                    "Chebyshev II: Waveshaper VCO", EFFECT_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG);
#endif
   return modelCHBModule;
}

#endif
