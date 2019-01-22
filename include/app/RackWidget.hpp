#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "widgets/FramebufferWidget.hpp"
#include "app/CableContainer.hpp"
#include "app/ModuleWidget.hpp"


namespace rack {


struct RackWidget : OpaqueWidget {
	FramebufferWidget *rails;
	Widget *moduleContainer;
	CableContainer *cableContainer;
	/** The last mouse position in the RackWidget */
	math::Vec mousePos;

	RackWidget();
	~RackWidget();

	void addModule(ModuleWidget *mw);
	void addModuleAtMouse(ModuleWidget *mw);
	/** Removes the module and transfers ownership to the caller */
	void removeModule(ModuleWidget *mw);
	/** Sets a module's box if non-colliding. Returns true if set */
	bool requestModuleBox(ModuleWidget *mw, math::Rect requestedBox);
	/** Moves a module to the closest non-colliding position */
	bool requestModuleBoxNearest(ModuleWidget *mw, math::Rect requestedBox);
	ModuleWidget *getModule(int moduleId);

	/** Completely clear the rack's modules and cables */
	void clear();
	json_t *toJson();
	void fromJson(json_t *rootJ);
	void pastePresetClipboard();

	void step() override;
	void draw(NVGcontext *vg) override;

	void onHover(const event::Hover &e) override;
	void onDragHover(const event::DragHover &e) override;
	void onButton(const event::Button &e) override;
	void onZoom(const event::Zoom &e) override;
};


} // namespace rack
