#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Menu.hpp>
#include <app/PortWidget.hpp>
#include <app/ParamWidget.hpp>
#include <plugin/Model.hpp>
#include <engine/Module.hpp>
#include <history.hpp>


namespace rack {
namespace app {


/** Manages an engine::Module in the rack. */
struct ModuleWidget : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	/** Not owned */
	plugin::Model* model = NULL;
	/** Owned */
	engine::Module* module = NULL;

	ModuleWidget();
	DEPRECATED ModuleWidget(engine::Module* module) : ModuleWidget() {
		setModule(module);
	}
	~ModuleWidget();

	/** Returns the Model instance of this ModuleWidget. */
	plugin::Model* getModel();
	void setModel(plugin::Model* model);

	/** Returns Module attached to this ModuleWidget. */
	engine::Module* getModule();
	/** Returns Module attached to this ModuleWidget, casted to the given Module type. */
	template <class TModule>
	TModule* getModule() {
		return dynamic_cast<TModule*>(getModule());
	}
	/** Associates this ModuleWidget with the Module.
	Transfers ownership to `this`.
	*/
	void setModule(engine::Module* module);

	widget::Widget* getPanel();
	/** Sets the panel and sets the size of the ModuleWidget from the panel.
	Transfers ownership.
	*/
	void setPanel(widget::Widget* panel);
	void setPanel(std::shared_ptr<window::Svg> svg);

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
	std::vector<ParamWidget*> getParams();
	std::vector<PortWidget*> getPorts();
	std::vector<PortWidget*> getInputs();
	std::vector<PortWidget*> getOutputs();

	void draw(const DrawArgs& args) override;
	void drawLayer(const DrawArgs& args, int layer) override;

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
	/** Returns whether paste was successful. */
	bool pasteJsonAction(json_t* rootJ);
	void copyClipboard();
	bool pasteClipboardAction();
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
	void cloneAction(bool cloneCables = true);
	void bypassAction(bool bypassed);
	/** Deletes `this` */
	void removeAction();
	void createContextMenu();

	// Returns the rack position in grid coordinates
	math::Vec getGridPosition();
	void setGridPosition(math::Vec pos);
	math::Vec getGridSize();
	math::Rect getGridBox();

	PRIVATE math::Vec& dragOffset();
	PRIVATE bool& dragEnabled();
	PRIVATE engine::Module* releaseModule();
};


} // namespace app
} // namespace rack
