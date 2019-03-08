#include "Squinky.hpp"

#ifdef _EV3
#include "ctrl/WaveformSelector.h"
#include "ctrl/SqWidgets.h"
#include "ctrl/SqMenuItem.h"
#include "WidgetComposite.h"

#include "EV3.h"
#include <sstream>

struct EV3Module : Module
{
    EV3Module();
    void step() override;
    EV3<WidgetComposite> ev3;
};

EV3Module::EV3Module()
    : Module(ev3.NUM_PARAMS,
    ev3.NUM_INPUTS,
    ev3.NUM_OUTPUTS,
    ev3.NUM_LIGHTS),
    ev3(this)
{
}

void EV3Module::step()
{
    ev3.step();
}

/************************************************************/

class EV3PitchDisplay
{
public:
    EV3PitchDisplay(EV3Module * mod) : module(mod)
    {
    }
    void step();

    /**
     * Labels must be added in order
     */
    void addOctLabel(Label*);
    void addSemiLabel(Label*);

private:
    EV3Module * const module;

    std::vector<Label*> octLabels;
    std::vector<Label*> semiLabels;
    std::vector<float> semiX;
    int lastOctave[3] = {100, 100, 100};
    int lastSemi[3] = {100, 100, 100};
    bool lastPatched[3] = {false, false, false};
    void updateAbsolute(int);
    void updateInterval(int);
    bool shouldUseInterval(int osc);
};

void EV3PitchDisplay::step()
{
    bool atLeastOneChanged = false;
    const int delta = EV3<WidgetComposite>::OCTAVE2_PARAM - EV3<WidgetComposite>::OCTAVE1_PARAM;
    for (int i = 0; i < 3; ++i) {
        const int octaveParam = EV3<WidgetComposite>::OCTAVE1_PARAM + delta * i;
        const int semiParam = EV3<WidgetComposite>::SEMI1_PARAM + delta * i;
        const int inputId = EV3<WidgetComposite>::CV1_INPUT + i;
        const int oct = module->params[octaveParam].value;
        const int semi = module->params[semiParam].value;
        const bool patched = module->inputs[inputId].active;
        if (semi != lastSemi[i] ||
            oct != lastOctave[i] ||
            patched != lastPatched[i]) {

            atLeastOneChanged = true;
            lastSemi[i] = semi;

            lastOctave[i] = oct;
            lastPatched[i] = patched;
        }
    }
    if (atLeastOneChanged) {
        for (int i = 0; i < 3; ++i) {
            if (shouldUseInterval(i)) {
                updateInterval(i);
            } else {
                updateAbsolute(i);
            }
        }
    }
}

static const char* intervalNames[] = {
    "0",
    "m2nd",
    "2nd",
    "m3rd",
    "M3rd",
    "4th",
    "Dim5th",
    "5th",
    "m6th",
    "M6th",
    "m7th",
    "M7th",
    "oct"
};

const static int offsetNatural = 10;
const static int offsetAccidental = 7;

static const int pitchOffsets[] = {
    offsetNatural,
    offsetAccidental,
    offsetNatural,      // D
    offsetAccidental,
    offsetNatural,      // E
    offsetNatural,      // F
    offsetAccidental,
    offsetNatural,      // g
    offsetAccidental,
    offsetNatural,
    offsetAccidental,
    offsetNatural,         //b

};

static const char* pitchNames[] = {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B"
};

static const int intervalOffsets[] = {
    11,
    -3,
    3,      // 2nd
    0,
    0,
    4,      // 4th
    -5,
    4,      // 5th
    0,
    0,
    0,
    2,
    2       // M7
};

void EV3PitchDisplay::addOctLabel(Label* l)
{
    octLabels.push_back(l);
}

void EV3PitchDisplay::addSemiLabel(Label* l)
{
    semiLabels.push_back(l);
    semiX.push_back(l->box.pos.x);
}

