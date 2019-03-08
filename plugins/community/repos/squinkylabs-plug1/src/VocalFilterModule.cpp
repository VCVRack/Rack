
#include "Squinky.hpp"

#ifdef _FORMANTS
#include "WidgetComposite.h"
#include "VocalFilter.h"
#include "ctrl/SqMenuItem.h"

using Comp = VocalFilter<WidgetComposite>;
/**
 * Implementation class for VocalWidget
 */
struct VocalFilterModule : Module
{

    VocalFilterModule();

    /**
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    Comp vocalFilter;
private:
    typedef float T;
};

#ifdef __V1
VocalFilterModule::VocalFilterModule() :
    vocalFilter(this)
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    onSampleRateChange();
    vocalFilter.init();
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
}
#else
VocalFilterModule::VocalFilterModule() : 
    Module(vocalFilter.NUM_PARAMS, vocalFilter.NUM_INPUTS, vocalFilter.NUM_OUTPUTS, vocalFilter.NUM_LIGHTS),
    vocalFilter(this)
{
    onSampleRateChange();
    vocalFilter.init();
}

#endif

void VocalFilterModule::onSampleRateChange()
{
    T rate = SqHelper::engineGetSampleRate();
    vocalFilter.setSampleRate(rate);
}

void VocalFilterModule::step()
{
    vocalFilter.step();
}

////////////////////
// module widget
////////////////////

struct VocalFilterWidget : ModuleWidget
{
    VocalFilterWidget(VocalFilterModule *);
#ifndef __V1
    Menu* createContextMenu() override;
#endif
    void addVowelLabels();
    void addModelKnob(std::shared_ptr<IComposite>, VocalFilterModule *module, float x, float y);
};

#ifndef __V1
inline Menu* VocalFilterWidget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();
    ManualMenuItem* manual = new ManualMenuItem(
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/formants.md");
    theMenu->addChild(manual);
    return theMenu;
}
#endif
void VocalFilterWidget::addVowelLabels()
{
    const float ledX = 20;
    const float ledDx = 26;
    const float ledY = 43;

    const float vOffsetX = -8;
    const float vOffsetY = 14;
    for (int i = 0; i < 5; ++i) {
        VocalFilter<WidgetComposite>::LightIds id = (VocalFilter<WidgetComposite>::LightIds) i;
        std::string ltext;
        switch (id) {
            case VocalFilter<WidgetComposite>::LED_A:
                ltext = "A";
                break;
            case VocalFilter<WidgetComposite>::LED_E:
                ltext = "E";
                break;
            case VocalFilter<WidgetComposite>::LED_I:
                ltext = "I";
                break;
            case VocalFilter<WidgetComposite>::LED_O:
                ltext = "O";
                break;
            case VocalFilter<WidgetComposite>::LED_U:
                ltext = "U";
                break;
            default:
                assert(false);
        }
        Label * label = nullptr; label = new Label();
        label->text = ltext;
        label->color = SqHelper::COLOR_BLACK;
        label->box.pos = Vec(ledX + vOffsetX + i * ledDx, ledY + vOffsetY);

        addChild(label);

//addChild(createLight<MediumLight<RedLight>>(Vec(41, 59), module, MyModule::BLINK_LIGHT));
	
        addChild(createLight<MediumLight<GreenLight>>(
            Vec(ledX + i * ledDx, ledY), module, id));
    }
}

void VocalFilterWidget::addModelKnob(std::shared_ptr<IComposite> icomp, VocalFilterModule *module, float x, float y)
{
   // 5 pos vocal model
   // 0(bass)  1(tenor) 2(countertenor) 3(alto)  4(soprano)
    Label* label = new Label();
    label->box.pos = Vec(x - 18, y + 24);
    label->text = "B";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);

    label = new Label();
    label->box.pos = Vec(x - 20, y + 0);
    label->text = "T";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);

    label = new Label();
    label->box.pos = Vec(x - 2, y - 20);
    label->text = "CT";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);

    label = new Label();
    label->box.pos = Vec(x + 30, y + 0);
    label->text = "A";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);

    label = new Label();
    label->box.pos = Vec(x + 23, y + 24);
    label->text = "S";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);

    addParam(SqHelper::createParam<RoundBlackSnapKnob>(
        icomp,
        Vec(x - .5, y),
        module,
        Comp::FILTER_MODEL_SELECT_PARAM));
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
VocalFilterWidget::VocalFilterWidget(VocalFilterModule *module) : ModuleWidget(module)
{
    box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(SqHelper::assetPlugin(plugin, "res/formants_panel.svg")));
        addChild(panel);
    }
    std::shared_ptr<IComposite> icomp = Comp::getDescription();

    addVowelLabels();

    const float mid = 70;               // the middle area, with the four main knobs
    const float rightOffsetY = 40;      // right knobs drop this amount
    const float row2L = mid + 20;       //the first row of knobs
    const float row2R = row2L + rightOffsetY;
    const float row3L = row2L + rightOffsetY * 2;
    const float row3R = row3L + rightOffsetY;

    const float col1 = 10;              // the left hand strip of inputs and atternuverters
    const float col2 = 50;              // left column of big knobs
    const float col3 = 100;
    const float col4 = 146;             // inputs and attv on right

    const float labelOffset = -18;      // height of label above knob
    const float dyUp = -14;             // vertical space between input and atten
    const float dyDn = 30;

    const float trimDyL = 1;            // move atten down to match knob;
    const float trimDyR = 22;           // move atten down to match knob;

    const float trimDx = 3;             // move to the right to match input
    Label* label;

    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(col2, row2L),
        module, Comp::FILTER_VOWEL_PARAM));

    addInput(createInput<PJ301MPort>(
        Vec(col1, row2L + dyDn),
        module, Comp::FILTER_VOWEL_CV_INPUT));

    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(col1 + trimDx, row2L + trimDyL),
        module,Comp::FILTER_VOWEL_TRIM_PARAM));

    // Fc
    label = new Label();
    label->box.pos = Vec(col3, row2R + labelOffset);
    label->text = "Fc";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);

    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(col3, row2R), module, Comp::FILTER_FC_PARAM));

    addInput(createInput<PJ301MPort>(
        Vec(col4, row2R + dyUp),
        module, Comp::FILTER_FC_CV_INPUT));

    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(col4 + trimDx, row2R + trimDyR),
        module, Comp::FILTER_FC_TRIM_PARAM));

   // Q
    label = new Label();
    label->box.pos = Vec(col2, row3L + labelOffset);
    label->text = "Q";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);

    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(col2, row3L),
        module, Comp::FILTER_Q_PARAM));

    addInput(createInput<PJ301MPort>(
        Vec(col1, row3L + dyDn),
        module, Comp::FILTER_Q_CV_INPUT));

    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(col1 + trimDx, row3L + trimDyL),
        module, Comp::FILTER_Q_TRIM_PARAM));

   // Brightness
    label = new Label();
    label->box.pos = Vec(col3, row3R + labelOffset);
    label->text = "Brite";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);
    addParam(SqHelper::createParam<Rogan1PSBlue>(
        icomp,
        Vec(col3, row3R),
        module, Comp::FILTER_BRIGHTNESS_PARAM));

    addInput(createInput<PJ301MPort>(
        Vec(col4, row3R + dyUp),
        module, Comp::FILTER_BRIGHTNESS_INPUT));
    addParam(SqHelper::createParam<Trimpot>(
        icomp,
        Vec(col4 + trimDx, row3R + trimDyR),
        module, Comp::FILTER_BRIGHTNESS_TRIM_PARAM));

    addModelKnob(icomp, module, 71, 274);

    // I.O on row bottom
    const float AudioInputX = 10.0;
    const float outputX = 140.0;
    const float iOrow = 312;
    const float ioLabelOffset = -19;

    label = new Label();
    label->box.pos = Vec(outputX - 6, iOrow + ioLabelOffset);
    label->text = "Out";
    label->color = SqHelper::COLOR_WHITE;
    addChild(label);
    label = new Label();
    label->box.pos = Vec(AudioInputX - 1, iOrow + ioLabelOffset);
    label->text = "In";
    label->color = SqHelper::COLOR_BLACK;
    addChild(label);

    addInput(createInput<PJ301MPort>(
        Vec(AudioInputX, iOrow),
        module, Comp::AUDIO_INPUT));
    addOutput(createOutput<PJ301MPort>(
        Vec(outputX, iOrow),
        module,
        Comp::AUDIO_OUTPUT));

    /*************************************************
     *  screws
     */
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, VocalFilter) {
   Model *modelVocalFilterModule = Model::create<VocalFilterModule, VocalFilterWidget>("Squinky Labs",
                                                                                       "squinkylabs-vocalfilter",
                                                                                       "Formants: Vocal Filter", EFFECT_TAG, FILTER_TAG);
   return modelVocalFilterModule;
}

#endif
