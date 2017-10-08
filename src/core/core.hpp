#include "rack.hpp"


using namespace rack;


////////////////////
// module widgets
////////////////////

struct AudioInterfaceWidget : ModuleWidget {
	AudioInterfaceWidget();
};

struct MidiInterfaceWidget : ModuleWidget {
	MidiInterfaceWidget();
	void step();
};

struct BridgeWidget : ModuleWidget {
	BridgeWidget();
};
