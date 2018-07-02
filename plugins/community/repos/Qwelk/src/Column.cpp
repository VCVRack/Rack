#include "dsp/digital.hpp"
#include "qwelk.hpp"


#define CHANNELS 4

namespace rack_plugin_Qwelk {

struct ModuleColumn : Module {
    enum ParamIds {
        PARAM_MASTER,
        PARAM_AVG,
        PARAM_WEIGHTED,
        NUM_PARAMS
    };
    enum InputIds {
        IN_SIG,
        IN_UPSTREAM = IN_SIG + CHANNELS,
        NUM_INPUTS = IN_UPSTREAM + CHANNELS
    };
    enum OutputIds {
        OUT_SIDE,
        OUT_DOWNSTREAM = OUT_SIDE + CHANNELS,
        NUM_OUTPUTS = OUT_DOWNSTREAM + CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    bool allow_neg_weights = false;
    
    ModuleColumn() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void ModuleColumn::step()
{
    bool avg        = params[PARAM_AVG].value == 0.0;
    bool weighted   = params[PARAM_WEIGHTED].value == 0.0;

    float total     = 0;
    float on_count  = 0;

    for (int i = 0; i < CHANNELS; ++i) {
        float in_upstream   = inputs[IN_UPSTREAM + i].value;
        float in_sig        = inputs[IN_SIG + i].value;

        // just forward the input signal as the side stream,
        // used for chaining multiple Columns together
        outputs[OUT_SIDE + i].value = in_sig;
        
        if (inputs[IN_UPSTREAM + i].active) {
            if (weighted)
                on_count += allow_neg_weights ? in_upstream : fabs(in_upstream);
            else if (in_upstream != 0.0)
                on_count += 1;
        }
        
        if (!weighted && in_sig != 0.0)
            on_count += 1;

        float product = weighted ? (in_upstream * in_sig) : (in_upstream + in_sig);

        total += product;
        
        outputs[OUT_DOWNSTREAM + i].value = avg ? ((on_count != 0) ? (total / on_count) : 0) : (total);
    }
}

struct WidgetColumn : ModuleWidget {
    WidgetColumn(ModuleColumn *module);
    Menu *createContextMenu() override;
};

WidgetColumn::WidgetColumn(ModuleColumn *module) : ModuleWidget(module) {

    setPanel(SVG::load(assetPlugin(plugin, "res/Column.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    addParam(ParamWidget::create<CKSS>(Vec(3.5, 30), module, ModuleColumn::PARAM_AVG, 0.0, 1.0, 1.0));
    addParam(ParamWidget::create<CKSS>(Vec(42, 30), module, ModuleColumn::PARAM_WEIGHTED, 0.0, 1.0, 1.0));

    float x = 2.5, xstep = 15, ystep = 23.5;
    for (int i = 0; i < CHANNELS; ++i)
    {
        float y = 80 + i * 80;
        addInput(Port::create<PJ301MPort>(Vec(x + xstep, y - ystep), Port::INPUT, module, ModuleColumn::IN_UPSTREAM + i));
        addOutput(Port::create<PJ301MPort>(Vec(x + xstep*2, y), Port::OUTPUT, module, ModuleColumn::OUT_SIDE + i));
        addInput(Port::create<PJ301MPort>(Vec(x, y), Port::INPUT, module, ModuleColumn::IN_SIG + i));
        addOutput(Port::create<PJ301MPort>(Vec(x + xstep, y + ystep), Port::OUTPUT, module, ModuleColumn::OUT_DOWNSTREAM + i));
    }
}

struct MenuItemAllowNegWeights : MenuItem {
    ModuleColumn *col;
    void onAction(EventAction &e) override
    {
        col->allow_neg_weights ^= true;
    }
    void step () override
    {
        rightText = (col->allow_neg_weights) ? "✔" : "";
    }
};

Menu *WidgetColumn::createContextMenu()
{
    Menu *menu = ModuleWidget::createContextMenu();

    MenuLabel *spacer = new MenuLabel();
    menu->addChild(spacer);

    ModuleColumn *column = dynamic_cast<ModuleColumn *>(module);
    assert(column);

    MenuItemAllowNegWeights *item = new MenuItemAllowNegWeights();
    item->text = "Allow Negative Weights";
    item->col  = column;
    menu->addChild(item);

    return menu;
}

} // namespace rack_plugin_Qwelk

using namespace rack_plugin_Qwelk;

RACK_PLUGIN_MODEL_INIT(Qwelk, Column) {
   Model *modelColumn = Model::create<ModuleColumn, WidgetColumn>(
      TOSTRING(SLUG), "Column", "Column", MIXER_TAG);
   return modelColumn;
}
