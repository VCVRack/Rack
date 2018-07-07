#include "LOGinstruments.hpp"

RACK_PLUGIN_MODEL_DECLARE(LOGinstruments, constant);
RACK_PLUGIN_MODEL_DECLARE(LOGinstruments, constant2);
RACK_PLUGIN_MODEL_DECLARE(LOGinstruments, Speck);
RACK_PLUGIN_MODEL_DECLARE(LOGinstruments, Britix);
RACK_PLUGIN_MODEL_DECLARE(LOGinstruments, Compa);
RACK_PLUGIN_MODEL_DECLARE(LOGinstruments, LessMess);
RACK_PLUGIN_MODEL_DECLARE(LOGinstruments, Velvet);
RACK_PLUGIN_MODEL_DECLARE(LOGinstruments, Crystal);

RACK_PLUGIN_INIT(LOGinstruments) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/leopard86/LOGinstruments");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/leopard86/LOGinstruments/blob/master/README.md");

	RACK_PLUGIN_MODEL_ADD(LOGinstruments, constant);
	RACK_PLUGIN_MODEL_ADD(LOGinstruments, constant2);
	RACK_PLUGIN_MODEL_ADD(LOGinstruments, Speck);
	RACK_PLUGIN_MODEL_ADD(LOGinstruments, Britix);
	RACK_PLUGIN_MODEL_ADD(LOGinstruments, Compa);
	RACK_PLUGIN_MODEL_ADD(LOGinstruments, LessMess);
	RACK_PLUGIN_MODEL_ADD(LOGinstruments, Velvet);
	RACK_PLUGIN_MODEL_ADD(LOGinstruments, Crystal);
}
