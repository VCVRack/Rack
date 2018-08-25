#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(com_soundchasing_stochasm);

#ifdef USE_VST2
#define plugin "com-soundchasing-stochasm"
#endif // USE_VST2

////////////////////
// module widgets
////////////////////

namespace rack_plugin_com_soundchasing_stochasm {

struct StochasmKnob : SVGKnob {
    StochasmKnob() {
       minAngle = float(-0.75*M_PI);
       maxAngle = float(0.75*M_PI);
    }
};

struct StochasmMintKnob : StochasmKnob {
    StochasmMintKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/MintKnob.svg")));
    }
};

struct StochasmTangerineKnob : StochasmKnob {
    StochasmTangerineKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/TangerineKnob.svg")));
    }
};


struct StochasmMintLargeKnob : StochasmKnob {
    StochasmMintLargeKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/MintKnobLarge.svg")));
    }
};

struct StochasmTangerineLargeKnob : StochasmKnob {
    StochasmTangerineLargeKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/TangerineKnobLarge.svg")));
    }
};

struct MintMomentarySwitch : SVGSwitch, MomentarySwitch {
    MintMomentarySwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/MintMomentary0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/MintMomentary1.svg")));
    }
};

struct TangerineMomentarySwitch : SVGSwitch, MomentarySwitch {
    TangerineMomentarySwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/TangerineMomentary0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/TangerineMomentary1.svg")));
    }
};

} // namespace rack_plugin_com_soundchasing_stochasm
