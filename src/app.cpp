#include <app.hpp>
#include <window.hpp>
#include <patch.hpp>
#include <engine/Engine.hpp>
#include <app/Scene.hpp>
#include <history.hpp>
#include <settings.hpp>


namespace rack {


void App::init() {
	engine = new engine::Engine;
	if (!settings::headless) {
		event = new event::State;
		history = new history::State;
		window = new Window;
		patch = new PatchManager;
		scene = new app::Scene;
		event->rootWidget = scene;
	}
}

App::~App() {
	// Set pointers to NULL so other objects will segfault when attempting to access them
	if (scene)
		delete scene;
	scene = NULL;
	if (patch)
		delete patch;
	patch = NULL;
	if (event)
		delete event;
	event = NULL;
	if (history)
		delete history;
	history = NULL;
	if (window)
		delete window;
	window = NULL;
	if (engine)
		delete engine;
	engine = NULL;
}


static App* appInstance = NULL;

void appInit() {
	assert(!appInstance);
	appInstance = new App;
	appInstance->init();
}

void appDestroy() {
	assert(appInstance);
	delete appInstance;
	appInstance = NULL;
}

App* appGet() {
	return appInstance;
}


} // namespace rack
