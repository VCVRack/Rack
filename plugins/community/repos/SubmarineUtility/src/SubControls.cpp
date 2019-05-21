#include "SubControls.hpp"
#include "global_pre.hpp"
#include "global_ui.hpp"
#include "window.hpp"

namespace rack_plugin_SubmarineUtility {

namespace SubControls {

struct RowShift {
	Vec position;
	int handled = false; 
};

struct RowShifter {
	std::vector<std::shared_ptr<RowShift>> rows;
	Widget *baseWidget;
	unsigned int addRow(Vec position) {
		// Search for row in existing list
		for (std::shared_ptr<RowShift> row : rows) {
			if (row->position.y != position.y)
				continue;			// This is not the row we are looking for
			if (row->handled)
				return false;			// This is the row but it's been done already
			if (row->position.x <= position.x)
				return false;			// This is the row already and covers the same ground
			row->position.x = position.x;		// We need to move the start point further to the left
			return true;
		}
		std::shared_ptr<RowShift> row = std::make_shared<RowShift>();
		row->position.x = position.x;
		row->position.y = position.y;
		rows.push_back(row);
		return true;					// We didn't find the row so we added it.
	}	
	unsigned int shift(float delta) {
		unsigned moreWork = false;
		for (std::shared_ptr<RowShift> row : rows) {
			if (row->handled)
				continue;			// This row has been done already
			row->handled = true;
			for (Widget *w : RACK_PLUGIN_UI_RACKWIDGET->moduleContainer->children) {
				if (baseWidget == w)
					continue;		// We are not moving the widget that caused this.
				if (row->position.x > w->box.pos.x)
					continue;		// This is to the left of our start position
				if (row->position.y != w->box.pos.y) {
					if (row->position.y > w->box.pos.y) {
						if (row->position.y < w->box.pos.y + w->box.size.y) {
							for (float i = 0; i < w->box.size.y; i += RACK_GRID_HEIGHT) {
								Vec newRow;
								newRow.x = w->box.pos.x;
								newRow.y = w->box.pos.y + i;
								moreWork |= addRow(newRow);	// These are the rows covered by a multi-row widget
							}
						}
					}
					continue;
				}
				if (w->box.size.y > RACK_GRID_HEIGHT) {
					for (float i = 0; i < w->box.size.y; i += RACK_GRID_HEIGHT) {
						Vec newRow;
						newRow.x = w->box.pos.x;
						newRow.y = w->box.pos.y + i;
						moreWork |= addRow(newRow);			// These are the rows covered by a multi row widget
					}
				}
				w->box.pos.x += delta;						// This widget should be moved
			}
		}
		return moreWork;
	}
	void shortShiftRight(float endPoint) {
		Widget *widgetToMove = baseWidget;
		std::shared_ptr<RowShift> row = rows[0];
		for (Widget *w : RACK_PLUGIN_UI_RACKWIDGET->moduleContainer->children) {
			if (w->box.pos.y != row->position.y)
				continue;			// This is not the row we are looking for
			if (w->box.pos.x <= row->position.x)	
				continue;			// This is to the left of the region we are looking at
			if (w->box.pos.x >= endPoint)
				continue;			// This is to the right of the region we are looking at
			if (w->box.pos.x > widgetToMove->box.pos.x)
				widgetToMove = w;
		}
		if (widgetToMove == baseWidget)
			return;					// Nothing more to move
		float delta = endPoint - widgetToMove->box.pos.x - widgetToMove->box.size.x;
		widgetToMove->box.pos.x += delta;		// Move this widget as far to the right as we can
		endPoint -= widgetToMove->box.size.x;		// Adjust our endpoint back to exclude this widget
		shortShiftRight(endPoint);			// Recurse to move the next widget
	}
	void shortShiftLeft(float delta, float endPoint) {
		std::shared_ptr<RowShift> row = rows[0];
		for (Widget *w : RACK_PLUGIN_UI_RACKWIDGET->moduleContainer->children) {
			if (w->box.pos.y != row->position.y)
				continue;			// This is not the row we are looking for
			if (w->box.pos.x  <= row->position.x)	
				continue;			// This is to the left of the region we are looking at
			if (w->box.pos.x >= endPoint)
				continue;			// This is to the right of the region we are looking at
			w->box.pos.x += delta;
		}
	}
	void process(float delta) {
		// Test to see if any multi-row items are involved
		float endPoint = 0.0f;
		std::shared_ptr<RowShift> row = rows[0];
		for (Widget *w : RACK_PLUGIN_UI_RACKWIDGET->moduleContainer->children) {
			if (w->box.size.y == RACK_GRID_HEIGHT)
				continue;			// This is a single row widget
			if (w->box.pos.y > row->position.y)
				continue;			// This is below the row
			if ((w->box.pos.y + w->box.size.y) > (row->position.y + RACK_GRID_HEIGHT)) {	// This is what we are looking for
				if ((endPoint == 0.0f) || endPoint > w->box.pos.x) {
					endPoint = w->box.pos.x;
				}
			}
		}	
		if (endPoint > 0.0f) {			// There is a multi-row element in the way
			if (delta < 0.0f) {
				shortShiftLeft(delta, endPoint);	// Just shift upto the endPoint
				return;
			}
			// Have we got enough space to shuffle up before the multi-row element
			float space = endPoint - baseWidget->box.pos.x - baseWidget->box.size.x;
			debug("Space %f", space);
			for (Widget *w : RACK_PLUGIN_UI_RACKWIDGET->moduleContainer->children) {
				if (w->box.pos.y != row->position.y)
					continue;			// This is not the row we are looking for
				if (w->box.pos.x <= row->position.x)	
					continue;			// This is to the left of the region we are looking at
				if (w->box.pos.x >= endPoint)
					continue;			// This is to the right of the region we are looking at
				space -= w->box.size.x;
				debug("Space adjusted to %f", space);
			}
			debug("Space %f delta %f", space, delta);
			if (space >= delta) {
				shortShiftRight(endPoint);		// Just shift upto the endPoint
				return;
			}
		}
		while (shift(delta));
	}
};

SizeableModuleWidget::SizeableModuleWidget(Module *module) : ModuleWidget(module) {
	box.size.x = moduleWidth;
	box.size.y = 380;
	handle = Widget::create<ModuleDragHandle>(Vec(box.size.x - 10, 175));
	handle->smw = this;
	addChild(handle);

	backPanel = Widget::create<BackPanel>(Vec(10, 15));
	backPanel->box.size.x = box.size.x - 20;
	backPanel->box.size.y = box.size.y - 30;
	addChild(backPanel);

	minimizeLogo = Widget::create<SubLogo>(Vec(0,0));
#define plugin "SubmarineUtility"
	minimizeLogo->setSVG(SVG::load(assetPlugin(plugin, "res/Sub2.svg")));
	minimizeLogo->visible = false;
	addChild(minimizeLogo);

	maximizeLogo = Widget::create<SubLogo>(Vec(moduleWidth - 20, 365));
	maximizeLogo->setSVG(SVG::load(assetPlugin(plugin, "res/Sub1.svg")));
	addChild(maximizeLogo);

	maximizeButton = Widget::create<MaximizeButton>(Vec(0, 175));
	maximizeButton->smw = this;
	maximizeButton->visible = false;
	addChild(maximizeButton);
}

void SizeableModuleWidget::Resize() {
	backPanel->box.size.x = box.size.x - 20;
	handle->box.pos.x = box.size.x - 10;
	maximizeLogo->box.pos.x = box.size.x - 20;
	handle->visible = sizeable;
	onResize();
}

void SizeableModuleWidget::draw(NVGcontext *vg) {
	nvgBeginPath(vg);
	nvgRect(vg,0,0,box.size.x, box.size.y);
	nvgFillColor(vg,nvgRGB(0x29, 0x4f, 0x77));
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, 0, 0);
	nvgLineTo(vg, box.size.x, 0);
	nvgLineTo(vg, box.size.x - 1, 1);
	nvgLineTo(vg, 1, 1);
	nvgClosePath(vg);
	nvgMoveTo(vg, 1, 1);
	nvgLineTo(vg, 1, box.size.y - 1);
	nvgLineTo(vg, 0, box.size.y);
	nvgLineTo(vg, 0, 0);
	nvgClosePath(vg);
	nvgFillColor(vg, nvgRGB(0x3a, 0x6e, 0xa5));
	nvgFill(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, box.size.x, 0);
	nvgLineTo(vg, box.size.x, box.size.y);
	nvgLineTo(vg, box.size.x - 1, box.size.y - 1);
	nvgLineTo(vg, box.size.x -1, 1);
	nvgClosePath(vg);
	nvgMoveTo(vg, box.size.x, box.size.y);
	nvgLineTo(vg, 0, box.size.y);
	nvgLineTo(vg, 1, box.size.y - 1);
	nvgLineTo(vg, box.size.x - 1, box.size.y - 1);
	nvgClosePath(vg);
	nvgFillColor(vg, nvgRGB(0x18, 0x2d, 0x44));
	nvgFill(vg);

