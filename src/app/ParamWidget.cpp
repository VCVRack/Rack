#include <app/ParamWidget.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/TextField.hpp>
#include <app/Scene.hpp>
#include <context.hpp>
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
		engine::ParamQuantity* pq = paramWidget->getParamQuantity();
		if (pq)
			text = pq->getDisplayValueString();
		selectAll();
	}

	void onSelectKey(const event::SelectKey& e) override {
		if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) {
			engine::ParamQuantity* pq = paramWidget->getParamQuantity();
			assert(pq);
			float oldValue = pq->getValue();
			if (pq)
				pq->setDisplayValueString(text);
			float newValue = pq->getValue();

			if (oldValue != newValue) {
				// Push ParamChange history action
				history::ParamChange* h = new history::ParamChange;
				h->moduleId = paramWidget->module->id;
				h->paramId = paramWidget->paramId;
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
		engine::ParamQuantity* pq = paramWidget->getParamQuantity();
		if (pq) {
			// Quantity string
			text = pq->getString();
			// Description
			std::string description = pq->getDescription();
			if (description != "") {
				text += "\n";
				text += description;
			}
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
		engine::ParamQuantity* pq = paramWidget->getParamQuantity();
		text = pq->getString();
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
		engine::ParamHandle* paramHandle = APP->engine->getParamHandle(paramWidget->module->id, paramWidget->paramId);
		if (paramHandle) {
			APP->engine->updateParamHandle(paramHandle, -1, 0);
		}
	}
};


engine::ParamQuantity* ParamWidget::getParamQuantity() {
	if (!module)
		return NULL;
	return module->paramQuantities[paramId];
}

void ParamWidget::createTooltip() {
	if (settings::paramTooltip && !this->tooltip && module) {
		ParamTooltip* tooltip = new ParamTooltip;
		tooltip->paramWidget = this;
		APP->scene->addChild(tooltip);
		this->tooltip = tooltip;
	}
}

void ParamWidget::destroyTooltip() {
	if (tooltip) {
		APP->scene->removeChild(tooltip);
		delete tooltip;
		tooltip = NULL;
	}
}

void ParamWidget::step() {
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		float value = pq->getSmoothValue();
		// Trigger change event when the ParamQuantity value changes
		if (value != lastValue) {
			event::Change eChange;
			onChange(eChange);
			lastValue = value;
		}
	}

	Widget::step();
}

void ParamWidget::draw(const DrawArgs& args) {
	Widget::draw(args);

	// Param map indicator
	engine::ParamHandle* paramHandle = module ? APP->engine->getParamHandle(module->id, paramId) : NULL;
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
		if (module) {
			APP->scene->rack->touchedParam = this;
		}
		e.consume(this);
	}

	// Right click to open context menu
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
		destroyTooltip();
		createContextMenu();
		e.consume(this);
	}
}

void ParamWidget::onDoubleClick(const event::DoubleClick& e) {
	resetAction();
}

void ParamWidget::onEnter(const event::Enter& e) {
	createTooltip();
}

void ParamWidget::onLeave(const event::Leave& e) {
	destroyTooltip();
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

	engine::ParamHandle* paramHandle = module ? APP->engine->getParamHandle(module->id, paramId) : NULL;
	if (paramHandle) {
		ParamUnmapItem* unmapItem = new ParamUnmapItem;
		unmapItem->text = "Unmap";
		unmapItem->rightText = paramHandle->text;
		unmapItem->paramWidget = this;
		menu->addChild(unmapItem);
	}

	appendContextMenu(menu);
}

void ParamWidget::resetAction() {
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq && pq->resetEnabled) {
		float oldValue = pq->getValue();
		pq->reset();
		float newValue = pq->getValue();

		if (oldValue != newValue) {
			// Push ParamChange history action
			history::ParamChange* h = new history::ParamChange;
			h->name = "reset parameter";
			h->moduleId = module->id;
			h->paramId = paramId;
			h->oldValue = oldValue;
			h->newValue = newValue;
			APP->history->push(h);
		}
	}
}


} // namespace app
} // namespace rack
