#include "history.hpp"


namespace rack {
namespace history {


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
