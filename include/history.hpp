#pragma once
#include "common.hpp"
#include "math.hpp"
#include "plugin/Model.hpp"
#include <vector>


namespace rack {
namespace history {


struct Action {
	virtual ~Action() {}
	virtual void undo() {}
	virtual void redo() {}
};


struct ModuleAdd : Action {
	Model *model;
	int moduleId;
	math::Vec pos;
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
