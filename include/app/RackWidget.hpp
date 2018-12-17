#pragma once
#include "app/common.hpp"
#include "app/WireWidget.hpp"
#include "app/WireContainer.hpp"


namespace rack {


struct RackWidget : OpaqueWidget {
	FramebufferWidget *rails;
	// Only put ModuleWidgets in here
	Widget *moduleContainer;
	// Only put WireWidgets in here
	WireContainer *wireContainer;
	std::string lastPath;
	Vec lastMousePos;
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
	bool requestModuleBox(ModuleWidget *m, Rect box);
	/** Moves a module to the closest non-colliding position */
	bool requestModuleBoxNearest(ModuleWidget *m, Rect box);

	void step() override;
	void draw(NVGcontext *vg) override;

	void onHover(event::Hover &e) override;
	void onButton(event::Button &e) override;
	void onZoom(event::Zoom &e) override;
};


} // namespace rack
