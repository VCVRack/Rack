#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <widget/FramebufferWidget.hpp>
#include <ui/Menu.hpp>
#include <app/RailWidget.hpp>
#include <app/ModuleWidget.hpp>
#include <app/CableWidget.hpp>
#include <app/PortWidget.hpp>
#include <app/ParamWidget.hpp>
#include <history.hpp>

#include <set>


namespace rack {
namespace app {


/** Container for ModuleWidget and CableWidget. */
struct RackWidget : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	CableWidget* incompleteCable = NULL;
	ParamWidget* touchedParam = NULL;
	int nextCableColorId = 0;

	RackWidget();
	~RackWidget();

	void step() override;
	void draw(const DrawArgs& args) override;

	void onHover(const HoverEvent& e) override;
	void onHoverKey(const HoverKeyEvent& e) override;
	void onButton(const ButtonEvent& e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDragHover(const DragHoverEvent& e) override;

	// Rack methods

	widget::Widget* getModuleContainer();
	widget::Widget* getCableContainer();
	math::Vec getMousePos();

	/** Completely clear the rack's modules and cables */
	void clear();
	void mergeJson(json_t* rootJ);
	void fromJson(json_t* rootJ);
	void pasteJsonAction(json_t* rootJ);
	void pasteModuleJsonAction(json_t* moduleJ);
	void pasteClipboardAction();

	// Module methods

	/** Adds a module and adds it to the Engine, adopting ownership.
	*/
	void addModule(ModuleWidget* mw);
	void addModuleAtMouse(ModuleWidget* mw);
	/** Removes the module and transfers ownership to the caller.
	*/
	void removeModule(ModuleWidget* mw);
	/** Sets a module's box if non-colliding. Returns true if set */
	bool requestModulePos(ModuleWidget* mw, math::Vec pos);
	/** Moves a module to the closest non-colliding position */
	void setModulePosNearest(ModuleWidget* mw, math::Vec pos);
	void setModulePosForce(ModuleWidget* mw, math::Vec pos);
	ModuleWidget* getModule(int64_t moduleId);
	std::vector<ModuleWidget*> getModules();
	bool hasModules();
	void updateModuleOldPositions();
	history::ComplexAction* getModuleDragAction();

	// Module selection methods

	void updateSelectionFromRect();
	void selectAll();
	void deselectAll();
	void select(ModuleWidget* mw, bool selected = true);
	bool hasSelection();
	const std::set<ModuleWidget*>& getSelected();
	bool isSelected(ModuleWidget* mw);
	json_t* selectionToJson(bool cables = true);
	void loadSelection(std::string path);
	void loadSelectionDialog();
	void saveSelection(std::string path);
	void saveSelectionDialog();
	void copyClipboardSelection();
	void resetSelectionAction();
	void randomizeSelectionAction();
	void disconnectSelectionAction();
	void cloneSelectionAction(bool cloneCables = true);
	void bypassSelectionAction(bool bypassed);
	bool isSelectionBypassed();
	void deleteSelectionAction();
	bool requestSelectionPos(math::Vec delta);
	void setSelectionPosNearest(math::Vec delta);
	void appendSelectionContextMenu(ui::Menu* menu);

	// Cable methods

	void clearCables();
	void clearCablesAction();
	/** Removes all cables connected to the port */
	void clearCablesOnPort(PortWidget* port);
	/** Adds a complete cable and adopts ownership.
	*/
	void addCable(CableWidget* cw);
	/** Removes cable and releases ownership to caller.
	*/
	void removeCable(CableWidget* cw);
	/** Takes ownership of `cw` and adds it as a child if it isn't already. */
	void setIncompleteCable(CableWidget* cw);
	CableWidget* releaseIncompleteCable();
	/** Returns the most recently added complete cable connected to the given Port, i.e. the top of the stack. */
	CableWidget* getTopCable(PortWidget* port);
	CableWidget* getCable(int64_t cableId);
	std::vector<CableWidget*> getCompleteCables();
	/** Returns all cables attached to port, complete or not. */
	std::vector<CableWidget*> getCablesOnPort(PortWidget* port);
};


} // namespace app
} // namespace rack
