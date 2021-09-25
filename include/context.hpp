#pragma once
#include <common.hpp>


namespace rack {


namespace history {
struct State;
} // namespace history


namespace engine {
struct Engine;
} // namespace engine


namespace window {
struct Window;
} // namespace window


namespace patch {
struct Manager;
} // namespace patch


namespace widget {
struct EventState;
} // namespace widget


namespace app {
struct Scene;
} // namespace app


/** Rack instance state
*/
struct Context {
	widget::EventState* event = NULL;
	app::Scene* scene = NULL;
	engine::Engine* engine = NULL;
	window::Window* window = NULL;
	history::State* history = NULL;
	patch::Manager* patch = NULL;

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
