#include <app/ParamWidget.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/MenuSeparator.hpp>
#include <ui/TextField.hpp>
#include <app/Scene.hpp>
#include <context.hpp>
#include <engine/Engine.hpp>
#include <engine/ParamQuantity.hpp>
#include <settings.hpp>
#include <history.hpp>
#include <helpers.hpp>


namespace rack {
namespace app {


struct ParamField : ui::TextField {
	ParamWidget* paramWidget;

	void step() override {
		// Keep selected
		APP->event->setSelectedWidget(this);
		TextField::step();
	}

	void setParamWidget(ParamWidget* paramWidget) {
		this->paramWidget = paramWidget;
		engine::ParamQuantity* pq = paramWidget->getParamQuantity();
		if (pq)
			text = pq->getDisplayValueString();
		selectAll();
	}

	void onSelectKey(const SelectKeyEvent& e) override {
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


struct ParamValueItem : ui::MenuItem {
	ParamWidget* paramWidget;
	float value;

	void onAction(const ActionEvent& e) override {
		engine::ParamQuantity* pq = paramWidget->getParamQuantity();
		if (pq) {
			float oldValue = pq->getValue();
			pq->setValue(value);
			float newValue = pq->getValue();

			if (oldValue != newValue) {
				// Push ParamChange history action
				history::ParamChange* h = new history::ParamChange;
				h->name = "set parameter";
				h->moduleId = paramWidget->module->id;
				h->paramId = paramWidget->paramId;
				h->oldValue = oldValue;
				h->newValue = newValue;
				APP->history->push(h);
			}
		}
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


engine::ParamQuantity* ParamWidget::getParamQuantity() {
	if (!module)
		return NULL;
	return module->paramQuantities[paramId];
}


struct ParamWidget::Internal {
	ui::Tooltip* tooltip = NULL;
	/** For triggering the Change event. `*/
	float lastValue = NAN;
};


ParamWidget::ParamWidget() {
	internal = new Internal;
}


ParamWidget::~ParamWidget() {
	delete internal;
}


void ParamWidget::createTooltip() {
	if (!settings::tooltips)
		return;
	if (internal->tooltip)
		return;
	if (!module)
		return;
	ParamTooltip* tooltip = new ParamTooltip;
	tooltip->paramWidget = this;
	APP->scene->addChild(tooltip);
	internal->tooltip = tooltip;
}


void ParamWidget::destroyTooltip() {
	if (!internal->tooltip)
		return;
	APP->scene->removeChild(internal->tooltip);
	delete internal->tooltip;
	internal->tooltip = NULL;
}

void ParamWidget::step() {
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		float value = pq->getValue();
		// Dispatch change event when the ParamQuantity value changes
		if (value != internal->lastValue) {
			ChangeEvent eChange;
			onChange(eChange);
			internal->lastValue = value;
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


void ParamWidget::onButton(const ButtonEvent& e) {
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


void ParamWidget::onDoubleClick(const DoubleClickEvent& e) {
	resetAction();
}


void ParamWidget::onEnter(const EnterEvent& e) {
	createTooltip();
}


void ParamWidget::onLeave(const LeaveEvent& e) {
	destroyTooltip();
}


void ParamWidget::createContextMenu() {
	ui::Menu* menu = createMenu();

	engine::ParamQuantity* pq = getParamQuantity();
	engine::SwitchQuantity* switchQuantity = dynamic_cast<engine::SwitchQuantity*>(pq);

	ParamLabel* paramLabel = new ParamLabel;
	paramLabel->paramWidget = this;
	menu->addChild(paramLabel);

	if (switchQuantity) {
		float minValue = pq->getMinValue();
		int index = (int) std::floor(pq->getValue() - minValue);
		int numStates = switchQuantity->labels.size();
		for (int i = 0; i < numStates; i++) {
			std::string label = switchQuantity->labels[i];
			ParamValueItem* paramValueItem = createMenuItem<ParamValueItem>(label, CHECKMARK(i == index));
			paramValueItem->paramWidget = this;
			paramValueItem->value = minValue + i;
			menu->addChild(paramValueItem);
		}
		if (numStates > 0) {
			menu->addChild(new ui::MenuSeparator);
		}
	}
	else {
		ParamField* paramField = new ParamField;
		paramField->box.size.x = 100;
		paramField->setParamWidget(this);
		menu->addChild(paramField);
	}

	// Initialize
	if (pq && pq->resetEnabled && pq->isBounded()) {
		menu->addChild(createMenuItem("Initialize", switchQuantity ? "" : "Double-click", [=]() {
			this->resetAction();
		}));
	}

	// Fine
	if (!switchQuantity) {
		menu->addChild(createMenuItem("Fine adjust", RACK_MOD_CTRL_NAME "+drag", NULL, true));
	}

	// Unmap
	engine::ParamHandle* paramHandle = module ? APP->engine->getParamHandle(module->id, paramId) : NULL;
	if (paramHandle) {
		menu->addChild(createMenuItem("Unmap", paramHandle->text, [=]() {
			APP->engine->updateParamHandle(paramHandle, -1, 0);
		}));
	}

	appendContextMenu(menu);
}


void ParamWidget::resetAction() {
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq && pq->resetEnabled && pq->isBounded()) {
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
