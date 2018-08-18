#include "Nohmad.hpp"

RACK_PLUGIN_MODEL_DECLARE(Nohmad, Noise);
RACK_PLUGIN_MODEL_DECLARE(Nohmad, StrangeAttractors);

RACK_PLUGIN_INIT(Nohmad) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/joelrobichaud/Nohmad");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/joelrobichaud/Nohmad");

	RACK_PLUGIN_MODEL_ADD(Nohmad, Noise);
	RACK_PLUGIN_MODEL_ADD(Nohmad, StrangeAttractors);
}
