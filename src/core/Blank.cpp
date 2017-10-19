#include "core.hpp"

using namespace rack;


struct ModuleResizeHandle : Widget {
	bool right = false;
	float originalWidth;
	float totalX;
	ModuleResizeHandle() {
		box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
	}
	Widget *onMouseDown(Vec pos, int button) override {
		if (button == 0)
			return this;
		return NULL;
	}
	void onDragStart() override {
		assert(parent);
		originalWidth = parent->box.size.x;
		totalX = 0.0;
	}
	void onDragMove(Vec mouseRel) override {
		ModuleWidget *m = dynamic_cast<ModuleWidget*>(parent);
		assert(m);
		totalX += mouseRel.x;
		float targetWidth = originalWidth;
		if (right)
			targetWidth += totalX;
		else
			targetWidth -= totalX;
		targetWidth = RACK_GRID_WIDTH * roundf(targetWidth / RACK_GRID_WIDTH);
		targetWidth = fmaxf(targetWidth, RACK_GRID_WIDTH * 3);
		Rect newBox = m->box;
		newBox.size.x = targetWidth;
		if (!right) {
			newBox.pos.x = m->box.pos.x + m->box.size.x - newBox.size.x;
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


BlankWidget::BlankWidget() {
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

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	topRightScrew = createScrew<ScrewSilver>(Vec(box.size.x - 30, 0));
	bottomRightScrew = createScrew<ScrewSilver>(Vec(box.size.x - 30, 365));
	addChild(topRightScrew);
	addChild(bottomRightScrew);
}

void BlankWidget::step() {
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

json_t *BlankWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();

	// // width
	json_object_set_new(rootJ, "width", json_real(box.size.x));

	return rootJ;
}

void BlankWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);

	// width
	json_t *widthJ = json_object_get(rootJ, "width");
	if (widthJ)
		box.size.x = json_number_value(widthJ);
}
