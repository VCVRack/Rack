#pragma once
#include "ui.hpp"
#include "app/AudioWidget.hpp"
#include "app/CircularShadow.hpp"
#include "app/common.hpp"
#include "app/Knob.hpp"
#include "app/LedDisplay.hpp"
#include "app/LightWidget.hpp"
#include "app/MidiWidget.hpp"
#include "app/ModuleLightWidget.hpp"
#include "app/ModuleWidget.hpp"
#include "app/MomentarySwitch.hpp"
#include "app/MultiLightWidget.hpp"
#include "app/ParamWidget.hpp"
#include "app/PluginManagerWidget.hpp"
#include "app/Port.hpp"
#include "app/RackRail.hpp"
#include "app/RackScene.hpp"
#include "app/RackScrollWidget.hpp"
#include "app/RackWidget.hpp"
#include "app/SVGButton.hpp"
#include "app/SVGKnob.hpp"
#include "app/SVGPanel.hpp"
#include "app/SVGPort.hpp"
#include "app/SVGScrew.hpp"
#include "app/SVGSlider.hpp"
#include "app/SVGSwitch.hpp"
#include "app/ToggleSwitch.hpp"
#include "app/Toolbar.hpp"
#include "app/WireContainer.hpp"
#include "app/WireWidget.hpp"


namespace rack {

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

} // namespace rack