	if (moduleWidth > 0) {
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(0x71, 0x9f, 0xcf, 0xff));
		nvgTextAlign(vg, NVG_ALIGN_LEFT);
		nvgText(vg, 3, 378, "submarine", NULL);
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgText(vg, box.size.x / 2, 12, moduleName.c_str(), NULL);
	}
	else {
		nvgSave(vg);
		nvgRotate(vg, -M_PI / 2);
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(0x71, 0x9f, 0xcf, 0xff));
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, -97.5, 7.5, moduleName.c_str(), NULL);
		nvgText(vg, -277.5, 7.5, "submarine", NULL);
		nvgRestore(vg);
	}
	ModuleWidget::draw(vg);
}

void SizeableModuleWidget::ShiftOthers(float delta) {
	if (!stabilized)
		return;
	if (delta == 0.0f)
		return;
	RowShifter shifter;
	shifter.baseWidget = this;
	shifter.addRow(this->box.pos);
	shifter.process(delta);
}

void SizeableModuleWidget::Minimize(unsigned int minimize) {
	float oldSize = box.size.x;
	if (minimize) {
		if (moduleWidth > 0)
			moduleWidth = -moduleWidth;
		box.size.x = 15;
		backPanel->visible = false;
		maximizeButton->visible = true;
		maximizeLogo->visible = false;
		minimizeLogo->visible = true;
		handle->visible = false;
		ShiftOthers(box.size.x - oldSize);
	}
	else {
		if (moduleWidth < 0)
			moduleWidth = -moduleWidth;
		ShiftOthers(moduleWidth - oldSize);
		box.size.x = moduleWidth;
		backPanel->visible = true;
		maximizeButton->visible = false;
		maximizeLogo->visible = true;
		minimizeLogo->visible = false;
		handle->visible = sizeable;
		Resize();
	}
}

