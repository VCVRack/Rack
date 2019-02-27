#pragma once
#include "common.hpp"


/** Accesses the global App pointer */
#define APP rack::appGet()


namespace rack {


namespace history {
	struct State;
} // namespace history


namespace engine {
	struct Engine;
} // namespace engine


struct Window;
struct PatchManager;


namespace widget {
	struct EventState;
};


namespace app {
	struct Scene;
} // namespace app


/** Contains the application state  */
struct App {
	widget::EventState *event = NULL;
	app::Scene *scene = NULL;
	engine::Engine *engine = NULL;
	Window *window = NULL;
	history::State *history = NULL;
	PatchManager *patch = NULL;

	void init(bool headless);
	~App();
};


void appInit(bool headless);
void appDestroy();
/** Returns the global App pointer */
App *appGet();


} // namespace rack
