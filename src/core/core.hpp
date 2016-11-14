#include "../5V.hpp"


Plugin *coreInit();

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
