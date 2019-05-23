#include "CatroModulo.hpp"

RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM1Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM2Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM3Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM4Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM5Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM6Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM7Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM8Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM9Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CM10Module);
RACK_PLUGIN_MODEL_DECLARE(CatroModulo, CatroModulo_CM7);

RACK_PLUGIN_INIT(CatroModulo) {
	RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.5");
	RACK_PLUGIN_INIT_WEBSITE("https://github.com/catronomix/catro-modulo");
	RACK_PLUGIN_INIT_MANUAL("https://github.com/catronomix/catro-modulo/blob/master/CM-manual-v0.6.4.pdf");

	// Add all Models defined throughout the plugin
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM1Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM2Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM3Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM4Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM5Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM6Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM7Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM8Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM9Module);
	RACK_PLUGIN_MODEL_ADD(CatroModulo, CM10Module);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
	printf("Catro has gone Modulo\n");
}

// void init(Plugin *p) {
// 	plugin = p;
// 	p->slug = TOSTRING(SLUG);
// 	p->version = TOSTRING(VERSION);
// 
// 	// Add all Models defined throughout the plugin
// 	p->addModel(modelCM1Module);
// 	p->addModel(modelCM2Module);
// 	p->addModel(modelCM3Module);
// 	p->addModel(modelCM4Module);
// 	p->addModel(modelCM5Module);
// 	p->addModel(modelCM6Module);
// 	p->addModel(modelCM7Module);
// 	p->addModel(modelCM8Module);
// 	p->addModel(modelCM9Module);
// 	p->addModel(modelCM10Module);
// 
// 	// Any other plugin initialization may go here.
// 	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
// }

//horizontal slider
void CM_Slider_big_red::onDragMove(EventDragMove& e) {
    std::swap(e.mouseRel.x, e.mouseRel.y);
    e.mouseRel.y = -e.mouseRel.y;
    Knob::onDragMove(e);
}

