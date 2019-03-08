
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _DG
#include "daveguide.h"

/**
 */
struct DGModule : Module
{
public:
    DGModule();
    /**
     *
     *
     * Overrides of Module functions
     */
    void step() override;

    Daveguide<WidgetComposite> dave;
private:
};

DGModule::DGModule()
    : Module(dave.NUM_PARAMS,
    dave.NUM_INPUTS,
    dave.NUM_OUTPUTS,
    dave.NUM_LIGHTS),
    dave(this)
{
}

void DGModule::step()
{
    dave.step();
}

////////////////////
// module widget
////////////////////

struct DGWidget : ModuleWidget
{
    DGWidget(DGModule *);

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


private:
    DGModule* const module;
};




/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
DGWidget::DGWidget(DGModule *module) :
    ModuleWidget(module),
    module(module)
{
    box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/blank_panel.svg")));
        addChild(panel);
    }

    addLabel(Vec(35, 20), "Daveguide");

    addInput(createInputCentered<PJ301MPort>(
        Vec(40, 340),
        module,
        Daveguide<WidgetComposite>::AUDIO_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(120, 340),
        module,
        Daveguide<WidgetComposite>::AUDIO_OUTPUT));


    const float labelDeltaY = 25;
    const float gainX = 40;
    const float offsetX = 114;
    const float labelDeltaX = -20;
    const float y = 100;
    const float y2 = y + 70;

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(gainX, y),
        module, Daveguide<WidgetComposite>::OCTAVE_PARAM, -5, 5, 0));
    addLabel(Vec(gainX + labelDeltaX, y + labelDeltaY), "octave");

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(offsetX, y),
        module, Daveguide<WidgetComposite>::TUNE_PARAM, -5, 5, 0));
    addLabel(Vec(offsetX + labelDeltaX, y + labelDeltaY), "tune");

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(gainX, y2),
        module, Daveguide<WidgetComposite>::DECAY_PARAM, -5, 5, 0));
    addLabel(Vec(gainX + labelDeltaX, y2 + labelDeltaY), "decay");

    addParam(createParamCentered<Rogan1PSBlue>(
        Vec(offsetX, y2),
        module, Daveguide<WidgetComposite>::FC_PARAM, -5, 5, 0));
    addLabel(Vec(offsetX + labelDeltaX, y2 + labelDeltaY), "filter");




    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, DG) {
   Model *modelDGModule = Model::create<DGModule,
                                        DGWidget>("Squinky Labs",
                                                  "squinkylabs-dvg",
                                                  "dg", EFFECT_TAG, OSCILLATOR_TAG, WAVESHAPER_TAG);
   return modelDGModule;
}
#endif

