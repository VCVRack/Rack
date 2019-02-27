#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/FramebufferWidget.hpp"
#include "app/ModuleWidget.hpp"
#include "app/CableWidget.hpp"
#include "app/PortWidget.hpp"
#include "app/ParamWidget.hpp"


namespace rack {
namespace app {


/** Container for ModuleWidget and CableWidget. */
struct RackWidget : widget::OpaqueWidget {
	widget::FramebufferWidget *rails;
	widget::Widget *moduleContainer;
	widget::Widget *cableContainer;
	CableWidget *incompleteCable = NULL;
	/** The last mouse position in the RackWidget */
	math::Vec mousePos;
	ParamWidget *touchedParam = NULL;

	RackWidget();
	~RackWidget();

	void step() override;
	void draw(const DrawArgs &args) override;

	void onHover(const widget::HoverEvent &e) override;
	void onHoverKey(const widget::HoverKeyEvent &e) override;
	void onDragHover(const widget::DragHoverEvent &e) override;
	void onButton(const widget::ButtonEvent &e) override;
	void onZoom(const widget::ZoomEvent &e) override;

	/** Completely clear the rack's modules and cables */
	void clear();
	json_t *toJson();
	void fromJson(json_t *rootJ);
	void pastePresetClipboardAction();

	// Module methods

	/** Adds a module and adds it to the Engine
	Ownership rules work like add/removeChild()
	*/
	void addModule(ModuleWidget *mw);
	void addModuleAtMouse(ModuleWidget *mw);
	/** Removes the module and transfers ownership to the caller */
	void removeModule(ModuleWidget *mw);
	/** Sets a module's box if non-colliding. Returns true if set */
	bool requestModuleBox(ModuleWidget *mw, math::Rect requestedBox);
	/** Moves a module to the closest non-colliding position */
	bool requestModuleBoxNearest(ModuleWidget *mw, math::Rect requestedBox);
	ModuleWidget *getModule(int moduleId);
	bool isEmpty();

	// Cable methods

	void clearCables();
	void clearCablesAction();
	/** Removes all complete cables connected to the port */
	void clearCablesOnPort(PortWidget *port);
	/** Adds a complete cable and adds it to the Engine.
	Ownership rules work like add/removeChild()
	*/
	void addCable(CableWidget *w);
	void removeCable(CableWidget *w);
	/** Takes ownership of `w` and adds it as a child if it isn't already */
	void setIncompleteCable(CableWidget *w);
	CableWidget *releaseIncompleteCable();
	/** Returns the most recently added complete cable connected to the given Port, i.e. the top of the stack */
	CableWidget *getTopCable(PortWidget *port);
	CableWidget *getCable(int cableId);
	/** Returns all cables attached to port, complete or not */
	std::list<CableWidget*> getCablesOnPort(PortWidget *port);
};


} // namespace app
} // namespace rack
