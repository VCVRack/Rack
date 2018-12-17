#pragma once
#include "common.hpp"


namespace rack {


namespace event {
	struct Context;
}

struct Scene;
struct Engine;
struct PluginManager;
struct AssetManager;


struct Context {
	bool devMode = false;

	event::Context *event = NULL;
	Scene *scene = NULL;
	Engine *engine = NULL;
	PluginManager *plugin = NULL;
	AssetManager *asset = NULL;
};


Context *context();


} // namespace rack