json_t *SizeableModuleWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();
	
	// moduleWidth
	json_object_set_new (rootJ, "width", json_real(moduleWidth));

	return rootJ;
}

void SizeableModuleWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);

	// width
	json_t *widthJ = json_object_get(rootJ, "width");
	if (widthJ)
		moduleWidth = json_number_value(widthJ);
	Minimize(moduleWidth < 0);
}

void ModuleDragHandle::onDragStart(EventDragStart &e) {
	dragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
	originalBox = smw->box;
}

void ModuleDragHandle::onDragMove(EventDragMove &e) {

	float newDragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
	float deltaX = newDragX - dragX;

	Rect newBox = originalBox;
	newBox.size.x += deltaX;
	newBox.size.x = fmaxf(newBox.size.x, smw->minimumWidth);
	newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
	RACK_PLUGIN_UI_RACKWIDGET->requestModuleBox(smw, newBox);
	smw->moduleWidth = smw->box.size.x;
	smw->Resize();
}

void ModuleDragHandle::draw(NVGcontext *vg) {
	for (float x = 2.0; x <= 8.0; x += 2.0) {
		nvgBeginPath(vg);
		const float margin = 5.0;
		nvgMoveTo(vg, x + 0.5, margin + 0.5);
		nvgLineTo(vg, x + 0.5, box.size.y - margin + 0.5);
		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, nvgRGBAf(0.5, 0.5, 0.5, 0.5));
		nvgStroke(vg);
	}
}

void MaximizeButton::onAction(EventAction &e) {
	smw->Minimize(false);
}

} // SubControls

} // namespace rack_plugin_SubmarineUtility
