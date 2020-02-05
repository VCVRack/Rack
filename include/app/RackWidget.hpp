#pragma once
#include <map>

#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <app/ModuleWidget.hpp>
#include <app/CableWidget.hpp>
#include <app/PortWidget.hpp>
#include <app/ParamWidget.hpp>
#include <history.hpp>


namespace rack {
namespace app {


/** Container for ModuleWidget and CableWidget. */
struct RackWidget : widget::OpaqueWidget {
	widget::Widget* moduleContainer;
	widget::Widget* cableContainer;
	CableWidget* incompleteCable = NULL;
	widget::FramebufferWidget* railFb;
	/** The last mouse position in the RackWidget */
	math::Vec mousePos;
	ParamWidget* touchedParam = NULL;
	int nextCableColorId = 0;

	RackWidget();
	~RackWidget();

	void step() override;
	void draw(const DrawArgs& args) override;

	void onHover(const event::Hover& e) override;
	void onHoverKey(const event::HoverKey& e) override;
	void onDragHover(const event::DragHover& e) override;
	void onButton(const event::Button& e) override;

	/** Completely clear the rack's modules and cables */
	void clear();
	void mergeJson(json_t* rootJ);
	void fromJson(json_t* rootJ);
	void pastePresetClipboardAction();

	// Module methods

	/** Adds a module and adds it to the Engine
	Ownership rules work like add/removeChild()
	*/
	void addModule(ModuleWidget* mw);
	void addModuleAtMouse(ModuleWidget* mw);
	/** Removes the module and transfers ownership to the caller */
	void removeModule(ModuleWidget* mw);
	/** Sets a module's box if non-colliding. Returns true if set */
	bool requestModulePos(ModuleWidget* mw, math::Vec pos);
	/** Moves a module to the closest non-colliding position */
	void setModulePosNearest(ModuleWidget* mw, math::Vec pos);
	void setModulePosForce(ModuleWidget* mw, math::Vec pos);
	ModuleWidget* getModule(int moduleId);
	bool isEmpty();
	void updateModuleOldPositions();
	history::ComplexAction* getModuleDragAction();

	// Cable methods

	void clearCables();
	void clearCablesAction();
	/** Removes all complete cables connected to the port */
	void clearCablesOnPort(PortWidget* port);
	/** Adds a complete cable.
	Ownership rules work like add/removeChild()
	*/
	void addCable(CableWidget* cw);
	void removeCable(CableWidget* cw);
	/** Takes ownership of `cw` and adds it as a child if it isn't already. */
	void setIncompleteCable(CableWidget* cw);
	CableWidget* releaseIncompleteCable();
	/** Returns the most recently added complete cable connected to the given Port, i.e. the top of the stack. */
	CableWidget* getTopCable(PortWidget* port);
	CableWidget* getCable(int cableId);
	/** Returns all cables attached to port, complete or not. */
	std::list<CableWidget*> getCablesOnPort(PortWidget* port);
};


} // namespace app
} // namespace rack
