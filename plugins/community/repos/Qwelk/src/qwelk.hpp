#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(Qwelk);

#ifdef USE_VST2
#define plugin "Qwelk"
#endif // USE_VST2



struct TinyKnob : RoundKnob {
    TinyKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/TinyKnob.svg")));
    }
};

struct SmallKnob : RoundKnob {
    SmallKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/SmallKnob.svg")));
    }
};

/*

struct WidgetAutomaton : ModuleWidget {
    WidgetAutomaton();
    Menu *createContextMenu() override;
};

struct WidgetChaos : ModuleWidget {
    WidgetChaos();
    Menu *createContextMenu() override;
};

struct WidgetByte : ModuleWidget {
    WidgetByte();
};

struct WidgetScaler : ModuleWidget {
    WidgetScaler();
};

struct WidgetXFade : ModuleWidget {
    WidgetXFade();
};

struct WidgetOr : ModuleWidget {
    WidgetOr();
};

struct WidgetNot : ModuleWidget {
    WidgetNot();
};

struct WidgetXor : ModuleWidget {
    WidgetXor();
};

struct WidgetNews : ModuleWidget {
    WidgetNews();
};
struct WidgetMix : ModuleWidget {
    WidgetMix();
};

struct WidgetColumn : ModuleWidget {
    WidgetColumn();
    Menu *createContextMenu() override;
};

struct WidgetGate : ModuleWidget {
    WidgetGate();
};

struct WidgetWrap : ModuleWidget {
    WidgetWrap();
};
*/
