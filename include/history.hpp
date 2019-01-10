#pragma once
#include "common.hpp"
#include "math.hpp"
#include "plugin/Model.hpp"
#include <vector>
#include <jansson.h>


namespace rack {
namespace history {


struct Action {
	virtual ~Action() {}
	virtual void undo() {}
	virtual void redo() {}
};


/** Batches multiple actions into one */
struct ComplexAction : Action {
	/** Ordered by time occurred. Undoing will replay them backwards. */
	std::vector<Action*> actions;
	~ComplexAction();
	void undo() override;
	void redo() override;
	void push(Action *action);
};


/** An action operating on a module
Subclass this to create your own custom actions for your module.
*/
struct ModuleAction : Action {
	int moduleId;
};


struct ModuleAdd : ModuleAction {
	Model *model;
	math::Vec pos;
	void undo() override;
	void redo() override;
};


struct ModuleRemove : ModuleAction {
	Model *model;
	math::Vec pos;
	json_t *moduleJ;
	~ModuleRemove();
	void undo() override;
	void redo() override;
};


struct ModuleMove : ModuleAction {
	math::Vec oldPos;
	math::Vec newPos;
	void undo() override;
	void redo() override;
};


struct ModuleBypass : ModuleAction {
	bool bypass;
	void undo() override;
	void redo() override;
};


struct ParamChange : ModuleAction {
	int paramId;
	float oldValue;
	float newValue;
	void undo() override;
	void redo() override;
};


struct CableAdd : Action {
	int cableId;
	int outputModuleId;
	int outputId;
	int inputModuleId;
	int inputId;
	void undo() override;
	void redo() override;
};


struct CableRemove : Action {
	int cableId;
	int outputModuleId;
	int outputId;
	int inputModuleId;
	int inputId;
	void undo() override;
	void redo() override;
};


struct State {
	std::vector<Action*> actions;
	int actionIndex = 0;

	~State();
	void push(Action *action);
	void undo();
	void redo();
};


} // namespace history
} // namespace rack
