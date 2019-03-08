
#pragma once

namespace rack_plugin_Skylights {

struct DavidLTPort : public SVGPort {
   DavidLTPort() {	
      setSVG(SVG::load(assetPlugin(plugin, "res/cntr_LT.svg")));
   }
};

} // namespace rack_plugin_Skylights
