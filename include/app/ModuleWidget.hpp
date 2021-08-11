#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Menu.hpp>
#include <app/PortWidget.hpp>
#include <app/ParamWidget.hpp>
#include <plugin/Model.hpp>
#include <engine/Module.hpp>
#include <history.hpp>
#include <list>


namespace rack {
namespace app {


/** Manages an engine::Module in the rack. */
struct ModuleWidget : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	plugin::Model* model = NULL;
	/** Owned */
	engine::Module* module = NULL;

	ModuleWidget();
	DEPRECATED ModuleWidget(engine::Module* module) : ModuleWidget() {
		setModule(module);
	}
	~ModuleWidget();

	plugin::Model* getModel();
	void setModel(plugin::Model* model);

	engine::Module* getModule();
	/** Associates this ModuleWidget with the Module.
	Transfers ownership.
	*/
	void setModule(engine::Module* module);

	/** Sets the panel and sets the size of the ModuleWidget from the panel.
	Transfers ownership.
	*/
	void setPanel(widget::Widget* panel);
	/** Use `setPanel(createPanel(svg))` instead. */
	void setPanel(std::shared_ptr<Svg> svg);

	/** Convenience functions for adding special widgets.
	Just calls addChild() with additional checking.
	It is not required to call this method. You may instead use addChild() in a child widget for example.
	*/
	void addParam(ParamWidget* param);
	void addInput(PortWidget* input);
	void addOutput(PortWidget* output);
	/** Scans children widgets recursively for a ParamWidget with the given paramId. */
	ParamWidget* getParam(int paramId);
	PortWidget* getInput(int portId);
	PortWidget* getOutput(int portId);
	/** Scans children widgets recursively for all ParamWidgets. */
	std::list<ParamWidget*> getParams();
	std::list<PortWidget*> getPorts();
	std::list<PortWidget*> getInputs();
	std::list<PortWidget*> getOutputs();

	void draw(const DrawArgs& args) override;
	void drawShadow(const DrawArgs& args);

	/** Override to add context menu entries to your subclass.
	It is recommended to add a blank `ui::MenuSeparator` first for spacing.
	*/
	virtual void appendContextMenu(ui::Menu* menu) {}

	void onHover(const HoverEvent& e) override;
	void onHoverKey(const HoverKeyEvent& e) override;
	void onButton(const ButtonEvent& e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDragMove(const DragMoveEvent& e) override;
	void onDragHover(const DragHoverEvent& e) override;

	json_t* toJson();
	void fromJson(json_t* rootJ);
	void pasteJsonAction(json_t* rootJ);
	void copyClipboard();
	void pasteClipboardAction();
	void load(std::string filename);
	void loadAction(std::string filename);
	void loadTemplate();
	void loadDialog();
	void save(std::string filename);
	void saveTemplate();
	void saveTemplateDialog();
	bool hasTemplate();
	void clearTemplate();
	void clearTemplateDialog();
	void saveDialog();

	/** Disconnects cables from all ports
	Called when the user clicks Disconnect Cables in the context menu.
	*/
	void disconnect();

	/** Resets the parameters of the module and calls the Module's randomize().
	Called when the user clicks Initialize in the context menu.
	*/
	void resetAction();
	/** Randomizes the parameters of the module and calls the Module's randomize().
	Called when the user clicks Randomize in the context menu.
	*/
	void randomizeAction();
	void appendDisconnectActions(history::ComplexAction* complexAction);
	void disconnectAction();
	void cloneAction();
	void bypassAction(bool bypassed);
	/** Deletes `this` */
	void removeAction();
	void createContextMenu();

	INTERNAL math::Vec& dragOffset();
	INTERNAL bool& dragEnabled();
	INTERNAL math::Vec& oldPos();
	INTERNAL engine::Module* releaseModule();
	INTERNAL bool& selected();
};


} // namespace app
} // namespace rack
