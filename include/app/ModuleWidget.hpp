#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "ui/Menu.hpp"
#include "app/PortWidget.hpp"
#include "app/ParamWidget.hpp"
#include "plugin/Model.hpp"
#include "engine/Module.hpp"


namespace rack {


struct ModuleWidget : OpaqueWidget {
	Model *model = NULL;
	/** Owns the module pointer */
	Module *module = NULL;

	Widget *panel = NULL;
	std::vector<ParamWidget*> params;
	std::vector<PortWidget*> outputs;
	std::vector<PortWidget*> inputs;
	/** For RackWidget dragging */
	math::Vec dragPos;
	math::Vec oldPos;

	ModuleWidget() {}
	DEPRECATED ModuleWidget(Module *module) {
		setModule(module);
	}
	~ModuleWidget();

	void draw(const DrawContext &ctx) override;
	void drawShadow(const DrawContext &ctx);

	void onHover(const event::Hover &e) override;
	void onButton(const event::Button &e) override;
	void onHoverKey(const event::HoverKey &e) override;
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void onDragMove(const event::DragMove &e) override;

	/** Associates this ModuleWidget with the Module
	Transfers ownership
	*/
	void setModule(Module *module);
	void setPanel(std::shared_ptr<SVG> svg);

	/** Convenience functions for adding special widgets (calls addChild()) */
	void addParam(ParamWidget *param);
	void addOutput(PortWidget *output);
	void addInput(PortWidget *input);
	ParamWidget *getParam(int paramId);
	PortWidget *getOutput(int outputId);
	PortWidget *getInput(int inputId);

	/** Overriding these is deprecated.
	Use Module::dataToJson() and dataFromJson() instead
	*/
	virtual json_t *toJson();
	virtual void fromJson(json_t *rootJ);

	/** Serializes/unserializes the module state */
	void copyClipboard();
	void pasteClipboardAction();
	void loadAction(std::string filename);
	void save(std::string filename);
	void loadDialog();
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
	void disconnectAction();
	void cloneAction();
	void bypassAction();
	/** Deletes `this` */
	void removeAction();
	void createContextMenu();
	/** Override to add context menu entries to your subclass.
	It is recommended to add a blank MenuEntry first for spacing.
	*/
	virtual void appendContextMenu(Menu *menu) {}
};


} // namespace rack
