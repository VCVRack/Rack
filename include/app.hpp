#pragma once
#include "common.hpp"


namespace rack {


namespace event {
	struct State;
}

namespace history {
	struct State;
}

struct Scene;
struct Engine;
struct Window;


/** Contains the application state  */
struct App {
	event::State *event = NULL;
	Scene *scene = NULL;
	Engine *engine = NULL;
	Window *window = NULL;
	history::State *history = NULL;

	App();
	~App();
};


void appInit();
void appDestroy();
/** Returns the global context */
App *app();


} // namespace rack
