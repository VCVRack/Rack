#pragma once
#include "common.hpp"


namespace rack {
namespace history {


struct Action {
	virtual ~Action() {}
	virtual void commit() {}
	virtual void commitInverse() {}
};



struct State {
	void push(Action *action);
	void undo();
	void redo();
};


} // namespace history
} // namespace rack
