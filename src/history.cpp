#include <history.hpp>
#include <app.hpp>
#include <app/Scene.hpp>
#include <engine/Cable.hpp>
#include <engine/Engine.hpp>


namespace rack {
namespace history {


ComplexAction::~ComplexAction() {
	for (Action* action : actions) {
		delete action;
	}
}

void ComplexAction::undo() {
	for (auto it = actions.rbegin(); it != actions.rend(); it++) {
		Action* action = *it;
		action->undo();
	}
}

void ComplexAction::redo() {
	for (Action* action : actions) {
		action->redo();
	}
}

void ComplexAction::push(Action* action) {
	actions.push_back(action);
}

bool ComplexAction::isEmpty() {
	return actions.empty();
}


ModuleAdd::~ModuleAdd() {
	json_decref(moduleJ);
}

void ModuleAdd::setModule(app::ModuleWidget* mw) {
	model = mw->model;
	assert(mw->module);
	moduleId = mw->module->id;
	pos = mw->box.pos;
	// ModuleAdd doesn't *really* need the state to be serialized, although ModuleRemove certainly does.
	// However, creating a module may give it a nondeterministic initial state for whatever reason, so serialize anyway.
	moduleJ = mw->toJson();
}

void ModuleAdd::undo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	APP->scene->rack->removeModule(mw);
	delete mw;
}

void ModuleAdd::redo() {
	app::ModuleWidget* mw = model->createModuleWidget();
	assert(mw);
	assert(mw->module);
	mw->module->id = moduleId;
	mw->box.pos = pos;
	mw->fromJson(moduleJ);
	APP->scene->rack->addModule(mw);
}


void ModuleMove::undo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	mw->box.pos = oldPos;
}

void ModuleMove::redo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	mw->box.pos = newPos;
}


void ModuleBypass::undo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	APP->engine->bypassModule(mw->module, !bypass);
}

void ModuleBypass::redo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	APP->engine->bypassModule(mw->module, bypass);
}


ModuleChange::~ModuleChange() {
	json_decref(oldModuleJ);
	json_decref(newModuleJ);
}

void ModuleChange::undo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	mw->fromJson(oldModuleJ);
}

void ModuleChange::redo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	mw->fromJson(newModuleJ);
}


void ParamChange::undo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	mw->module->params[paramId].value = oldValue;
}

void ParamChange::redo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	assert(mw);
	mw->module->params[paramId].value = newValue;
}


void CableAdd::setCable(app::CableWidget* cw) {
	assert(cw->cable);
	assert(cw->cable->id >= 0);
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
	app::CableWidget* cw = APP->scene->rack->getCable(cableId);
	APP->scene->rack->removeCable(cw);
	delete cw;
}

void CableAdd::redo() {
	app::CableWidget* cw = new app::CableWidget;
	cw->cable->id = cableId;

	app::ModuleWidget* outputModule = APP->scene->rack->getModule(outputModuleId);
	assert(outputModule);
	app::PortWidget* outputPort = outputModule->getOutput(outputId);
	assert(outputPort);
	cw->setOutput(outputPort);

	app::ModuleWidget* inputModule = APP->scene->rack->getModule(inputModuleId);
	assert(inputModule);
	app::PortWidget* inputPort = inputModule->getInput(inputId);
	assert(inputPort);
	cw->setInput(inputPort);

	cw->color = color;

	APP->scene->rack->addCable(cw);
}


State::State() {
	clear();
}

State::~State() {
	clear();
}

void State::clear() {
	for (Action* action : actions) {
		delete action;
	}
	actions.clear();
	actionIndex = 0;
	savedIndex = -1;
}

void State::push(Action* action) {
	for (int i = actionIndex; i < (int) actions.size(); i++) {
		delete actions[i];
	}
	actions.resize(actionIndex);
	actions.push_back(action);
	actionIndex++;
	// Unset the savedIndex if we just permanently overwrote the saved state
	if (actionIndex == savedIndex) {
		savedIndex = -1;
	}
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

std::string State::getUndoName() {
	if (!canUndo())
		return "";
	return actions[actionIndex - 1]->name;
}

std::string State::getRedoName() {
	if (!canRedo())
		return "";
	return actions[actionIndex]->name;
}

void State::setSaved() {
	savedIndex = actionIndex;
}

bool State::isSaved() {
	return actionIndex == savedIndex;
}


} // namespace history
} // namespace rack
