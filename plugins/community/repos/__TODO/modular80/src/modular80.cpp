#include "modular80.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/cschol/modular80";
	p->manual = "https://github.com/cschol/modular80/blob/master/README.md";

	p->addModel(modelLogistiker);
}
