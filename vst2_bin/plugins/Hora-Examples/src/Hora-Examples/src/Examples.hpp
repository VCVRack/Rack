#include "rack.hpp" 

using namespace rack;

RACK_PLUGIN_DECLARE(Hora_Examples);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "Hora-Examples"
#endif // USE_VST2

struct switch_0 : SVGSwitch, ToggleSwitch {
switch_0() {
    addFrame(SVG::load(assetPlugin(plugin,"res/switch_0.svg")));
    addFrame(SVG::load(assetPlugin(plugin,"res/switch_1.svg")));
    sw->wrap();
    box.size = sw->box.size;
    }
};
struct jack : SVGPort{
jack() {
    background->svg = SVG::load(assetPlugin(plugin,"res/jack.svg"));
    background->wrap();
    box.size = background->box.size;
    }
};
struct mediumKnob : SVGKnob{
mediumKnob() {
    box.size = Vec(25, 25);
    minAngle = -0.75*M_PI;
    maxAngle = 0.75*M_PI;
    setSVG(SVG::load(assetPlugin(plugin,"res/mediumKnob.svg")));
    SVGWidget *shadow = new SVGWidget();
    shadow->setSVG(SVG::load(assetPlugin(plugin, "res/mediumknobShadow.svg")));
    addChild(shadow);
    }
};
struct mediumRotarysnap : SVGKnob{
mediumRotarysnap() {
    box.size = Vec(25, 25);
    minAngle = -0.75*M_PI;
    maxAngle = 0.75*M_PI;
    setSVG(SVG::load(assetPlugin(plugin,"res/mediumRotary.svg")));
    SVGWidget *shadow = new SVGWidget();
    shadow->setSVG(SVG::load(assetPlugin(plugin, "res/mediumRotaryShadow.svg")));
    addChild(shadow);
    }
};
struct mediumRotarysnap_snap : mediumRotarysnap{
    mediumRotarysnap_snap() {
    snap = true;
    }
};
struct button : SVGSwitch, MomentarySwitch {
    button(){
    addFrame(SVG::load(assetPlugin(plugin,"res/button.svg")));
    addFrame(SVG::load(assetPlugin(plugin,"res/buttonPushed.svg")));
    sw->wrap();
    box.size = sw->box.size;
    }
};
