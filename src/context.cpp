#include <context.hpp>
#include <window/Window.hpp>
#include <patch.hpp>
#include <engine/Engine.hpp>
#include <app/Scene.hpp>
#include <history.hpp>
#include <midiloopback.hpp>


namespace rack {


Context::~Context() {
	// Deleting NULL is safe in C++.

	// Set pointers to NULL so other objects will segfault when attempting to access them

	INFO("Deleting window");
	delete window;
	window = NULL;

	INFO("Deleting patch manager");
	delete patch;
	patch = NULL;

	INFO("Deleting scene");
	delete scene;
	scene = NULL;

	INFO("Deleting event state");
	delete event;
	event = NULL;

	INFO("Deleting history state");
	delete history;
	history = NULL;

	INFO("Deleting engine");
	delete engine;
	engine = NULL;

	INFO("Deleting MIDI loopback");
	delete midiLoopbackContext;
	midiLoopbackContext = NULL;
}


static thread_local Context* threadContext = NULL;

Context* contextGet() {
	assert(threadContext);
	return threadContext;
}

// Apple's clang incorrectly compiles this function when -O2 or higher is enabled.
#ifdef ARCH_MAC
__attribute__((optnone))
#endif
void contextSet(Context* context) {
	threadContext = context;
}


} // namespace rack
