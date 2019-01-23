#include "history.hpp"
#include "app.hpp"
#include "app/Scene.hpp"
#include "engine/Cable.hpp"


namespace rack {
namespace history {


ComplexAction::~ComplexAction() {
	for (Action *action : actions) {
		delete action;
	}
}

void ComplexAction::undo() {
	for (auto it = actions.rbegin(); it != actions.rend(); it++) {
		Action *action = *it;
		action->undo();
	}
}

void ComplexAction::redo() {
	for (Action *action : actions) {
		action->redo();
	}
}

void ComplexAction::push(Action *action) {
	actions.push_back(action);
}


void ModuleAdd::undo() {
	ModuleWidget *moduleWidget = app()->scene->rackWidget->getModule(moduleId);
	assert(moduleWidget);
	app()->scene->rackWidget->removeModule(moduleWidget);
	delete moduleWidget;
}

void ModuleAdd::redo() {
	ModuleWidget *moduleWidget = model->createModuleWidget();
	assert(moduleWidget);
	assert(moduleWidget->module);
	moduleWidget->module->id = moduleId;
	moduleWidget->box.pos = pos;
	app()->scene->rackWidget->addModule(moduleWidget);
}


ModuleRemove::~ModuleRemove() {
	json_decref(moduleJ);
}

void ModuleRemove::undo() {
	ModuleWidget *moduleWidget = model->createModuleWidget();
	assert(moduleWidget);
	assert(moduleWidget->module);
	moduleWidget->module->id = moduleId;
	moduleWidget->box.pos = pos;
	moduleWidget->fromJson(moduleJ);
	app()->scene->rackWidget->addModule(moduleWidget);
}

void ModuleRemove::redo() {
	ModuleWidget *moduleWidget = app()->scene->rackWidget->getModule(moduleId);
	assert(moduleWidget);
	app()->scene->rackWidget->removeModule(moduleWidget);
	delete moduleWidget;
}


void ModuleMove::undo() {
	ModuleWidget *moduleWidget = app()->scene->rackWidget->getModule(moduleId);
	assert(moduleWidget);
	moduleWidget->box.pos = oldPos;
}

void ModuleMove::redo() {
	ModuleWidget *moduleWidget = app()->scene->rackWidget->getModule(moduleId);
	assert(moduleWidget);
	moduleWidget->box.pos = newPos;
}


void ModuleBypass::undo() {
	ModuleWidget *moduleWidget = app()->scene->rackWidget->getModule(moduleId);
	assert(moduleWidget);
	moduleWidget->module->bypass = !bypass;
}

void ModuleBypass::redo() {
	ModuleWidget *moduleWidget = app()->scene->rackWidget->getModule(moduleId);
	assert(moduleWidget);
	moduleWidget->module->bypass = bypass;
}


void ParamChange::undo() {
	ModuleWidget *moduleWidget = app()->scene->rackWidget->getModule(moduleId);
	assert(moduleWidget);
	moduleWidget->module->params[paramId].value = oldValue;
}

void ParamChange::redo() {
	ModuleWidget *moduleWidget = app()->scene->rackWidget->getModule(moduleId);
	assert(moduleWidget);
	moduleWidget->module->params[paramId].value = newValue;
}


void CableAdd::setCable(CableWidget *cw) {
	assert(cw->cable);
	assert(cw->cable->id > 0);
	cableId = cw->cable->id;
	assert(cw->cable->outputModule);
	outputModuleId = cw->cable->outputModule->id;
	outputId = cw->cable->outputId;
	assert(cw->cable->inputModule);
	inputModuleId = cw->cable->inputModule->id;
	inputId = cw->cable->inputId;
	color = cw->color;
}

void CableAdd::undo() {
	CableWidget *cw = app()->scene->rackWidget->getCable(cableId);
	app()->scene->rackWidget->removeCable(cw);
	delete cw;
}

void CableAdd::redo() {
	CableWidget *cw = new CableWidget;
	cw->cable->id = cableId;

	ModuleWidget *outputModule = app()->scene->rackWidget->getModule(outputModuleId);
	assert(outputModule);
	PortWidget *outputPort = outputModule->getOutput(outputId);
	assert(outputPort);
	cw->setOutput(outputPort);

	ModuleWidget *inputModule = app()->scene->rackWidget->getModule(inputModuleId);
	assert(inputModule);
	PortWidget *inputPort = inputModule->getInput(inputId);
	assert(inputPort);
	cw->setInput(inputPort);

	cw->color = color;

	app()->scene->rackWidget->addCable(cw);
}


State::~State() {
	for (Action *action : actions) {
		delete action;
	}
}

void State::push(Action *action) {
	for (int i = actionIndex; i < (int) actions.size(); i++) {
		delete actions[i];
	}
	actions.resize(actionIndex);
	actions.push_back(action);
	actionIndex++;
}

void State::undo() {
	if (actionIndex > 0) {
		actionIndex--;
		actions[actionIndex]->undo();
	}
}

void State::redo() {
	if ((int) actions.size() > actionIndex) {
		actions[actionIndex]->redo();
		actionIndex++;
	}
}


} // namespace history
} // namespace rack
