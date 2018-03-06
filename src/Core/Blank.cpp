#include "Core.hpp"

using namespace rack;


struct ModuleResizeHandle : Widget {
	bool right = false;
	float dragX;
	Rect originalBox;
	ModuleResizeHandle() {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
	}
	void onMouseDown(EventMouseDown &e) override {
		if (e.button == 0) {
			e.consumed = true;
			e.target = this;
		}
	}
	void onDragStart(EventDragStart &e) override {
		dragX = gRackWidget->lastMousePos.x;
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();
		originalBox = m->box;
	}
	void onDragMove(EventDragMove &e) override {
		ModuleWidget *m = getAncestorOfType<ModuleWidget>();

		float newDragX = gRackWidget->lastMousePos.x;
		float deltaX = newDragX - dragX;

		Rect newBox = originalBox;
		const float minWidth = 3 * RACK_GRID_WIDTH;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = fmaxf(newBox.size.x, minWidth);
			newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}
		gRackWidget->requestModuleBox(m, newBox);
	}
	void draw(NVGcontext *vg) override {
		for (float x = 5.0; x <= 10.0; x += 5.0) {
			nvgBeginPath(vg);
			const float margin = 5.0;
			nvgMoveTo(vg, x + 0.5, margin + 0.5);
			nvgLineTo(vg, x + 0.5, box.size.y - margin + 0.5);
			nvgStrokeWidth(vg, 1.0);
			nvgStrokeColor(vg, nvgRGBAf(0.5, 0.5, 0.5, 0.5));
			nvgStroke(vg);
		}
	}
};


struct BlankWidget : ModuleWidget {
	Panel *panel;
	Widget *topRightScrew;
	Widget *bottomRightScrew;
	Widget *rightHandle;

	BlankWidget(Module *module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 10, RACK_GRID_HEIGHT);

		{
			panel = new LightPanel();
			panel->box.size = box.size;
			addChild(panel);
		}

		ModuleResizeHandle *leftHandle = new ModuleResizeHandle();
		ModuleResizeHandle *rightHandle = new ModuleResizeHandle();
		rightHandle->right = true;
		this->rightHandle = rightHandle;
		addChild(leftHandle);
		addChild(rightHandle);

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		topRightScrew = Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0));
		bottomRightScrew = Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365));
		addChild(topRightScrew);
		addChild(bottomRightScrew);
	}

	void step() override {
		panel->box.size = box.size;
		topRightScrew->box.pos.x = box.size.x - 30;
		bottomRightScrew->box.pos.x = box.size.x - 30;
		if (box.size.x < RACK_GRID_WIDTH * 6) {
			topRightScrew->visible = bottomRightScrew->visible = false;
		}
		else {
			topRightScrew->visible = bottomRightScrew->visible = true;
		}
		rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
		ModuleWidget::step();
	}

	json_t *toJson() override {
		json_t *rootJ = ModuleWidget::toJson();

		// // width
		json_object_set_new(rootJ, "width", json_real(box.size.x));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);

		// width
		json_t *widthJ = json_object_get(rootJ, "width");
		if (widthJ)
			box.size.x = json_number_value(widthJ);
	}
};


Model *modelBlank = Model::create<Module, BlankWidget>("Core", "Blank", "Blank", BLANK_TAG);
