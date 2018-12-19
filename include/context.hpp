#pragma once
#include "common.hpp"


namespace rack {


namespace event {
	struct Context;
}

struct Scene;
struct Engine;
struct Window;


struct Context {
	event::Context *event = NULL;
	Scene *scene = NULL;
	Engine *engine = NULL;
	Window *window = NULL;

	bool skipLoadOnLaunch = false;
};


Context *context();


} // namespace rack
