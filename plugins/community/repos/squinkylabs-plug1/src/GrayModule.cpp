
#include "Squinky.hpp"
#include "WidgetComposite.h"

#ifdef _GRAY
#include "Gray.h"
#include "ctrl/SqMenuItem.h"

using Comp = Gray<WidgetComposite>;

/**
 */
struct GrayModule : Module
{
public:
    GrayModule();
    /**
     *
     *
     * Overrides of Module functions
     */
    void step() override;

    std::shared_ptr<Comp> gray;
private:
};

#ifdef __V1
GrayModule::GrayModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);

    //wait until after config to allocate this guy.
    gray = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this);
}
#else
GrayModule::GrayModule()
    : Module(Comp::NUM_PARAMS,
    Comp::NUM_INPUTS,
    Comp::NUM_OUTPUTS,
    Comp::NUM_LIGHTS),
    gray(std::make_shared<Comp>(this))
{
}
#endif

void GrayModule::step()
{
    gray->step();
}

////////////////////
// module widget
////////////////////

struct GrayWidget : ModuleWidget
{
    GrayWidget(GrayModule *);

#ifdef __V1
    void appendContextMenu(Menu *menu) override;
#else
    Menu* createContextMenu() override;
#endif

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

private:
    void addBits(GrayModule *module);
};

const float jackCol = 99.5;
const float ledCol = 69;
const float vertSpace = 31;  // 31.4
const float firstBitY = 64;

inline void GrayWidget::addBits(GrayModule *module)
{
    for (int i = 0; i < 8; ++i) {
        const Vec v(jackCol, firstBitY + i * vertSpace);
        addOutput(createOutputCentered<PJ301MPort>(
            v,
            module,
            Gray<WidgetComposite>::OUTPUT_0 + i));
        addChild(createLight<MediumLight<GreenLight>>(
            Vec(ledCol, firstBitY + i * vertSpace - 6),
            module,
            Gray<WidgetComposite>::LIGHT_0 + i));
    }
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
#ifdef __V1
GrayWidget::GrayWidget(GrayModule *module)
{
    setModule(module);
#else
GrayWidget::GrayWidget(GrayModule *module) :
    ModuleWidget(module)
{
#endif
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(SqHelper::assetPlugin(plugin, "res/gray.svg")));
        addChild(panel);
    }

    addBits(module);
    addInput(createInputCentered<PJ301MPort>(
        Vec(22, 339),
        module,
        Gray<WidgetComposite>::INPUT_CLOCK));
    addLabel(Vec(0, 310), "Clock");

    addParam(SqHelper::createParamCentered<CKSS>(
        icomp,
        Vec(71, 33),
        module,
        Gray<WidgetComposite>::PARAM_CODE));
    addLabel(Vec(2, 27), "Balanced");

    addOutput(createOutputCentered<PJ301MPort>(
        Vec(100, 339),
        module,
        Gray<WidgetComposite>::OUTPUT_MIXED));
    addLabel(Vec(82, 310), "Mix", SqHelper::COLOR_WHITE);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

#ifdef __V1
void GrayWidget::appendContextMenu(Menu* theMenu) 
{
    ManualMenuItem* manual = new ManualMenuItem("https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/gray-code.md");
    theMenu->addChild(manual);   
}
#else
inline Menu* GrayWidget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();
    ManualMenuItem* manual = new ManualMenuItem(
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/gray-code.md");
    theMenu->addChild(manual);
    return theMenu;
}
#endif

RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, Gray) {
   Model *modelGrayModule = Model::create<GrayModule,
                                          GrayWidget>("Squinky Labs",
                                                      "squinkylabs-gry",
                                                      "Gray Code: Eclectic clock divider", CLOCK_MODULATOR_TAG, RANDOM_TAG);
   return modelGrayModule;
}

#endif
