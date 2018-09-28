#pragma once
#include <vector>
#include <jansson.h>
#include "widgets.hpp"
#include "ui.hpp"


static const float SVG_DPI = 75.0;
static const float MM_PER_IN = 25.4;


namespace rack {


inline float in2px(float inches) {
	return inches * SVG_DPI;
}

inline math::Vec in2px(math::Vec inches) {
	return inches.mult(SVG_DPI);
}

inline float mm2px(float millimeters) {
	return millimeters * (SVG_DPI / MM_PER_IN);
}

inline math::Vec mm2px(math::Vec millimeters) {
	return millimeters.mult(SVG_DPI / MM_PER_IN);
}


struct Model;
struct Module;
struct Wire;

struct RackWidget;
struct ParamWidget;
struct Port;
struct SVGPanel;

////////////////////
// module
////////////////////

// A 1HPx3U module should be 15x380 pixels. Thus the width of a module should be a factor of 15.
static const float RACK_GRID_WIDTH = 15;
static const float RACK_GRID_HEIGHT = 380;
static const math::Vec RACK_GRID_SIZE = math::Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
static const std::string PRESET_FILTERS = "VCV Rack module preset (.vcvm):vcvm";
static const std::string PATCH_FILTERS = "VCV Rack patch (.vcv):vcv";

} // namespace rack


#include "app/ModuleWidget.hpp"


namespace rack {


struct WireWidget : OpaqueWidget {
	Port *outputPort = NULL;
	Port *inputPort = NULL;
	Port *hoveredOutputPort = NULL;
	Port *hoveredInputPort = NULL;
	Wire *wire = NULL;
	NVGcolor color;

	WireWidget();
	~WireWidget();
	/** Synchronizes the plugged state of the widget to the owned wire */
	void updateWire();
	math::Vec getOutputPos();
	math::Vec getInputPos();
	json_t *toJson();
	void fromJson(json_t *rootJ);
	void draw(NVGcontext *vg) override;
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
	void draw(NVGcontext *vg) override;
};

struct RackWidget : OpaqueWidget {
	FramebufferWidget *rails;
	// Only put ModuleWidgets in here
	Widget *moduleContainer;
	// Only put WireWidgets in here
	WireContainer *wireContainer;
	std::string lastPath;
	math::Vec lastMousePos;
	bool lockModules = false;

	RackWidget();
	~RackWidget();

	/** Completely clear the rack's modules and wires */
	void clear();
	/** Clears the rack and loads the template patch */
	void reset();
	void loadDialog();
	void saveDialog();
	void saveAsDialog();
	/** If `lastPath` is defined, ask the user to reload it */
	void revert();
	/** Disconnects all wires */
	void disconnect();
	void save(std::string filename);
	void load(std::string filename);
	json_t *toJson();
	void fromJson(json_t *rootJ);
	/** Creates a module and adds it to the rack */
	ModuleWidget *moduleFromJson(json_t *moduleJ);
	void pastePresetClipboard();

	void addModule(ModuleWidget *m);
	/** Removes the module and transfers ownership to the caller */
	void deleteModule(ModuleWidget *m);
	void cloneModule(ModuleWidget *m);
	/** Sets a module's box if non-colliding. Returns true if set */
	bool requestModuleBox(ModuleWidget *m, math::Rect box);
	/** Moves a module to the closest non-colliding position */
	bool requestModuleBoxNearest(ModuleWidget *m, math::Rect box);

	void step() override;
	void draw(NVGcontext *vg) override;

	void on(event::Hover &e) override;
	void on(event::Button &e) override;
	void on(event::Zoom &e) override;
};

struct RackRail : TransparentWidget {
	void draw(NVGcontext *vg) override;
};

////////////////////
// ParamWidgets and other components
////////////////////

/** A Widget that exists on a Panel and interacts with a Module */
struct Component : OpaqueWidget {
	Module *module = NULL;
};

struct CircularShadow : TransparentWidget {
	float blurRadius;
	float opacity;
	CircularShadow();
	void draw(NVGcontext *vg) override;
};

/** A Component which has control over a Param (defined in engine.hpp) */
struct ParamWidget : Component, QuantityWidget {
	int paramId;
	/** Used to momentarily disable value randomization
	To permanently disable or change randomization behavior, override the randomize() method instead of changing this.
	*/
	bool randomizable = true;
	/** Apply per-sample smoothing in the engine */
	bool smooth = false;

