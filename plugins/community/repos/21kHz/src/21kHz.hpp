#include "rack.hpp"


using namespace rack;

// Forward-declare the Plugin, defined in Template.cpp
#define plugin "21kHz"


////////////////////////////////

// Knobs

struct kHzKnob : RoundKnob {
    kHzKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/Components/kHzKnob.svg")));
        shadow->box.pos = Vec(0.0, 3.5);
    }
};

struct kHzKnobSmall : RoundKnob {
    kHzKnobSmall() {
        setSVG(SVG::load(assetPlugin(plugin, "res/Components/kHzKnobSmall.svg")));
        shadow->box.pos = Vec(0.0, 2.5);
    }
};

struct kHzKnobSnap : kHzKnob {
    kHzKnobSnap() {
        snap = true;
    }
};

struct kHzKnobSmallSnap : kHzKnobSmall {
    kHzKnobSmallSnap() {
        snap = true;
    }
};

// Buttons

struct kHzButton : SVGSwitch, ToggleSwitch {
    kHzButton() {
        addFrame(SVG::load(assetPlugin(plugin, "res/Components/kHzButton_0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/Components/kHzButton_1.svg")));
    }
};
        
// Ports

struct kHzPort : SVGPort {
    kHzPort() {
        setSVG(SVG::load(assetPlugin(plugin, "res/Components/kHzPort.svg")));
        shadow->box.pos = Vec(0.0, 2.0);
    }
};

// Misc

struct kHzScrew : SVGScrew {
    kHzScrew() {
        sw->setSVG(SVG::load(assetPlugin(plugin, "res/Components/kHzScrew.svg")));
    }
};
