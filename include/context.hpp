#pragma once
#include "common.hpp"


namespace rack {


struct Scene;
struct Engine;

namespace event {
	struct Context;
}


struct Context {
	bool devMode = false;

	event::Context *event = NULL;
	Scene *scene = NULL;
	Engine *engine = NULL;
};


Context *context();


} // namespace rack
