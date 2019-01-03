#pragma once
#include "common.hpp"


namespace rack {


namespace event {
	struct State;
}

struct Scene;
struct Engine;
struct Window;


struct Context {
	event::State *event = NULL;
	Scene *scene = NULL;
	Engine *engine = NULL;
	Window *window = NULL;

	bool skipLoadOnLaunch = false;
};


Context *context();


} // namespace rack
