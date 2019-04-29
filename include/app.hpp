#pragma once
#include "common.hpp"


namespace rack {


namespace history {
	struct State;
} // namespace history


namespace engine {
	struct Engine;
} // namespace engine


struct Window;
struct PatchManager;


namespace event {
	struct State;
} // namespace event


namespace app {
	struct Scene;
} // namespace app


/** Contains the application state  */
struct App {
	event::State *event = NULL;
	app::Scene *scene = NULL;
	engine::Engine *engine = NULL;
	Window *window = NULL;
	history::State *history = NULL;
	PatchManager *patch = NULL;

	void init();
	~App();
};


void appInit();
void appDestroy();
/** Returns the global App pointer */
App *appGet();

/** Accesses the global App pointer */
#define APP appGet()


} // namespace rack
