#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct ModuleDragType;

struct BaseWidget : ModuleWidget {
  Rect colourHotZone;
	float backgroundHue = 1.f;
	float backgroundSaturation = 1.f;
	float backgroundLuminosity = 0.25f;

  ModuleDragType *currentDragType = NULL;

  BaseWidget(Module *module);

	void onDragStart(EventDragStart& e) override;
	void onDragMove(EventDragMove& e) override;
	void onDragEnd(EventDragEnd& e) override;
  void draw(NVGcontext* ctx) override;

	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
};

} // namespace rack_plugin_rcm
