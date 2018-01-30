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
};

// struct MIDICCToCVWidget : ModuleWidget {
// 	MIDICCToCVWidget();
// 	void step() override;
// };

// struct MIDIClockToCVWidget : ModuleWidget {
// 	MIDIClockToCVWidget();
// 	void step() override;
// };

// struct MIDITriggerToCVWidget : ModuleWidget {
// 	MIDITriggerToCVWidget();
// 	void step() override;
// };

// struct QuadMidiToCVWidget : ModuleWidget {
// 	QuadMidiToCVWidget();
// 	void step() override;
// };

struct BridgeWidget : ModuleWidget {
	BridgeWidget();
};

struct BlankWidget : ModuleWidget {
	Panel *panel;
	Widget *topRightScrew;
	Widget *bottomRightScrew;
	Widget *rightHandle;
	BlankWidget();
	void step() override;
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
};

struct NotesWidget : ModuleWidget {
	TextField *textField;
	NotesWidget();
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
};
