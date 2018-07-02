#include "rack.hpp"


using namespace rack;

RACK_PLUGIN_DECLARE(ErraticInstruments);

#ifdef USE_VST2
#define plugin "ErraticInstruments"
#endif // USE_VST2


// struct MPEToCVWidget : ModuleWidget {
// 	MPEToCVWidget();
// 	Menu *createContextMenu() override;
// 	void step() override;
// };

// struct QuadMPEToCVWidget : ModuleWidget {
// 	QuadMPEToCVWidget();
// 	Menu *createContextMenu() override;
// 	void step() override;
// };
