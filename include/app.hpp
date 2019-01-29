#pragma once
#include "common.hpp"


/** Accesses the global App pointer */
#define APP rack::app::get()


namespace rack {


namespace event {
	struct State;
} // namespace event


namespace history {
	struct State;
} // namespace history


namespace engine {
	struct Engine;
} // namespace engine


struct Window;
struct PatchManager;


namespace app {


struct Scene;


/** Contains the application state  */
struct App {
	event::State *event = NULL;
	Scene *scene = NULL;
	engine::Engine *engine = NULL;
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


} // namespace app
} // namespace rack
