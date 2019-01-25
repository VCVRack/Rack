#include "history.hpp"
#include "app.hpp"
#include "app/Scene.hpp"
#include "engine/Cable.hpp"
#include "engine/Engine.hpp"


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


ModuleAdd::~ModuleAdd() {
	json_decref(moduleJ);
}

void ModuleAdd::setModule(ModuleWidget *mw) {
	model = mw->model;
	assert(mw->module);
	moduleId = mw->module->id;
	pos = mw->box.pos;
	// ModuleAdd doesn't *really* need the state to be serialized, although ModuleRemove certainly does.
	// However, creating a module may give it a nondeterministic initial state for whatever reason, so serialize anyway.
	moduleJ = mw->toJson();
}

void ModuleAdd::undo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	app()->scene->rackWidget->removeModule(mw);
	delete mw;
}

void ModuleAdd::redo() {
	ModuleWidget *mw = model->createModuleWidget();
	assert(mw);
	assert(mw->module);
	mw->module->id = moduleId;
	mw->box.pos = pos;
	mw->fromJson(moduleJ);
	app()->scene->rackWidget->addModule(mw);
}


void ModuleMove::undo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	mw->box.pos = oldPos;
}

void ModuleMove::redo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	mw->box.pos = newPos;
}


void ModuleBypass::undo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	app()->engine->bypassModule(mw->module, !bypass);
}

void ModuleBypass::redo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	app()->engine->bypassModule(mw->module, bypass);
}


ModuleChange::~ModuleChange() {
	json_decref(oldModuleJ);
	json_decref(newModuleJ);
}

void ModuleChange::undo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	mw->fromJson(oldModuleJ);
}

void ModuleChange::redo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	mw->fromJson(newModuleJ);
}


void ParamChange::undo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	mw->module->params[paramId].value = oldValue;
}

void ParamChange::redo() {
	ModuleWidget *mw = app()->scene->rackWidget->getModule(moduleId);
	assert(mw);
	mw->module->params[paramId].value = newValue;
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
	clear();
}

void State::clear() {
	for (Action *action : actions) {
		delete action;
	}
	actions.clear();
	actionIndex = 0;
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
	if (canUndo()) {
		actionIndex--;
		actions[actionIndex]->undo();
	}
}

void State::redo() {
	if (canRedo()) {
		actions[actionIndex]->redo();
		actionIndex++;
	}
}

bool State::canUndo() {
	return actionIndex > 0;
}

bool State::canRedo() {
	return actionIndex < (int) actions.size();
}


} // namespace history
} // namespace rack
