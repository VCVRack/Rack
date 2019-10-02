#include <app/ParamWidget.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/TextField.hpp>
#include <app/Scene.hpp>
#include <app.hpp>
#include <engine/Engine.hpp>
#include <settings.hpp>
#include <history.hpp>
#include <helpers.hpp>


namespace rack {
namespace app {


struct ParamField : ui::TextField {
	ParamWidget* paramWidget;

	void step() override {
		// Keep selected
		APP->event->setSelected(this);
		TextField::step();
	}

	void setParamWidget(ParamWidget* paramWidget) {
		this->paramWidget = paramWidget;
		if (paramWidget->paramQuantity)
			text = paramWidget->paramQuantity->getDisplayValueString();
		selectAll();
	}

	void onSelectKey(const event::SelectKey& e) override {
		if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
			float oldValue = paramWidget->paramQuantity->getValue();
			if (paramWidget->paramQuantity)
				paramWidget->paramQuantity->setDisplayValueString(text);
			float newValue = paramWidget->paramQuantity->getValue();

			if (oldValue != newValue) {
				// Push ParamChange history action
				history::ParamChange* h = new history::ParamChange;
				h->moduleId = paramWidget->paramQuantity->module->id;
				h->paramId = paramWidget->paramQuantity->paramId;
				h->oldValue = oldValue;
				h->newValue = newValue;
				APP->history->push(h);
			}

			ui::MenuOverlay* overlay = getAncestorOfType<ui::MenuOverlay>();
			overlay->requestDelete();
			e.consume(this);
		}

		if (!e.getTarget())
			TextField::onSelectKey(e);
	}
};


struct ParamTooltip : ui::Tooltip {
	ParamWidget* paramWidget;

	void step() override {
		if (paramWidget->paramQuantity) {
			// Quantity string
			text = paramWidget->paramQuantity->getString();
			// Param description
			std::string description = paramWidget->paramQuantity->description;
			if (!description.empty())
				text += "\n" + description;
		}
		Tooltip::step();
		// Position at bottom-right of parameter
		box.pos = paramWidget->getAbsoluteOffset(paramWidget->box.size).round();
		// Fit inside parent (copied from Tooltip.cpp)
		assert(parent);
		box = box.nudge(parent->box.zeroPos());
	}
};


struct ParamLabel : ui::MenuLabel {
	ParamWidget* paramWidget;
	void step() override {
		text = paramWidget->paramQuantity->getString();
		MenuLabel::step();
	}
};


struct ParamResetItem : ui::MenuItem {
	ParamWidget* paramWidget;
	void onAction(const event::Action& e) override {
		paramWidget->resetAction();
	}
};


struct ParamFineItem : ui::MenuItem {
};


struct ParamUnmapItem : ui::MenuItem {
	ParamWidget* paramWidget;
	void onAction(const event::Action& e) override {
		engine::ParamHandle* paramHandle = APP->engine->getParamHandle(paramWidget->paramQuantity->module->id, paramWidget->paramQuantity->paramId);
		if (paramHandle) {
			APP->engine->updateParamHandle(paramHandle, -1, 0);
		}
	}
};


void ParamWidget::step() {
	if (paramQuantity) {
		float value = paramQuantity->getValue();
		// Trigger change event when paramQuantity value changes
		if (value != dirtyValue) {
			dirtyValue = value;
			event::Change eChange;
			onChange(eChange);
		}
	}

	Widget::step();
}

void ParamWidget::draw(const DrawArgs& args) {
	Widget::draw(args);

	// Param map indicator
	engine::ParamHandle* paramHandle = paramQuantity ? APP->engine->getParamHandle(paramQuantity->module->id, paramQuantity->paramId) : NULL;
	if (paramHandle) {
		NVGcolor color = paramHandle->color;
		nvgBeginPath(args.vg);
		const float radius = 6;
		// nvgCircle(args.vg, box.size.x / 2, box.size.y / 2, radius);
		nvgRect(args.vg, box.size.x - radius, box.size.y - radius, radius, radius);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);
		nvgStrokeColor(args.vg, color::mult(color, 0.5));
		nvgStrokeWidth(args.vg, 1.0);
		nvgStroke(args.vg);
	}
}

void ParamWidget::onButton(const event::Button& e) {
	OpaqueWidget::onButton(e);

	// Touch parameter
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
		if (paramQuantity) {
			APP->scene->rack->touchedParam = this;
		}
		e.consume(this);
	}

	// Right click to open context menu
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
		createContextMenu();
		e.consume(this);
	}
}

void ParamWidget::onDoubleClick(const event::DoubleClick& e) {
	resetAction();
}

void ParamWidget::onEnter(const event::Enter& e) {
	if (settings::paramTooltip && !tooltip && paramQuantity) {
		ParamTooltip* paramTooltip = new ParamTooltip;
		paramTooltip->paramWidget = this;
		APP->scene->addChild(paramTooltip);
		tooltip = paramTooltip;
	}
}

void ParamWidget::onLeave(const event::Leave& e) {
	if (tooltip) {
		APP->scene->removeChild(tooltip);
		delete tooltip;
		tooltip = NULL;
	}
}

void ParamWidget::fromJson(json_t* rootJ) {
	json_t* valueJ = json_object_get(rootJ, "value");
	if (valueJ) {
		if (paramQuantity)
			paramQuantity->setValue(json_number_value(valueJ));
	}
}

void ParamWidget::createContextMenu() {
	ui::Menu* menu = createMenu();

	ParamLabel* paramLabel = new ParamLabel;
	paramLabel->paramWidget = this;
	menu->addChild(paramLabel);

	ParamField* paramField = new ParamField;
	paramField->box.size.x = 100;
	paramField->setParamWidget(this);
	menu->addChild(paramField);

	ParamResetItem* resetItem = new ParamResetItem;
	resetItem->text = "Initialize";
	resetItem->rightText = "Double-click";
	resetItem->paramWidget = this;
	menu->addChild(resetItem);

	// ParamFineItem *fineItem = new ParamFineItem;
	// fineItem->text = "Fine adjust";
	// fineItem->rightText = RACK_MOD_CTRL_NAME "+drag";
	// fineItem->disabled = true;
	// menu->addChild(fineItem);

	engine::ParamHandle* paramHandle = paramQuantity ? APP->engine->getParamHandle(paramQuantity->module->id, paramQuantity->paramId) : NULL;
	if (paramHandle) {
		ParamUnmapItem* unmapItem = new ParamUnmapItem;
		unmapItem->text = "Unmap";
		unmapItem->rightText = paramHandle->text;
		unmapItem->paramWidget = this;
		menu->addChild(unmapItem);
	}
}

void ParamWidget::resetAction() {
	if (paramQuantity && paramQuantity->isBounded()) {
		float oldValue = paramQuantity->getValue();
		reset();
		// Here's another way of doing it, but either works.
		// paramQuantity->getParam()->reset();
		float newValue = paramQuantity->getValue();

		if (oldValue != newValue) {
			// Push ParamChange history action
			history::ParamChange* h = new history::ParamChange;
			h->name = "reset parameter";
			h->moduleId = paramQuantity->module->id;
			h->paramId = paramQuantity->paramId;
			h->oldValue = oldValue;
			h->newValue = newValue;
			APP->history->push(h);
		}
	}
}


} // namespace app
} // namespace rack
