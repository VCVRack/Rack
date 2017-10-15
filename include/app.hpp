#pragma once
#include <vector>
#include <jansson.h>
#include "widgets.hpp"


namespace rack {


struct Module;
struct Wire;

struct RackWidget;
struct ParamWidget;
struct Port;
struct Scene;

////////////////////
// module
////////////////////

// A 1U module should be 15x380. Thus the width of a module should be a factor of 15.
#define RACK_GRID_WIDTH 15
#define RACK_GRID_HEIGHT 380

struct Model;
struct ModuleWidget : OpaqueWidget {
	Model *model = NULL;
	/** Owns the module pointer */
	Module *module = NULL;

	std::vector<Port*> inputs;
	std::vector<Port*> outputs;
	std::vector<ParamWidget*> params;

	~ModuleWidget();
	void setModule(Module *module);
	/** Convenience functions for adding special widgets (calls addChild()) */
	void addInput(Port *input);
	void addOutput(Port *output);
	void addParam(ParamWidget *param);

	virtual json_t *toJson();
	virtual void fromJson(json_t *rootJ);

	/** Disconnects cables from all ports
	Called when the user clicks Disconnect Cables in the context menu.
	*/
	virtual void disconnect();
	/** Resets the parameters of the module and calls the Module's randomize().
	Called when the user clicks Initialize in the context menu.
	*/
	virtual void initialize();
	/** Randomizes the parameters of the module and calls the Module's randomize().
	Called when the user clicks Randomize in the context menu.
	*/
	virtual void randomize();
	virtual Menu *createContextMenu();

	void draw(NVGcontext *vg);

	Vec dragPos;
	Widget *onMouseMove(Vec pos, Vec mouseRel);
	Widget *onHoverKey(Vec pos, int key);
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
	void onMouseDownOpaque(int button);
};

struct ValueLight;
struct WireWidget : OpaqueWidget {
	Port *outputPort = NULL;
	Port *inputPort = NULL;
	Port *hoveredOutputPort = NULL;
	Port *hoveredInputPort = NULL;
	ValueLight *inputLight;
	ValueLight *outputLight;
	Wire *wire = NULL;
	NVGcolor color;

	WireWidget();
	~WireWidget();
	/** Synchronizes the plugged state of the widget to the owned wire */
	void updateWire();
	Vec getOutputPos();
	Vec getInputPos();
	void draw(NVGcontext *vg);
	void drawPlugs(NVGcontext *vg);
};

struct WireContainer : TransparentWidget {
	WireWidget *activeWire = NULL;
	/** Takes ownership of `w` and adds it as a child if it isn't already */
	void setActiveWire(WireWidget *w);
	/** "Drops" the wire onto the port, making an engine connection if successful */
	void commitActiveWire();
	void removeTopWire(Port *port);
	void removeAllWires(Port *port);
	/** Returns the most recently added wire connected to the given Port, i.e. the top of the stack */
	WireWidget *getTopWire(Port *port);
	void draw(NVGcontext *vg);
};

struct RackWidget : OpaqueWidget {
	FramebufferWidget *rails;
	// Only put ModuleWidgets in here
	Widget *moduleContainer;
	// Only put WireWidgets in here
	WireContainer *wireContainer;
	std::string lastPath;

	RackWidget();
	~RackWidget();

	void clear();
	void initialize();
	void openDialog();
	void saveDialog();
	void saveAsDialog();
	void savePatch(std::string filename);
	void loadPatch(std::string filename);
	json_t *toJson();
	void fromJson(json_t *rootJ);

	void addModule(ModuleWidget *m);
	/** Transfers ownership to the caller so they must `delete` it if that is the intension */
	void deleteModule(ModuleWidget *m);
	void cloneModule(ModuleWidget *m);
	/** Sets a module's box if non-colliding. Returns true if set */
	bool requestModuleBox(ModuleWidget *m, Rect box);
	/** Moves a module to the closest non-colliding position */
	bool requestModuleBoxNearest(ModuleWidget *m, Rect box);
	void step();
	void draw(NVGcontext *vg);

	void onMouseDownOpaque(int button);
};

struct RackRail : TransparentWidget {
	void draw(NVGcontext *vg);
};

struct Panel : TransparentWidget {
	NVGcolor backgroundColor;
	std::shared_ptr<Image> backgroundImage;
	void draw(NVGcontext *vg);
};

struct SVGPanel : FramebufferWidget {
	void setBackground(std::shared_ptr<SVG> svg);
};

////////////////////
// params
////////////////////

struct CircularShadow : TransparentWidget {
	float blur = 0.0;
	void draw(NVGcontext *vg);
};

struct Light : TransparentWidget {
	NVGcolor bgColor = nvgRGBf(0, 0, 0);
	NVGcolor color = nvgRGBf(1, 1, 1);
	void draw(NVGcontext *vg);
};

struct ParamWidget : OpaqueWidget, QuantityWidget {
	Module *module = NULL;
	int paramId;
	/** Used to momentarily disable value randomization
	To permanently disable or change randomization behavior, override the randomize() method instead of changing this.
	*/
	bool randomizable = true;

