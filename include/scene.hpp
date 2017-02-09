#pragma once
#include <vector>
#include <jansson.h>
#include "widgets.hpp"


namespace rack {


struct Module;
struct Wire;

struct RackWidget;
struct ParamWidget;
struct InputPort;
struct OutputPort;
struct Scene;

////////////////////
// module
////////////////////

// A 1U module should be 15x380. Thus the width of a module should be a factor of 15.
struct Model;
struct ModuleWidget : OpaqueWidget {
	Model *model = NULL;
	// Eventually this should be replaced with a `moduleId` which will be used for inter-process communication between the gui world and the audio world.
	Module *module = NULL;
	// int moduleId;

	std::vector<InputPort*> inputs;
	std::vector<OutputPort*> outputs;
	std::vector<ParamWidget*> params;

	ModuleWidget(Module *module);
	~ModuleWidget();
	// Convenience functions for adding special widgets (calls addChild())
	void addInput(InputPort *input);
	void addOutput(OutputPort *output);
	void addParam(ParamWidget *param);

	json_t *toJson();
	void fromJson(json_t *root);
	void disconnectPorts();
	void resetParams();
	void cloneParams(ModuleWidget *source);

	void draw(NVGcontext *vg);

	bool requested = false;
	Vec requestedPos;
	Vec dragPos;
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
	void onMouseDown(int button);
};

struct WireWidget : OpaqueWidget {
	OutputPort *outputPort = NULL;
	InputPort *inputPort = NULL;
	OutputPort *hoveredOutputPort = NULL;
	InputPort *hoveredInputPort = NULL;
	Wire *wire = NULL;
	NVGcolor color;

	WireWidget();
	~WireWidget();
	void updateWire();
	void draw(NVGcontext *vg);
	void drawOutputPlug(NVGcontext *vg);
	void drawInputPlug(NVGcontext *vg);
};

struct RackWidget : OpaqueWidget {
	// Only put ModuleWidgets in here
	Widget *moduleContainer;
	// Only put WireWidgets in here
	Widget *wireContainer;
	WireWidget *activeWire = NULL;
	std::shared_ptr<Image> railsImage;

	RackWidget();
	~RackWidget();
	void clear();
	void savePatch(std::string filename);
	void loadPatch(std::string filename);
	json_t *toJson();
	void fromJson(json_t *root);

	void repositionModule(ModuleWidget *module);
	void step();
	void draw(NVGcontext *vg);

	void onMouseDown(int button);
};

struct Panel : TransparentWidget {
	NVGcolor backgroundColor;
	NVGcolor borderColor;
	std::shared_ptr<Image> backgroundImage;
	void draw(NVGcontext *vg);
};

////////////////////
// params
////////////////////

struct Light : TransparentWidget {
	NVGcolor color;
	void draw(NVGcontext *vg);
};

struct ParamWidget : OpaqueWidget, QuantityWidget {
	Module *module = NULL;
	int paramId;

	json_t *toJson();
	void fromJson(json_t *root);
	void onMouseDown(int button);
	void onChange();
};

struct Knob : ParamWidget {
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
};

struct SpriteKnob : Knob, SpriteWidget {
	int minIndex, maxIndex, spriteCount;
	void step();
};

struct Switch : ParamWidget, SpriteWidget {
};

struct ToggleSwitch : virtual Switch {
	void onDragStart() {
		index = 1;
	}
	void onDragEnd() {
		index = 0;
	}
	void onDragDrop(Widget *origin) {
		if (origin != this)
			return;

		// Cycle through modes
		// e.g. a range of [0.0, 3.0] would have modes 0, 1, 2, and 3.
		float v = value + 1.0;
		setValue(v > maxValue ? minValue : v);
	}
};

struct MomentarySwitch : virtual Switch {
	void onDragStart() {
		setValue(maxValue);
		index = 1;
	}
	void onDragEnd() {
		setValue(minValue);
		index = 0;
	}
};

////////////////////
// ports
////////////////////

struct Port : OpaqueWidget {
	Module *module = NULL;
	WireWidget *connectedWire = NULL;

	Port();
	~Port();
	void disconnect();

	void onMouseDown(int button);
	void onDragEnd();
};

struct InputPort : Port {
	int inputId;

	void onDragStart();
	void onDragDrop(Widget *origin);
	void onDragEnter(Widget *origin);
	void onDragLeave(Widget *origin);
};

struct OutputPort : Port {
	int outputId;

	void onDragStart();
	void onDragDrop(Widget *origin);
	void onDragEnter(Widget *origin);
	void onDragLeave(Widget *origin);
};

////////////////////
// scene
////////////////////

struct Toolbar : OpaqueWidget {
	Slider *wireOpacitySlider;
	Slider *wireTensionSlider;
	RadioButton *cpuUsageButton;

	Toolbar();
	void draw(NVGcontext *vg);
};

struct RackScene : Scene {
	Toolbar *toolbar;
	ScrollWidget *scrollWidget;

	RackScene();
	void step();
	void draw(NVGcontext *vg);
};

////////////////////
// globals
////////////////////

extern std::string gApplicationName;
extern std::string gApplicationVersion;

extern Scene *gScene;
extern RackWidget *gRackWidget;

void sceneInit();
void sceneDestroy();

} // namespace rack
