#include "rack.hpp"


using namespace rack;


////////////////////
// module widgets
////////////////////

struct AudioInterfaceWidget : ModuleWidget {
	AudioInterfaceWidget();
};

struct MidiToCVWidget : ModuleWidget {
	MidiToCVWidget();
	void step();
};

struct MIDICCToCVWidget : ModuleWidget {
	MIDICCToCVWidget();
	void step();
};
