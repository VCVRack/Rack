#pragma once
#include "common.hpp"


namespace rack {


namespace event {
	struct State;
}

namespace history {
	struct State;
}


struct Engine;
struct Window;
struct PatchManager;


namespace app {


struct Scene;


/** Contains the application state  */
struct App {
	event::State *event = NULL;
	Scene *scene = NULL;
	Engine *engine = NULL;
	Window *window = NULL;
	history::State *history = NULL;
	PatchManager *patch = NULL;

	App();
	~App();
};


void init();
void destroy();
/** Returns the global App pointer */
App *get();


/** Accesses the global App pointer */
#define APP rack::app::get()


} // namespace app
} // namespace rack