bool EV3PitchDisplay::shouldUseInterval(int osc)
{
    bool ret = false;       // always safe to use abolute

    if (lastPatched[osc]) {
        ret = false;            // if current one is patched, use absolute

    } else if ((osc > 0) && lastPatched[osc - 1]) {
        ret = true;             // if prev patched and we are not, go for it
    } else if ((osc > 1) && lastPatched[osc - 2]) {
        ret = true;             // if prev-prev patched and we are not, go for it
    }

    //printf("should use interval (%d) ret %d", osc, ret);
    return ret;
}

void EV3PitchDisplay::updateInterval(int osc)
{

    int refSemi = 0;
    int refOctave = 0;
    int oct = 5 + lastOctave[osc];
    int semi = lastSemi[osc];

    assert(osc > 0);
    const bool prevPatched = lastPatched[osc - 1];
    if (prevPatched) {
        refOctave = 5 + lastOctave[osc - 1];
        refSemi = lastSemi[osc - 1];
       // printf("got from prev %d (%d, %d)\n", osc-1, refOctave, refSemi);
    } else {
        assert(osc > 1);
        refOctave = 5 + lastOctave[osc - 2];
        refSemi = lastSemi[osc - 2];
       //  printf("got from prev %d (%d, %d)\n", osc-2, refOctave, refSemi);
    }

    const int currentPitch = oct * 12 + semi;
    const int refPitch = refOctave * 12 + refSemi;
    const int relativePitch = currentPitch - refPitch;

    int adjustedOctave = 0;
    int adjustedSemi = 0;

    adjustedOctave = relativePitch / 12;
    adjustedSemi = relativePitch - (adjustedOctave * 12);
    if (adjustedSemi < 0) {
        adjustedOctave--;
        adjustedSemi += 12;
    }

    assert(adjustedSemi >= 0);
    assert(adjustedSemi < 12);

    std::stringstream so;
    so << adjustedOctave;
    octLabels[osc]->text = so.str();

    semiLabels[osc]->text = intervalNames[adjustedSemi];
    semiLabels[osc]->box.pos.x = semiX[osc] + intervalOffsets[adjustedSemi];
}

void EV3PitchDisplay::updateAbsolute(int osc)
{
    std::stringstream so;
    int oct = 5 + lastOctave[osc];
    int semi = lastSemi[osc];

    if (semi < 0) {
        --oct;
        semi += 12;
    }
    so << oct;
    octLabels[osc]->text = so.str();

    semiLabels[osc]->text = pitchNames[semi];
    semiLabels[osc]->box.pos.x = semiX[osc] + pitchOffsets[semi];
}

struct EV3Widget : ModuleWidget
{
    EV3Widget(EV3Module *);
    void makeSections(EV3Module *);
    void makeSection(EV3Module *, int index);
    void makeInputs(EV3Module *);
    void makeInput(EV3Module* module, int row, int col, int input,
        const char* name, float labelDeltaX);
    void makeOutputs(EV3Module *);
    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void step() override;
    Menu* createContextMenu() override;

    EV3PitchDisplay pitchDisplay;
    EV3Module* const module;
    Label* plusOne = nullptr;
    Label* plusTwo = nullptr;
    bool wasNormalizing = false;
};

inline Menu* EV3Widget::createContextMenu()
{
    Menu* theMenu = ModuleWidget::createContextMenu();
    ManualMenuItem* manual = new ManualMenuItem(
        "https://github.com/squinkylabs/SquinkyVCV/blob/master/docs/ev3.md");
    theMenu->addChild(manual);
    return theMenu;
}

static const NVGcolor COLOR_GREEN2 = nvgRGB(0x90, 0xff, 0x3e);
void EV3Widget::step()
{
    ModuleWidget::step();
    pitchDisplay.step();
    bool norm = module->ev3.isLoweringVolume();
    if (norm != wasNormalizing) {
        wasNormalizing = norm;
        auto color = norm ? COLOR_GREEN2 : COLOR_WHITE;
        plusOne->color = color;
        plusTwo->color = color;
    }
}

