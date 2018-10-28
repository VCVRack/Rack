
#include <sstream>
#include "Squinky.hpp"
#ifdef _CPU_HOG

#include "WidgetComposite.h"

#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"


/** The following block of constants control what
 * this plugin does. Change them and re-build
 */
static const int numLoadThreads = 1;
static const int drawMillisecondSleep = 0;

static std::atomic<bool> drawIsSleeping;

/**
 * Implementation class for BootyModule
 */
struct CPU_HogModule : Module
{
    CPU_HogModule();
    ~CPU_HogModule();

    /**
     * Overrides of Module functions
     */
    void step() override;

    int stepsWhileDrawing = 0;
private:
    typedef float T;
    std::vector< std::shared_ptr<ThreadClient> > threads;
};

class PServer : public ThreadServer
{
public:
    PServer(std::shared_ptr<ThreadSharedState> state)
        : ThreadServer(state)
    {

    }
    virtual void threadFunction() override;

    ~PServer()
    {
    }
private:
    bool didRun = false;
    double dummy = 0;
};

void PServer::threadFunction()
{
    sharedState->serverRunning = true;
    for (bool done = false; !done; ) {
        if (sharedState->serverStopRequested.load()) {
            done = true;
        } else {
          // now kill a lot of time
            for (int i = 0; i < 10000; ++i) {
                dummy += std::log(rand()) * std::sin(rand());
            }

        }
    }

    thread->detach();
    sharedState->serverRunning = false;
}

CPU_HogModule::CPU_HogModule() : Module(0, 0, 0, 0)
{
    for (int i = 0; i < numLoadThreads; ++i) {
        std::shared_ptr<ThreadSharedState> state = std::make_shared<ThreadSharedState>();
        std::unique_ptr<ThreadServer> server(new PServer(state));
        threads.push_back(
            std::make_shared<ThreadClient>(
            state,
            std::move(server)));
    }

    // TODO: can we assume onSampleRateChange() gets called first, so this is unnecessary?
    onSampleRateChange();
}

CPU_HogModule::~CPU_HogModule()
{
    threads.resize(0);
}


void CPU_HogModule::step()
{
    if (drawIsSleeping) {
        stepsWhileDrawing++;
    }
}

////////////////////
// module widget
////////////////////

struct CPU_HogWidget : ModuleWidget
{
    CPU_HogWidget(CPU_HogModule *);
    void draw(NVGcontext *vg) override
    {
        const CPU_HogModule* pMod = static_cast<const CPU_HogModule*>(module);
        std::stringstream s;
        s << pMod->stepsWhileDrawing;
        steps->text = s.str();

        ModuleWidget::draw(vg);
        if (drawMillisecondSleep) {
            drawIsSleeping = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(drawMillisecondSleep));
            drawIsSleeping = false;
        }
    }
    Label* steps;
};

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
CPU_HogWidget::CPU_HogWidget(CPU_HogModule *module) : ModuleWidget(module)
{
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/cpu_hog_panel.svg")));
        addChild(panel);
    }

    Label* label = new Label();
    label->box.pos = Vec(10, 140);
    label->text = "SleepSteps";
    label->color = COLOR_BLACK;
    addChild(label);

    steps = new Label();
    steps->box.pos = Vec(10, 180);
    steps->text = "";
    steps->color = COLOR_BLACK;
    addChild(steps);

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
RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, CPU_hog) {
   Model *modelCPU_HogModule = Model::create<CPU_HogModule, CPU_HogWidget>("Squinky Labs",
                                                                           "squinkylabs-cpuhog",
                                                                           "CPU Hog", EFFECT_TAG);
   return modelCPU_HogModule;
}
#endif

