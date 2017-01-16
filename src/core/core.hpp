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
	void draw(NVGcontext *vg);
};

struct MidiInterfaceWidget : ModuleWidget {
	MidiInterfaceWidget();
	void draw(NVGcontext *vg);
};
