#include <context.hpp>
#include <window.hpp>
#include <patch.hpp>
#include <engine/Engine.hpp>
#include <app/Scene.hpp>
#include <history.hpp>
#include <settings.hpp>


namespace rack {


void Context::init() {
	engine = new engine::Engine;
	patch = new PatchManager;
	if (!settings::headless) {
		event = new event::State;
		history = new history::State;
		window = new Window;
		scene = new app::Scene;
		event->rootWidget = scene;
	}
}

Context::~Context() {
	// Set pointers to NULL so other objects will segfault when attempting to access them
	if (scene)
		delete scene;
	scene = NULL;
	if (event)
		delete event;
	event = NULL;
	if (history)
		delete history;
	history = NULL;
	if (window)
		delete window;
	window = NULL;
	if (patch)
		delete patch;
	patch = NULL;
	if (engine)
		delete engine;
	engine = NULL;
}


static thread_local Context* context = NULL;

void contextInit() {
	assert(!context);
	context = new Context;
	context->init();
}

void contextDestroy() {
	assert(context);
	delete context;
	context = NULL;
}

Context* contextGet() {
	assert(context);
	return context;
}

void contextSet(Context* context) {
	rack::context = context;
}


} // namespace rack
