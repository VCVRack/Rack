#include "app.hpp"
#include "event.hpp"
#include "window.hpp"
#include "patch.hpp"
#include "engine/Engine.hpp"
#include "app/Scene.hpp"
#include "history.hpp"


namespace rack {
namespace app {


void App::init(bool headless) {
	engine = new engine::Engine;
	if (!headless) {
		event = new event::State;
		history = new history::State;
		window = new Window;
		patch = new PatchManager;
		scene = new Scene;
		event->rootWidget = scene;
	}
}

App::~App() {
	// Set pointers to NULL so other objects will segfault when attempting to access them
	if (scene) delete scene;
	scene = NULL;
	if (patch) delete patch;
	patch = NULL;
	if (event) delete event;
	event = NULL;
	if (history) delete history;
	history = NULL;
	if (window) delete window;
	window = NULL;
	if (engine) delete engine;
	engine = NULL;
}


static App *app = NULL;

void init(bool headless) {
	assert(!app);
	app = new App;
	app->init(headless);
}

void destroy() {
	assert(app);
	delete app;
	app = NULL;
}

App *get() {
	return app;
}


} // namespace app
} // namespace rack
