
#include <sstream>
#include "Squinky.hpp"

#ifdef _KS
#include "WidgetComposite.h"
#include "KSComposite.h"

/**
 */
struct KSModule : Module
{
public:
    KSModule();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    KSComposite<WidgetComposite> composite;
private:

};

void KSModule::onSampleRateChange()
{
}

KSModule::KSModule()
    : Module(composite.NUM_PARAMS,
    composite.NUM_INPUTS,
    composite.NUM_OUTPUTS,
    composite.NUM_LIGHTS),
    composite(this)
{
    onSampleRateChange();
    composite.init();
}

void KSModule::step()
{
    composite.step();
}

////////////////////
// module widget
////////////////////

struct KCCompositeWidget : ModuleWidget
{
    KCCompositeWidget(KSModule *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addJacks(KSModule*);
    void addKnobs(KSModule*);
};

void KCCompositeWidget::addJacks(KSModule*)
{
    const float verticalShift = 0;
    const float col1 = 12;
    const float col2 = 46;
    const float col3 = 81;
    const float col4 = 115;
    const float outputLabelY = 300;

    using CCOMP = KSComposite<WidgetComposite>;

    addInput(Port::create<PJ301MPort>(
        Vec(col1, 273 + verticalShift),
        Port::INPUT,
        module,
        CCOMP::PITCH_INPUT));
    addLabel(Vec(9, 255 + verticalShift), "cv");

    addInput(Port::create<PJ301MPort>(
        Vec(col2, 273 + verticalShift),
        Port::INPUT,
        module,
        CCOMP::FM_INPUT));

    addLabel(Vec(43, 255 + verticalShift), "fm");

    addInput(Port::create<PJ301MPort>(
        Vec(col3, 273 + verticalShift),
        Port::INPUT,
        module,
        CCOMP::SYNC_INPUT));
    addLabel(Vec(72, 255 + verticalShift), "sync");

    addInput(Port::create<PJ301MPort>(
        Vec(col4, 273 + verticalShift),
        Port::INPUT,
        module,
        CCOMP::PW_INPUT));
    addLabel(Vec(107, 255 + verticalShift), "pwm");

    addOutput(Port::create<PJ301MPort>(
        Vec(col1, 317 + verticalShift),
        Port::OUTPUT,
        module,
        CCOMP::SIN_OUTPUT));
    addLabel(Vec(8, outputLabelY + verticalShift), "sin");

    addOutput(Port::create<PJ301MPort>(
        Vec(col2, 317 + verticalShift),
        Port::OUTPUT,
        module,
        CCOMP::TRI_OUTPUT));
    addLabel(Vec(44, outputLabelY + verticalShift), "tri");

    addOutput(Port::create<PJ301MPort>(
        Vec(col3, 317 + verticalShift),
        Port::OUTPUT,
        module,
        CCOMP::SAW_OUTPUT));
    addLabel(Vec(75, outputLabelY + verticalShift), "saw");

    addOutput(Port::create<PJ301MPort>(
        Vec(col4, 317 + verticalShift),
        Port::OUTPUT,
        module,
        CCOMP::SQR_OUTPUT));
    addLabel(Vec(111, outputLabelY + verticalShift), "sqr");
}


void KCCompositeWidget::addKnobs(KSModule*)
{

}
/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
KCCompositeWidget::KCCompositeWidget(KSModule *module) : ModuleWidget(module)
{
    box.size = Vec(15 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/ks_panel.svg")));
        addChild(panel);
    }

    addJacks(module);
    addKnobs(module);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, KS) {
   Model *modelKSModule = Model::create<KSModule,
                                        KCCompositeWidget>("Squinky Labs",
                                                           "squinkylabs-ks",
                                                           "kitchen sink", RANDOM_TAG);
   return modelKSModule;
}
#endif