	json_t *toJson();
	void fromJson(json_t *rootJ);
	virtual void reset();
	virtual void randomize();
	void on(event::Button &e) override;
	void on(event::Change &e) override;
};

/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	/** Snap to nearest integer while dragging */
	bool snap = false;
	/** Multiplier for mouse movement to adjust knob value */
	float speed = 1.0;
	float dragValue;
	Knob();
	void on(event::DragStart &e) override;
	void on(event::DragMove &e) override;
	void on(event::DragEnd &e) override;
};

/** A knob which rotates an SVG and caches it in a framebuffer */
struct SVGKnob : Knob, FramebufferWidget {
	TransformWidget *tw;
	SVGWidget *sw;
	CircularShadow *shadow;
	/** Angles in radians */
	float minAngle, maxAngle;

	SVGKnob();
	void setSVG(std::shared_ptr<SVG> svg);
	void step() override;
	void on(event::Change &e) override;
};

/** Behaves like a knob but linearly moves an SVGWidget between two points.
Can be used for horizontal or vertical linear faders.
*/
struct SVGSlider : Knob, FramebufferWidget {
	SVGWidget *background;
	SVGWidget *handle;
	/** Intermediate positions will be interpolated between these positions */
	math::Vec minHandlePos, maxHandlePos;

	SVGSlider();
	void setSVGs(std::shared_ptr<SVG> backgroundSVG, std::shared_ptr<SVG> handleSVG);
	void step() override;
	void on(event::Change &e) override;
};

/** A ParamWidget with multiple frames corresponding to its value */
struct SVGSwitch : virtual ParamWidget, FramebufferWidget {
	std::vector<std::shared_ptr<SVG>> frames;
	SVGWidget *sw;
	SVGSwitch();
	/** Adds an SVG file to represent the next switch position */
	void addFrame(std::shared_ptr<SVG> svg);
	void on(event::Change &e) override;
};

/** A switch that cycles through each mechanical position */
struct ToggleSwitch : virtual ParamWidget {
	void on(event::DragStart &e) override;
};

/** A switch that is turned on when held and turned off when released.
Consider using SVGButton if the switch simply changes the state of your Module when clicked.
*/
struct MomentarySwitch : virtual ParamWidget {
	/** Don't randomize state */
	void randomize() override {}
	void on(event::DragStart &e) override;
	void on(event::DragEnd &e) override;
};

/** A Component with a default (up) and active (down) state when clicked.
Does not modify a Param, simply calls onAction() of a subclass.
*/
struct SVGButton : Component, FramebufferWidget {
	Module *module = NULL;
	std::shared_ptr<SVG> defaultSVG;
	std::shared_ptr<SVG> activeSVG;
	SVGWidget *sw;
	SVGButton();
	/** If `activeSVG` is NULL, `defaultSVG` is used as the active state instead. */
	void setSVGs(std::shared_ptr<SVG> defaultSVG, std::shared_ptr<SVG> activeSVG);
	void on(event::DragStart &e) override;
	void on(event::DragEnd &e) override;
};

////////////////////
// IO widgets
////////////////////

struct LedDisplay : virtual EventWidget {
	void draw(NVGcontext *vg) override;
};

struct LedDisplaySeparator : TransparentWidget {
	LedDisplaySeparator();
	void draw(NVGcontext *vg) override;
};

struct LedDisplayChoice : TransparentWidget {
	std::string text;
	std::shared_ptr<Font> font;
	math::Vec textOffset;
	NVGcolor color;
	LedDisplayChoice();
	void draw(NVGcontext *vg) override;
	void on(event::Button &e) override;
};

struct LedDisplayTextField : TextField {
	std::shared_ptr<Font> font;
	math::Vec textOffset;
	NVGcolor color;
	LedDisplayTextField();
	void draw(NVGcontext *vg) override;
	int getTextPosition(math::Vec mousePos) override;
};


