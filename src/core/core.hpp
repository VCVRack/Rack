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

struct MIDIClockToCVWidget : ModuleWidget {
	MIDIClockToCVWidget();
	void step();
};

struct BridgeWidget : ModuleWidget {
	BridgeWidget();
};

struct BlankWidget : ModuleWidget {
	Panel *panel;
	Widget *topRightScrew;
	Widget *bottomRightScrew;
	Widget *rightHandle;
	BlankWidget();
	void step();
	json_t *toJson();
	void fromJson(json_t *rootJ);
};
