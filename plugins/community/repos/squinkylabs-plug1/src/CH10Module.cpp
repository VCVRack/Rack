#include "Squinky.hpp"

#ifdef _CH10
#include "ctrl/SqWidgets.h"
#include "WidgetComposite.h"
#include "CH10.h"
#include "ctrl/ToggleButton.h"

#include <sstream>

/**
 */
struct CH10Module : Module
{
public:
    CH10Module();
    /**
     *
     * Overrides of Module functions
     */
    void step() override;
    void onSampleRateChange() override;

    CH10<WidgetComposite> ch10;

private:

};

void CH10Module::onSampleRateChange()
{
}

CH10Module::CH10Module()
    : Module(CH10<WidgetComposite>::NUM_PARAMS,
    CH10<WidgetComposite>::NUM_INPUTS,
    CH10<WidgetComposite>::NUM_OUTPUTS,
    CH10<WidgetComposite>::NUM_LIGHTS),
    ch10(this)
{
    onSampleRateChange();
    ch10.init();
}

void CH10Module::step()
{
    ch10.step();
}

////////////////////
// module widget
////////////////////

struct CH10Widget : ModuleWidget
{
    CH10Widget(CH10Module *);

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }
    void makeA(CH10Module *);
    void makeB(CH10Module *);
    void makeAB(CH10Module *);
    void addSwitch(float x, float y, int id);
    void makeVCO(CH10Module*, int whichOne);
};

const static float gridSize = 28;
const static float gridCol1 = 140;
const static float gridRow1 = 300;

inline void CH10Widget::makeA(CH10Module *)
{
    for (int i = 0; i < 10; ++i) {
        const float x = gridCol1;
        const float y = gridRow1 - i * gridSize;
        addSwitch(x, y, CH10<Widget>::A0_PARAM + i);
    }
}

inline void CH10Widget::makeB(CH10Module *)
{
    for (int i = 0; i < 10; ++i) {
        const float x = gridCol1 + gridSize * (i + 1);
        const float y = gridRow1 + gridSize;
        addSwitch(x, y, CH10<Widget>::B0_PARAM + i);
    }
}

inline void CH10Widget::makeAB(CH10Module *)
{
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            float x = gridCol1 + gridSize * (col + 1);
            float y = gridRow1 - row * gridSize;
            int id = CH10<Widget>::A0B0_PARAM +
                col + row * 10;
            addSwitch(x, y, id);
        }
    }
}

inline void CH10Widget::addSwitch(float x, float y, int id)
{
    ToggleButton* tog = ParamWidget::create<ToggleButton>(
        Vec(x, y),
        module,
        id,
        0.0f, 1, 0);

    tog->addSvg("res/square-button-01.svg");
    tog->addSvg("res/square-button-02.svg");
    addParam(tog);
}

const float rowSpacing = 40;
const float vcoACol = 50;
const float vcoBCol = 90;
const float vcoOctRow = 60;
const float vcoSemiRow = vcoOctRow + rowSpacing;
const float vcoCVRow = vcoSemiRow + rowSpacing;

inline void CH10Widget::makeVCO(CH10Module* module, int whichVCO)
{
    const float x = whichVCO ? vcoBCol : vcoACol;
    addParam(createParamCentered<Blue30Knob>(
        Vec(x, vcoOctRow),
        module,
        CH10<WidgetComposite>::AOCTAVE_PARAM + whichVCO,
        -5.f, 4.f, 0.f));

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x, vcoSemiRow), module,
        CH10<WidgetComposite>::ASEMI_PARAM + whichVCO,
        -11.f, 11.0f, 0.f));

    addInput(createInputCentered<PJ301MPort>(
        Vec(x, vcoCVRow),
        module,
        CH10<WidgetComposite>::ACV_INPUT + whichVCO));


}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
CH10Widget::CH10Widget(CH10Module *module) : ModuleWidget(module)
{
    box.size = Vec(35 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/ch10_panel.svg")));
        addChild(panel);
    }

    makeA(module);
    makeB(module);
    makeAB(module);
    makeVCO(module, 0);
    makeVCO(module, 1);

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(70, 300),
        module,
        CH10<WidgetComposite>::MIXED_OUTPUT));

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, CH10) {
   Model *modelCH10Module = Model::create<CH10Module,
                                          CH10Widget>("Squinky Labs",
                                                      "squinkylabs-ch10",
                                                      "-- ch10 --", RANDOM_TAG);
   return modelCH10Module;
}

#endif
