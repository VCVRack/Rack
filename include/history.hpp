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

	struct WireInfo {
		int wireId;
		int outputModuleId;
		int outputId;
		int inputModuleId;
		int inputId;
	};
	std::vector<WireInfo> wireInfos;

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


struct ParamChange : ModuleAction {
	int paramId;
	float oldValue;
	float newValue;
	void undo() override;
	void redo() override;
};


struct WireAdd : Action {
	int wireId;
	int outputModuleId;
	int outputId;
	int inputModuleId;
	int inputId;
	void undo() override;
	void redo() override;
};


struct WireRemove : Action {
	int wireId;
	int outputModuleId;
	int outputId;
	int inputModuleId;
	int inputId;
	void undo() override;
	void redo() override;
};


struct WireMove : Action {
	int wireId;
	int oldOutputModuleId;
	int oldOutputId;
	int oldInputModuleId;
	int oldInputId;
	int newOutputModuleId;
	int newOutputId;
	int newInputModuleId;
	int newInputId;
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
