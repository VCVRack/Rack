#include "rack.hpp"


using namespace rack;

Plugin *coreInit();

void audioInit();
void midiInit();

////////////////////
// module widgets
////////////////////

struct AudioInterfaceWidget : ModuleWidget {
	AudioInterfaceWidget();
};

struct MidiInterfaceWidget : ModuleWidget {
	MidiInterfaceWidget();
};
