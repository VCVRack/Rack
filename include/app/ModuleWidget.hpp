#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "ui/Menu.hpp"
#include "app/SVGPanel.hpp"
#include "app/Port.hpp"
#include "app/ParamWidget.hpp"
#include "plugin/Model.hpp"
#include "engine/Module.hpp"


namespace rack {


struct SVGPanel;
struct Port;


struct ModuleWidget : OpaqueWidget {
	Model *model = NULL;
	/** Owns the module pointer */
	Module *module = NULL;

	SVGPanel *panel = NULL;
	std::vector<Port*> inputs;
	std::vector<Port*> outputs;
	std::vector<ParamWidget*> params;

	ModuleWidget(Module *module);
	~ModuleWidget();
	/** Convenience functions for adding special widgets (calls addChild()) */
	void addInput(Port *input);
	void addOutput(Port *output);
	void addParam(ParamWidget *param);
	void setPanel(std::shared_ptr<SVG> svg);

	virtual json_t *toJson();
	virtual void fromJson(json_t *rootJ);

	void copyClipboard();
	void pasteClipboard();
	void save(std::string filename);
	void load(std::string filename);
	void loadDialog();
	void saveDialog();

	virtual void create();
	virtual void _delete();
	/** Disconnects cables from all ports
	Called when the user clicks Disconnect Cables in the context menu.
	*/
	virtual void disconnect();
	/** Resets the parameters of the module and calls the Module's randomize().
	Called when the user clicks Initialize in the context menu.
	*/
	virtual void reset();
	/** Deprecated */
	virtual void initialize() final {}
	/** Randomizes the parameters of the module and calls the Module's randomize().
	Called when the user clicks Randomize in the context menu.
	*/
	virtual void randomize();
	/** Do not subclass this to add context menu entries. Use appendContextMenu() instead */
	virtual Menu *createContextMenu();
	/** Override to add context menu entries to your subclass.
	It is recommended to add a blank MenuEntry first for spacing.
	*/
	virtual void appendContextMenu(Menu *menu) {}

	void draw(NVGcontext *vg) override;
	void drawShadow(NVGcontext *vg);

	Vec dragPos;
	void onHover(event::Hover &e) override;
	void onButton(event::Button &e) override;
	void onHoverKey(event::HoverKey &e) override;
	void onDragStart(event::DragStart &e) override;
	void onDragEnd(event::DragEnd &e) override;
	void onDragMove(event::DragMove &e) override;
};


} // namespace rack
