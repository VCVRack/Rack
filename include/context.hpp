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

	void init();
	~Context();
};


void contextInit();
void contextDestroy();
/** Returns the global Context pointer */
Context* contextGet();
/** Sets the context for this thread.
You must call this every thread if you want to use the APP macro in that thread.
*/
void contextSet(Context* context);

/** Accesses the global Context pointer */
#define APP rack::contextGet()


} // namespace rack
