#pragma once
#include "common.hpp"


namespace rack {


namespace event {
	struct State;
}

struct Scene;
struct Engine;
struct Window;


/** Contains the application state  */
struct Context {
	event::State *event = NULL;
	Scene *scene = NULL;
	Engine *engine = NULL;
	Window *window = NULL;
};


/** Returns the global context */
Context *context();


} // namespace rack
