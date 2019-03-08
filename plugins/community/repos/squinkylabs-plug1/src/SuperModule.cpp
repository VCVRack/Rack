
#include "Squinky.hpp"

#ifdef _SUPER
#include "WidgetComposite.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqMenuItem.h"
#include "Super.h"
#include "ctrl/ToggleButton.h"
#include "ctrl/SemitoneDisplay.h"
#include "IMWidgets.hpp"

#include <sstream>

/**
 */
struct SuperModule : Module
{
public:
    SuperModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    Super<WidgetComposite> super;
};

void SuperModule::onSampleRateChange()
{
}

SuperModule::SuperModule()
    : Module(super.NUM_PARAMS,
    super.NUM_INPUTS,
    super.NUM_OUTPUTS,
    super.NUM_LIGHTS),
    super(this)
{
    onSampleRateChange();
    super.init();
}

void SuperModule::step()
{
    super.step();
}

////////////////////
// module widget
////////////////////

struct superWidget : ModuleWidget
{
    superWidget(SuperModule *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
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
        semitoneDisplay.step();
        ModuleWidget::step();
    }

    void addPitchKnobs(SuperModule *);
    void addOtherKnobs(SuperModule *);
    void addJacks(SuperModule *);
    // Menu* createContextMenu() override;

    SemitoneDisplay semitoneDisplay;
};


// inline Menu* superWidget::createContextMenu()
// {
//     Menu* theMenu = ModuleWidget::createContextMenu();
//     ManualMenuItem* manual = new ManualMenuItem(
//         "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/saws.md");
//     theMenu->addChild(manual);
//     return theMenu;
// }

const float col1 = 40;
const float col2 = 110;

const float row1 = 71;
const float row2 = 134;
const float row3 = 220;
const float row4 = 250;

const float jackRow1 = 290;
const float jackRow2 = 332;

const float labelOffsetBig = -40;
const float labelOffsetSmall = -32;

void superWidget::addPitchKnobs(SuperModule *)
{
    // Octave
    auto oct = createParamCentered<Rogan1PSBlue>(
        Vec(col1, row1), module, Super<WidgetComposite>::OCTAVE_PARAM, -5, 4, 0);
    oct->snap = true;
    oct->smooth = false;
    addParam(oct);
    Label* l = addLabel(
        Vec(col1 - 23, row1 + labelOffsetBig),
        "Oct");
    semitoneDisplay.setOctLabel(l, Super<WidgetComposite>::OCTAVE_PARAM);

    // Semi
    auto semi = createParamCentered<Rogan1PSBlue>(
        Vec(col2, row1), module, Super<WidgetComposite>::SEMI_PARAM, -11, 11, 0);
    semi->snap = true;
    semi->smooth = false;
    addParam(semi);
    l = addLabel(
        Vec(col2 - 27, row1 + labelOffsetBig),
        "Semi");
    semitoneDisplay.setSemiLabel(l, Super<WidgetComposite>::SEMI_PARAM);

    // Fine
    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(col1, row2), module, Super<WidgetComposite>::FINE_PARAM, -1, 1, 0));
    addLabel(
        Vec(col1 - 19,
        row2 + labelOffsetBig),
        "Fine");

    // FM
    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(col2, row2), module, Super<WidgetComposite>::FM_PARAM, 0, 1, 0));
    addLabel(
        Vec(col2 - 15, row2 + labelOffsetBig),
        "FM");
}

void superWidget::addOtherKnobs(SuperModule *)
{
    // Detune
    addParam(createParamCentered<Blue30Knob>(
        Vec(col1, row3), module, Super<WidgetComposite>::DETUNE_PARAM, -5, 5, 0));
    addLabel(
        Vec(col1 - 27, row3 + labelOffsetSmall),
        "Detune");

    addParam(createParamCentered<Trimpot>(
        Vec(col1, row4), module, Super<WidgetComposite>::DETUNE_TRIM_PARAM, -1, 1, 0));

    addParam(createParamCentered<Blue30Knob>(
        Vec(col2, row3), module, Super<WidgetComposite>::MIX_PARAM, -5, 5, 0));
    addLabel(
        Vec(col2 - 18, row3 + labelOffsetSmall),
        "Mix");
    addParam(createParamCentered<Trimpot>(
        Vec(col2, row4), module, Super<WidgetComposite>::MIX_TRIM_PARAM, -1, 1, 0));
}

const float jackX = 27;
const float jackDx = 33;
const float jackOffsetLabel = -30;
const float jackLabelPoints = 11;

void superWidget::addJacks(SuperModule *)
{
    Label* l = nullptr;
    // first row
    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX, jackRow1),
        module,
        Super<WidgetComposite>::DETUNE_INPUT));
    l = addLabel(
        Vec(jackX - 25, jackRow1 + jackOffsetLabel),
        "Detune");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 3 * jackDx, jackRow1),
        module,
        Super<WidgetComposite>::MIX_INPUT));
    l = addLabel(
        Vec(jackX + 3 * jackDx - 15, jackRow1 + jackOffsetLabel),
        "Mix");
    l->fontSize = jackLabelPoints;

    // second row
    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX, jackRow2),
        module,
        Super<WidgetComposite>::CV_INPUT));
    l = addLabel(
        Vec(jackX - 20, jackRow2 + jackOffsetLabel),
        "V/Oct");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 1 * jackDx, jackRow2),
        module,
        Super<WidgetComposite>::TRIGGER_INPUT));
    l = addLabel(
        Vec(jackX + 1 * jackDx - 17, jackRow2 + jackOffsetLabel),
        "Trig");
    l->fontSize = jackLabelPoints;

    addInput(createInputCentered<PJ301MPort>(
        Vec(jackX + 2 * jackDx, jackRow2),
        module,
        Super<WidgetComposite>::FM_INPUT));
    l = addLabel(
        Vec(jackX + 2 * jackDx - 14, jackRow2 + jackOffsetLabel), "FM");
    l->fontSize = jackLabelPoints;

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(jackX + 3 * jackDx, jackRow2),
        module,
        Super<WidgetComposite>::MAIN_OUTPUT));
    l = addLabel(
        Vec(jackX + 3 * jackDx - 18, jackRow2 + jackOffsetLabel),
        "Out", COLOR_WHITE);
    l->fontSize = jackLabelPoints;
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
superWidget::superWidget(SuperModule *module) :
    ModuleWidget(module),
    semitoneDisplay(module)
{
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/super_panel.svg")));
        addChild(panel);

        // Is this really needed?
        auto border = new PanelBorderWidget();
        border->box = box;
        addChild(border);
    }

    addPitchKnobs(module);
    addOtherKnobs(module);
    addJacks(module);

    // the "classic" switch
    ToggleButton* tog = createParamCentered<ToggleButton>(
        Vec(83, 164),
        module,
        Super<WidgetComposite>::CLEAN_PARAM,
        0.0f, 2, 0);
    tog->addSvg("res/clean-switch-01.svg");
    tog->addSvg("res/clean-switch-02.svg");
    tog->addSvg("res/clean-switch-03.svg");
    addParam(tog);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, Super) {
   Model *modelSuperModule = Model::create<SuperModule,
                                           superWidget>("Squinky Labs",
                                                        "squinkylabs-super",
                                                        "Saws: super saw VCO emulation", RANDOM_TAG);
   return modelSuperModule;
}

#endif