const int dy = -6;      // apply to everything

void EV3Widget::makeSection(EV3Module *module, int index)
{
    const float x = (30 - 4) + index * (86 + 4);
    const float x2 = x + (36 + 2);
    const float y = 80 + dy;
    const float y2 = y + 56 + dy;
    const float y3 = y2 + 40 + dy;

    const int delta = EV3<WidgetComposite>::OCTAVE2_PARAM - EV3<WidgetComposite>::OCTAVE1_PARAM;

    pitchDisplay.addOctLabel(
        addLabel(Vec(x - 10, y - 32), "Oct"));
    pitchDisplay.addSemiLabel(
        addLabel(Vec(x2 - 22, y - 32), "Semi"));

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x, y), module,
        EV3<WidgetComposite>::OCTAVE1_PARAM + delta * index,
        -5.0f, 4.0f, 0.f));

    addParam(createParamCentered<Blue30SnapKnob>(
        Vec(x2, y), module,
        EV3<WidgetComposite>::SEMI1_PARAM + delta * index,
        -11.f, 11.0f, 0.f));

    addParam(createParamCentered<Blue30Knob>(
        Vec(x, y2), module,
        EV3<WidgetComposite>::FINE1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    addLabel(Vec(x - 20, y2 - 34), "Fine");

    addParam(createParamCentered<Blue30Knob>(
        Vec(x2, y2), module,
        EV3<WidgetComposite>::FM1_PARAM + delta * index,
        0.f, 1.f, 0));
    addLabel(Vec(x2 - 19, y2 - 34), "Mod");

    const float dy = 27;
    const float x0 = x;

    addParam(createParamCentered<Trimpot>(
        Vec(x0, y3), module, EV3<WidgetComposite>::PW1_PARAM + delta * index,
        -1.f, 1.f, 0));
    if (index == 0)
        addLabel(Vec(x0 + 10, y3 - 8), "pw");

    addParam(createParamCentered<Trimpot>(
        Vec(x0, y3 + dy), module,
        EV3<WidgetComposite>::PWM1_PARAM + delta * index,
        -1.0f, 1.0f, 0));
    if (index == 0)
        addLabel(Vec(x0 + 10, y3 + dy - 8), "pwm");

    // sync switches
    const float swx = x + 29;
    const float lbx = x + 19;

    if (index != 0) {
        addParam(ParamWidget::create<CKSS>(
            Vec(swx, y3), module, EV3<WidgetComposite>::SYNC1_PARAM + delta * index,
            0.0f, 1.0f, 0.0f));
        addLabel(Vec(lbx - 3, y3 - 20), "sync");
        addLabel(Vec(lbx + 2, y3 + 20), "off");
    }

    const float y4 = y3 + 43;
    const float xx = x - 12;
        // include one extra wf - none
    const float numWaves = (float) EV3<WidgetComposite>::Waves::END;
    const float defWave = (float) EV3<WidgetComposite>::Waves::SIN;
    addParam(ParamWidget::create<WaveformSelector>(
        Vec(xx, y4),
        module,
        EV3<WidgetComposite>::WAVE1_PARAM + delta * index,
        0.0f, numWaves, defWave));
}

void EV3Widget::makeSections(EV3Module* module)
{
    makeSection(module, 0);
    makeSection(module, 1);
    makeSection(module, 2);
}

const float row1Y = 280 + dy - 4;       // -4 = move the last section up
const float rowDY = 30;
const float colDX = 45;

void EV3Widget::makeInput(EV3Module* module, int row, int col,
    int inputNum, const char* name, float labelXDelta)
{
    EV3<WidgetComposite>::InputIds input = EV3<WidgetComposite>::InputIds(inputNum);
    const float y = row1Y + row * rowDY;
    const float x = 14 + col * colDX;
    const float labelX = labelXDelta + x - 6;
    addInput(Port::create<PJ301MPort>(
        Vec(x, y), Port::INPUT, module, input));
    if (row == 0)
        addLabel(Vec(labelX, y - 20), name);
}

void EV3Widget::makeInputs(EV3Module* module)
{
    // Row 0 = top row, 2 = bottom row
    auto row2Input = [](int row, EV3<WidgetComposite>::InputIds baseInput) {
        // map inputs directly to rows
        return baseInput + row;
    };

    for (int row = 0; row < 3; ++row) {
        makeInput(module, row, 0,
            row2Input(row, EV3<WidgetComposite>::CV1_INPUT),
            "V/oct", -3);
        makeInput(module, row, 1,
            row2Input(row, EV3<WidgetComposite>::FM1_INPUT),
            "Fm", 3);
        makeInput(module, row, 2,
            row2Input(row, EV3<WidgetComposite>::PWM1_INPUT),
            "Pwm", -2);
    }
}

void EV3Widget::makeOutputs(EV3Module *)
{
    const float x = 160;
    const float trimY = row1Y + 11;
    const float outX = x + 30;

    addParam(createParamCentered<Trimpot>(
        Vec(x, trimY), module, EV3<WidgetComposite>::MIX1_PARAM,
        0.0f, 1.0f, 0));
    addParam(createParamCentered<Trimpot>(
        Vec(x, trimY + rowDY), module, EV3<WidgetComposite>::MIX2_PARAM,
        0.0f, 1.0f, 0));
    addParam(createParamCentered<Trimpot>(
        Vec(x, trimY + 2 * rowDY), module, EV3<WidgetComposite>::MIX3_PARAM,
        0.0f, 1.0f, 0));

    addOutput(Port::create<PJ301MPort>(
        Vec(outX, row1Y),
        Port::OUTPUT, module, EV3<WidgetComposite>::VCO1_OUTPUT));
    addLabel(Vec(outX + 20, row1Y + 0 * rowDY + 2), "1", COLOR_WHITE);

    addOutput(Port::create<PJ301MPort>(
        Vec(outX, row1Y + rowDY),
        Port::OUTPUT, module, EV3<WidgetComposite>::VCO2_OUTPUT));
    addLabel(Vec(outX + 20, row1Y + 1 * rowDY + 2), "2", COLOR_WHITE);

    addOutput(Port::create<PJ301MPort>(
        Vec(outX, row1Y + 2 * rowDY),
        Port::OUTPUT, module, EV3<WidgetComposite>::VCO3_OUTPUT));
    addLabel(Vec(outX + 20, row1Y + 2 * rowDY + 2), "3", COLOR_WHITE);

    addOutput(Port::create<PJ301MPort>(
        Vec(outX + 41, row1Y + rowDY),
        Port::OUTPUT, module, EV3<WidgetComposite>::MIX_OUTPUT));
    plusOne = addLabel(Vec(outX + 41, row1Y + rowDY - 17), "+", COLOR_WHITE);
    plusTwo = addLabel(Vec(outX + 41, row1Y + rowDY + 20), "+", COLOR_WHITE);
}

/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */
EV3Widget::EV3Widget(EV3Module *module) :
    ModuleWidget(module),
    pitchDisplay(module),
    module(module)
{
    box.size = Vec(18 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/ev3_panel.svg")));
        addChild(panel);
    }

    makeSections(module);
    makeInputs(module);
    makeOutputs(module);

    // screws
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

}



RACK_PLUGIN_MODEL_INIT(squinkylabs_plug1, EV3) {
   Model *modelEV3Module = Model::create<EV3Module,
                                         EV3Widget>("Squinky Labs",
                                                    "squinkylabs-ev3",
                                                    "EV3: Triple VCO with even waveform", OSCILLATOR_TAG);
   return modelEV3Module;
}
#endif

