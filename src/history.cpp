#include <history.hpp>
#include <context.hpp>
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
	assert(mw);
	model = mw->getModel();
	assert(mw->getModule());
	moduleId = mw->getModule()->id;
	pos = mw->box.pos;
	// ModuleAdd doesn't *really* need the state to be serialized, although ModuleRemove certainly does.
	// However, creating a module may give it a nondeterministic initial state for whatever reason, so serialize anyway.
	moduleJ = APP->engine->moduleToJson(mw->getModule());
}

void ModuleAdd::undo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	if (!mw)
		return;
	APP->scene->rack->removeModule(mw);
	delete mw;
}

void ModuleAdd::redo() {
	INFO("Creating module %s", model->getFullName().c_str());
	engine::Module* module = model->createModule();
	module->id = moduleId;
	try {
		module->fromJson(moduleJ);
	}
	catch (Exception& e) {
		WARN("%s", e.what());
	}
	APP->engine->addModule(module);

	INFO("Creating module widget %s", model->getFullName().c_str());
	app::ModuleWidget* mw = model->createModuleWidget(module);
	mw->box.pos = pos;
	APP->scene->rack->addModule(mw);
}


void ModuleMove::undo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	if (!mw)
		return;
	mw->box.pos = oldPos;
	APP->scene->rack->updateExpanders();
}

void ModuleMove::redo() {
	app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
	if (!mw)
		return;
	mw->box.pos = newPos;
	APP->scene->rack->updateExpanders();
}


void ModuleBypass::undo() {
	engine::Module* module = APP->engine->getModule(moduleId);
	if (!module)
		return;
	APP->engine->bypassModule(module, !bypassed);
}

void ModuleBypass::redo() {
	engine::Module* module = APP->engine->getModule(moduleId);
	if (!module)
		return;
	APP->engine->bypassModule(module, bypassed);
}


ModuleChange::~ModuleChange() {
	json_decref(oldModuleJ);
	json_decref(newModuleJ);
}

void ModuleChange::undo() {
	engine::Module* module = APP->engine->getModule(moduleId);
	if (!module)
		return;
	APP->engine->moduleFromJson(module, oldModuleJ);
}

void ModuleChange::redo() {
	engine::Module* module = APP->engine->getModule(moduleId);
	if (!module)
		return;
	APP->engine->moduleFromJson(module, newModuleJ);
}


void ParamChange::undo() {
	engine::Module* module = APP->engine->getModule(moduleId);
	if (!module)
		return;
	APP->engine->setParamValue(module, paramId, oldValue);
}

void ParamChange::redo() {
	engine::Module* module = APP->engine->getModule(moduleId);
	if (!module)
		return;
	APP->engine->setParamValue(module, paramId, newValue);
}


void CableAdd::setCable(app::CableWidget* cw) {
	assert(cw);
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
	if (!cw)
		return;
	APP->scene->rack->removeCable(cw);
	delete cw;
}

void CableAdd::redo() {
	engine::Cable* cable = new engine::Cable;
	cable->id = cableId;
	cable->inputModule = APP->engine->getModule(inputModuleId);
	if (!cable->inputModule) {
		delete cable;
		return;
	}
	cable->inputId = inputId;
	cable->outputModule = APP->engine->getModule(outputModuleId);
	if (!cable->outputModule) {
		delete cable;
		return;
	}
	cable->outputId = outputId;
	APP->engine->addCable(cable);

	app::CableWidget* cw = new app::CableWidget;
	cw->setCable(cable);
	cw->color = color;
	APP->scene->rack->addCable(cw);
}


void CableColorChange::setCable(app::CableWidget* cw) {
	assert(cw);
	assert(cw->cable);
	assert(cw->cable->id >= 0);
	cableId = cw->cable->id;
}

void CableColorChange::undo() {
	app::CableWidget* cw = APP->scene->rack->getCable(cableId);
	if (!cw)
		return;
	cw->color = oldColor;
}

void CableColorChange::redo() {
	app::CableWidget* cw = APP->scene->rack->getCable(cableId);
	if (!cw)
		return;
	cw->color = newColor;
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
	// Delete all future actions (if we have undone some actions)
	for (int i = actionIndex; i < (int) actions.size(); i++) {
		delete actions[i];
	}
	actions.resize(actionIndex);
	// Delete actions from beginning if limit is reached
	static const int limit = 500;
	int n = (int) actions.size() - limit + 1;
	if (n > 0) {
		for (int i = 0; i < n; i++) {
			delete actions[i];
		}
		actions.erase(actions.begin(), actions.begin() + n);
		actionIndex -= n;
		savedIndex -= n;
	}
	// Push action
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
