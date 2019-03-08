#include <tuple>
#include <limits>

#include "rack.hpp"
#include "../BaseWidget.hpp"
#include "RollAreaWidget.hpp"

using namespace rack;

namespace rack_plugin_rcm {

struct PianoRollModule;
struct PianoRollWidget;
struct ModuleDragType;

enum CopyPasteState {
	COPYREADY,
	PATTERNLOADED,
	MEASURELOADED
};

struct PianoRollWidget : BaseWidget {
	PianoRollModule* module;
	CopyPasteState state;
	RollAreaWidget* rollAreaWidget;

	PianoRollWidget(PianoRollModule *module);

	Rect getRollArea();

	// Event Handlers

	void appendContextMenu(Menu* menu) override;

	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;

};

} // namespace rack_plugin_rcm
