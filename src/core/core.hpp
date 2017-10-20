#include "rack.hpp"


using namespace rack;


////////////////////
// module widgets
////////////////////

struct AudioInterfaceWidget : ModuleWidget {
	AudioInterfaceWidget();
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
	void step() override;
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
};
