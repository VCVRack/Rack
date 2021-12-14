#include "plugin.hpp"
#include <context.hpp>


namespace rack {
namespace core {


struct BlankModule : Module {
	int width = 10;

	/** Legacy for <=v1 patches */
	void fromJson(json_t* rootJ) override {
		Module::fromJson(rootJ);
		json_t* widthJ = json_object_get(rootJ, "width");
		if (widthJ)
			width = std::round(json_number_value(widthJ) / RACK_GRID_WIDTH);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "width", json_integer(width));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* widthJ = json_object_get(rootJ, "width");
		if (widthJ)
			width = json_integer_value(widthJ);
	}
};


struct BlankPanel : Widget {
	Widget* panelBorder;

	BlankPanel() {
		panelBorder = new PanelBorder;
		addChild(panelBorder);
	}

	void step() override {
		panelBorder->box.size = box.size;
		Widget::step();
	}

	void draw(const DrawArgs& args) override {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(args.vg, nvgRGB(0xe6, 0xe6, 0xe6));
		nvgFill(args.vg);
		Widget::draw(args);
	}
};


struct ModuleResizeHandle : OpaqueWidget {
	bool right = false;
	Vec dragPos;
	Rect originalBox;
	BlankModule* module;

	ModuleResizeHandle() {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
	}

	void onDragStart(const DragStartEvent& e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;

		dragPos = APP->scene->rack->getMousePos();
		ModuleWidget* mw = getAncestorOfType<ModuleWidget>();
		assert(mw);
		originalBox = mw->box;
	}

	void onDragMove(const DragMoveEvent& e) override {
		ModuleWidget* mw = getAncestorOfType<ModuleWidget>();
		assert(mw);

		Vec newDragPos = APP->scene->rack->getMousePos();
		float deltaX = newDragPos.x - dragPos.x;

		Rect newBox = originalBox;
		Rect oldBox = mw->box;
		const float minWidth = 3 * RACK_GRID_WIDTH;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = std::fmax(newBox.size.x, minWidth);
			newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = std::fmax(newBox.size.x, minWidth);
			newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}

		// Set box and test whether it's valid
		mw->box = newBox;
		if (!APP->scene->rack->requestModulePos(mw, newBox.pos)) {
			mw->box = oldBox;
		}
		module->width = std::round(mw->box.size.x / RACK_GRID_WIDTH);
	}

	void draw(const DrawArgs& args) override {
		for (float x = 5.0; x <= 10.0; x += 5.0) {
			nvgBeginPath(args.vg);
			const float margin = 5.0;
			nvgMoveTo(args.vg, x + 0.5, margin + 0.5);
			nvgLineTo(args.vg, x + 0.5, box.size.y - margin + 0.5);
			nvgStrokeWidth(args.vg, 1.0);
			nvgStrokeColor(args.vg, nvgRGBAf(0.5, 0.5, 0.5, 0.5));
			nvgStroke(args.vg);
		}
	}
};


struct BlankWidget : ModuleWidget {
	Widget* topRightScrew;
	Widget* bottomRightScrew;
	Widget* rightHandle;
	BlankPanel* blankPanel;

	BlankWidget(BlankModule* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * 10, RACK_GRID_HEIGHT);

		blankPanel = new BlankPanel;
		addChild(blankPanel);

		ModuleResizeHandle* leftHandle = new ModuleResizeHandle;
		leftHandle->module = module;
		addChild(leftHandle);

		ModuleResizeHandle* rightHandle = new ModuleResizeHandle;
		rightHandle->right = true;
		this->rightHandle = rightHandle;
		rightHandle->module = module;
		addChild(rightHandle);

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		topRightScrew = createWidget<ScrewSilver>(Vec(box.size.x - 30, 0));
		bottomRightScrew = createWidget<ScrewSilver>(Vec(box.size.x - 30, 365));
		addChild(topRightScrew);
		addChild(bottomRightScrew);

		// Set box width from loaded Module before adding to the RackWidget, so modules aren't unnecessarily shoved around.
		if (module) {
			box.size.x = module->width * RACK_GRID_WIDTH;
		}
	}

	void step() override {
		BlankModule* module = dynamic_cast<BlankModule*>(this->module);
		if (module) {
			box.size.x = module->width * RACK_GRID_WIDTH;
		}

		blankPanel->box.size = box.size;
		topRightScrew->box.pos.x = box.size.x - 30;
		bottomRightScrew->box.pos.x = box.size.x - 30;
		if (box.size.x < RACK_GRID_WIDTH * 6) {
			topRightScrew->hide();
			bottomRightScrew->hide();
		}
		else {
			topRightScrew->show();
			bottomRightScrew->show();
		}
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
		ModuleWidget::step();
	}
};


Model* modelBlank = createModel<BlankModule, BlankWidget>("Blank");


} // namespace core
} // namespace rack
