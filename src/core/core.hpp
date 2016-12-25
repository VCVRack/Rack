#include "Rack.hpp"


rack::Plugin *coreInit();

void audioInit();
void midiInit();

////////////////////
// module widgets
////////////////////

struct AudioInterfaceWidget : rack::ModuleWidget {
	AudioInterfaceWidget();
	void draw(NVGcontext *vg);
};

struct MidiInterfaceWidget : rack::ModuleWidget {
	MidiInterfaceWidget();
	void draw(NVGcontext *vg);
};
