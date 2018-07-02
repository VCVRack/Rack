
#include "Squinky.hpp"
#include "WidgetComposite.h"
#include "ThreadPriority.h"


struct ThreadBoostModule : Module
{
    enum ParamIds
    {
        THREAD_BOOST_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {

        NUM_INPUTS
    };
    enum OutputIds
    {

        NUM_OUTPUTS
    };
    enum LightIds
    {
        NORMAL_LIGHT,
        BOOSTED_LIGHT,
        REALTIME_LIGHT,
        ERROR_LIGHT,
        NUM_LIGHTS
    };

    ThreadBoostModule();

    /**
     * Overrides of Module functions
     */
    void step() override;

private:
    int boostState = 0;
    void lightOnly(LightIds l)
    {
        for (int i = NORMAL_LIGHT; i < NUM_LIGHTS; ++i) {
            bool b = (i == l);
            lights[i].value = b;
        }
    }

};

ThreadBoostModule::ThreadBoostModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
{
}

void ThreadBoostModule::step()
{
    float x = params[THREAD_BOOST_PARAM].value + .5f;
    int i = std::floor(x);
    if (i != boostState) {
        switch (i) {
            case 0:
                ThreadPriority::restore();
                lightOnly(NORMAL_LIGHT);
                break;
            case 1:
            {
                bool b = ThreadPriority::boostNormal();
                if (b) {
                    lightOnly(BOOSTED_LIGHT);
                } else {
                    lightOnly(ERROR_LIGHT);
                }
                break;
            }
            case 2:
            {
                bool b = ThreadPriority::boostRealtime();
                if (b) {
                    lightOnly(REALTIME_LIGHT);
                } else {
                    lightOnly(ERROR_LIGHT);
                }
                break;
            }
        }
        boostState = i;
    }
}

////////////////////
// module widget
////////////////////

struct ThreadBoostWidget : ModuleWidget
{
    ThreadBoostWidget(ThreadBoostModule *);
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
ThreadBoostWidget::ThreadBoostWidget(ThreadBoostModule *module)
    : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/thread_booster_panel.svg")));
        addChild(panel);
    }

    addParam(ParamWidget::create<NKK>(
        Vec(30, 140), module, ThreadBoostModule::THREAD_BOOST_PARAM, 0.0f, 2.0f, 0.0f));

    const int ledX = 10;
    const int labelX = 16;
    const int ledY = 200;
    const int labelY = ledY - 5;
    const int deltaY = 30;
    Label* label;
    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY), module, ThreadBoostModule::NORMAL_LIGHT));
    label = new Label();
    label->box.pos = Vec(labelX, labelY);
    label->text = "Normal";
    label->color = COLOR_BLACK;
    addChild(label);

    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY + deltaY), module, ThreadBoostModule::BOOSTED_LIGHT));
    label = new Label();
    label->box.pos = Vec(labelX, labelY + deltaY);
    label->text = "Boost";
    label->color = COLOR_BLACK;
    addChild(label);

    addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(
        Vec(ledX, ledY + 2 * deltaY), module, ThreadBoostModule::REALTIME_LIGHT));
    label = new Label();
    label->box.pos = Vec(labelX, labelY + 2 * deltaY);
    label->text = "Real-time";
    label->color = COLOR_BLACK;
    addChild(label);

    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(
        Vec(ledX, ledY + 3 * deltaY), module, ThreadBoostModule::ERROR_LIGHT));
    label = new Label();
    label->box.pos = Vec(labelX, labelY + 3 * deltaY);
    label->text = "Error";
    label->color = COLOR_BLACK;
    addChild(label);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, ThreadBoost) {
   Model *modelThreadBoostModule = Model::create<ThreadBoostModule, ThreadBoostWidget>("Squinky Labs",
                                                                                       "squinkylabs-booster",
                                                                                       "Thread Booster", UTILITY_TAG);
   return modelThreadBoostModule;
}