	json_t *toJson();
	void fromJson(json_t *rootJ);
	virtual void randomize();
	void onMouseDownOpaque(int button);
	void onChange();
};

/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	void onDragStart();
	void onDragMove(Vec mouseRel);
	void onDragEnd();
	/** Tell engine to smoothly vary this parameter */
	void onChange();
};

struct SpriteKnob : virtual Knob, SpriteWidget {
	int minIndex, maxIndex, spriteCount;
	void step();
};

/** A knob which rotates an SVG and caches it in a framebuffer */
struct SVGKnob : virtual Knob, FramebufferWidget {
	/** Angles in radians */
	float minAngle, maxAngle;
	/** Not owned */
	TransformWidget *tw;
	SVGWidget *sw;

	SVGKnob();
	void setSVG(std::shared_ptr<SVG> svg);
	void step();
	void onChange();
};

/** Snaps to the nearest integer value on mouse release */
struct SnapKnob : virtual Knob {
	void onDragEnd() {
		setValue(roundf(value));
		Knob::onDragEnd();
	}
};

struct SVGSlider : Knob, FramebufferWidget {
	/** Intermediate positions will be interpolated between these positions */
	Vec minHandlePos, maxHandlePos;
	/** Not owned */
	SVGWidget *background;
	SVGWidget *handle;

	SVGSlider();
	void step();
	void onChange();
};

struct Switch : ParamWidget {
};

struct SVGSwitch : virtual Switch, FramebufferWidget {
	std::vector<std::shared_ptr<SVG>> frames;
	/** Not owned */
	SVGWidget *sw;

	SVGSwitch();
	/** Adds an SVG file to represent the next switch position */
	void addFrame(std::shared_ptr<SVG> svg);
	void step();
	void onChange();
};

/** A switch that cycles through each mechanical position */
struct ToggleSwitch : virtual Switch {
	void onDragStart() {
		// Cycle through values
		// e.g. a range of [0.0, 3.0] would have modes 0, 1, 2, and 3.
		if (value >= maxValue)
			setValue(minValue);
		else
			setValue(value + 1.0);
	}
};

/** A switch that is turned on when held */
struct MomentarySwitch : virtual Switch {
	/** Don't randomize state */
	void randomize() {}
	void onDragStart() {
		setValue(maxValue);
	}
	void onDragEnd() {
		setValue(minValue);
	}
};

////////////////////
// ports
////////////////////

struct Port : OpaqueWidget {
	enum PortType {
		INPUT,
		OUTPUT
	};

	Module *module = NULL;
	PortType type = INPUT;
	int portId;

	~Port();
	void draw(NVGcontext *vg);
	void onMouseDownOpaque(int button);
	void onDragEnd();
	void onDragStart();
	void onDragDrop(Widget *origin);
	void onDragEnter(Widget *origin);
	void onDragLeave(Widget *origin);
};

struct SVGPort : Port, FramebufferWidget {
	SVGWidget *background;

	SVGPort();
	void draw(NVGcontext *vg);
};

/** If you don't add these to your ModuleWidget, they will fall out of the rack... */
struct SVGScrew : FramebufferWidget {
	SVGWidget *sw;

	SVGScrew();
};

////////////////////
// scene
////////////////////

struct Toolbar : OpaqueWidget {
	Slider *wireOpacitySlider;
	Slider *wireTensionSlider;
	RadioButton *cpuUsageButton;
	RadioButton *plugLightButton;

	Toolbar();
	void draw(NVGcontext *vg);
};

struct PluginManagerWidget : Widget {
	Widget *loginWidget;
	Widget *manageWidget;
	Widget *downloadWidget;
	PluginManagerWidget();
	void step();
};

struct RackScene : Scene {
	ScrollWidget *scrollWidget;
	ZoomWidget *zoomWidget;

	RackScene();
	void step();
	void draw(NVGcontext *vg);
	Widget *onHoverKey(Vec pos, int key);
};

////////////////////
// globals
////////////////////

extern std::string gApplicationName;
extern std::string gApplicationVersion;
extern std::string gApiHost;

// Easy access to "singleton" widgets
extern RackWidget *gRackWidget;
extern Toolbar *gToolbar;

void sceneInit();
void sceneDestroy();

} // namespace rack
