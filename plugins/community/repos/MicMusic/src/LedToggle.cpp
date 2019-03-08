#include "LedToggle.hpp"

namespace rack_plugin_MicMusic {

LedToggle::LedToggle() {
    addFrame(SVG::load(assetPlugin(plugin, "res/LedToggleOff.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/LedToggleOn.svg")));
}

} // namespace rack_plugin_MicMusic
