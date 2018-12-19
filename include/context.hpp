#pragma once
#include "common.hpp"


namespace rack {


namespace event {
	struct Context;
}

struct Scene;
struct Engine;
struct PluginManager;
struct Window;


struct Context {
	event::Context *event = NULL;
	Scene *scene = NULL;
	Engine *engine = NULL;
	PluginManager *plugin = NULL;
	Window *window = NULL;
};


Context *context();


} // namespace rack