struct AudioIO;
struct MidiIO;

struct AudioWidget : LedDisplay {
	/** Not owned */
	AudioIO *audioIO = NULL;
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *sampleRateChoice;
	LedDisplaySeparator *sampleRateSeparator;
	LedDisplayChoice *bufferSizeChoice;
	AudioWidget();
	void step() override;
};

struct MidiWidget : LedDisplay {
	/** Not owned */
	MidiIO *midiIO = NULL;
	LedDisplayChoice *driverChoice;
	LedDisplaySeparator *driverSeparator;
	LedDisplayChoice *deviceChoice;
	LedDisplaySeparator *deviceSeparator;
	LedDisplayChoice *channelChoice;
	MidiWidget();
	void step() override;
};

////////////////////
// lights
////////////////////

struct LightWidget : TransparentWidget {
	NVGcolor bgColor = nvgRGBA(0, 0, 0, 0);
	NVGcolor color = nvgRGBA(0, 0, 0, 0);
	NVGcolor borderColor = nvgRGBA(0, 0, 0, 0);
	void draw(NVGcontext *vg) override;
	virtual void drawLight(NVGcontext *vg);
	virtual void drawHalo(NVGcontext *vg);
};

/** Mixes a list of colors based on a list of brightness values */
struct MultiLightWidget : LightWidget {
	/** Colors of each value state */
	std::vector<NVGcolor> baseColors;
	void addBaseColor(NVGcolor baseColor);
	/** Sets the color to a linear combination of the baseColors with the given weights */
	void setValues(const std::vector<float> &values);
};

/** A MultiLightWidget that points to a module's Light or a range of lights
Will access firstLightId, firstLightId + 1, etc. for each added color
*/
struct ModuleLightWidget : MultiLightWidget {
	Module *module = NULL;
	int firstLightId;
	void step() override;
};

////////////////////
// ports
////////////////////

struct Port : Component {
	enum PortType {
		INPUT,
		OUTPUT
	};
	PortType type = INPUT;
	int portId;
	MultiLightWidget *plugLight;

	Port();
	~Port();
	void step() override;
	void draw(NVGcontext *vg) override;
	void on(event::Button &e) override;
	void on(event::DragStart &e) override;
	void on(event::DragEnd &e) override;
	void on(event::DragDrop &e) override;
	void on(event::DragEnter &e) override;
	void on(event::DragLeave &e) override;
};

struct SVGPort : Port, FramebufferWidget {
	SVGWidget *background;
	CircularShadow *shadow;

	SVGPort();
	void setSVG(std::shared_ptr<SVG> svg);
	void draw(NVGcontext *vg) override;
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
	Slider *zoomSlider;
	RadioButton *cpuUsageButton;

	Toolbar();
	void draw(NVGcontext *vg) override;
};

struct PluginManagerWidget : virtual EventWidget {
	Widget *loginWidget;
	Widget *manageWidget;
	Widget *downloadWidget;
	PluginManagerWidget();
	void step() override;
};

struct RackScrollWidget : ScrollWidget {
	void step() override;
};

struct RackScene : Scene {
	ScrollWidget *scrollWidget;
	ZoomWidget *zoomWidget;

	RackScene();
	void step() override;
	void draw(NVGcontext *vg) override;
	void on(event::HoverKey &e) override;
	void on(event::PathDrop &e) override;
};

////////////////////
// globals
////////////////////

extern std::string gApplicationName;
extern std::string gApplicationVersion;
extern std::string gApiHost;
extern std::string gLatestVersion;
extern bool gCheckVersion;

// Easy access to "singleton" widgets
extern RackScene *gRackScene;
extern RackWidget *gRackWidget;
extern Toolbar *gToolbar;

void appInit(bool devMode);
void appDestroy();
void appModuleBrowserCreate();
json_t *appModuleBrowserToJson();
void appModuleBrowserFromJson(json_t *rootJ);


/** Deprecated. Will be removed in v1 */
json_t *colorToJson(NVGcolor color);
/** Deprecated. Will be removed in v1 */
NVGcolor jsonToColor(json_t *colorJ);


} // namespace rack
