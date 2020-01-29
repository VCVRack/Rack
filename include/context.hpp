#pragma once
#include <common.hpp>


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
struct Context {
	event::State* event = NULL;
	app::Scene* scene = NULL;
	engine::Engine* engine = NULL;
	Window* window = NULL;
	history::State* history = NULL;
	PatchManager* patch = NULL;

	/** Call this after setting the context for this thread, because initialization of application state accesses APP. */
	void init();
	~Context();
};


/** Returns the global Context pointer */
Context* contextGet();
/** Sets the context for this thread.
You must set the context when preparing each thread if the code uses the APP macro in that thread.
*/
void contextSet(Context* context);

/** Accesses the global Context pointer. Just an alias for contextGet(). */
#define APP rack::contextGet()


} // namespace rack
