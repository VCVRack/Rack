#pragma once
#include <common.hpp>
#include <math.hpp>
#include <color.hpp>
#include <plugin/Model.hpp>
#include <vector>
#include <jansson.h>


namespace rack {


namespace app {
	struct ModuleWidget;
	struct CableWidget;
} // namespace app


/** Undo history actions for the Rack application */
namespace history {


/** An undo action with an inverse redo action.

Pointers to Modules, Params, etc. are not allowed in Actions because the object they refer to may be deleted and restored.
Instead, use moduleIds, etc.
*/
struct Action {
	/** Name of the action, lowercase. Used in the phrase "Undo ..." */
	std::string name;
	virtual ~Action() {}
	virtual void undo() {}
	virtual void redo() {}
};


template <class TAction>
struct InverseAction : TAction {
	void undo() override {
		TAction::redo();
	}
	void redo() override {
		TAction::undo();
	}
};


/** Batches multiple actions into one */
struct ComplexAction : Action {
	/** Ordered by time occurred. Undoing will replay them backwards. */
	std::vector<Action*> actions;
	~ComplexAction();
	void undo() override;
	void redo() override;
	void push(Action *action);
	bool isEmpty();
};


/** An action operating on a module.
Subclass this to create your own custom actions for your module.
*/
struct ModuleAction : Action {
	int moduleId;
};


struct ModuleAdd : ModuleAction {
	plugin::Model *model;
	math::Vec pos;
	json_t *moduleJ;
	ModuleAdd() {
		name = "add module";
	}
	~ModuleAdd();
	void setModule(app::ModuleWidget *mw);
	void undo() override;
	void redo() override;
};


struct ModuleRemove : InverseAction<ModuleAdd> {
	ModuleRemove() {
		name = "remove module";
	}
};


struct ModuleMove : ModuleAction {
	math::Vec oldPos;
	math::Vec newPos;
	void undo() override;
	void redo() override;
	ModuleMove() {
		name = "move module";
	}
};


struct ModuleBypass : ModuleAction {
	bool bypass;
	void undo() override;
	void redo() override;
	ModuleBypass() {
		name = "bypass module";
	}
};


struct ModuleChange : ModuleAction {
	json_t *oldModuleJ;
	json_t *newModuleJ;
	ModuleChange() {
		name = "change module";
	}
	~ModuleChange();
	void undo() override;
	void redo() override;
};


struct ParamChange : ModuleAction {
	int paramId;
	float oldValue;
	float newValue;
	void undo() override;
	void redo() override;
	ParamChange() {
		name = "change parameter";
	}
};


struct CableAdd : Action {
	int cableId;
	int outputModuleId;
	int outputId;
	int inputModuleId;
	int inputId;
	NVGcolor color;
	void setCable(app::CableWidget *cw);
	void undo() override;
	void redo() override;
	CableAdd() {
		name = "add cable";
	}
};


struct CableRemove : InverseAction<CableAdd> {
	CableRemove() {
		name = "remove cable";
	}
};


struct State {
	std::vector<Action*> actions;
	int actionIndex;
	/** Action index of saved patch state. */
	int savedIndex;

	State();
	~State();
	void clear();
	void push(Action *action);
	void undo();
	void redo();
	bool canUndo();
	bool canRedo();
	std::string getUndoName();
	std::string getRedoName();
	void setSaved();
	bool isSaved();
};


} // namespace history
} // namespace rack
