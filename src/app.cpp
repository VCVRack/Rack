#include "app.hpp"
#include "event.hpp"
#include "window.hpp"
#include "patch.hpp"
#include "engine/Engine.hpp"
#include "app/Scene.hpp"
#include "history.hpp"


namespace rack {
namespace app {


App::App() {
	event = new event::State;
	history = new history::State;
	window = new Window;
	engine = new engine::Engine;
	patch = new PatchManager;
	scene = new Scene;
	event->rootWidget = scene;
}

App::~App() {
	// Set pointers to NULL so other objects will segfault when attempting to access them
	delete scene; scene = NULL;
	delete patch; patch = NULL;
	delete event; event = NULL;
	delete history; history = NULL;
	delete engine; engine = NULL;
	delete window; window = NULL;
}


static App *c = NULL;

void init() {
	assert(!c);
	c = new App;
}

void destroy() {
	assert(c);
	delete c;
	c = NULL;
}

App *get() {
	return c;
}


} // namespace app
} // namespace rack
