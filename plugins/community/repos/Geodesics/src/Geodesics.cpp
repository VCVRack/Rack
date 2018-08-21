//***********************************************************************************************
//Geodesics: A modular collection for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from the Fundamental plugins by Andrew Belt 
//  and graphics from the Component Library by Wes Milholen 
//See ./LICENSE.txt for all licenses
//See ./res/fonts/ for font licenses
//
//***********************************************************************************************


#include "Geodesics.hpp"

RACK_PLUGIN_MODEL_DECLARE(Geodesics, BlackHoles);
RACK_PLUGIN_MODEL_DECLARE(Geodesics, Pulsars);
RACK_PLUGIN_MODEL_DECLARE(Geodesics, Branes);
RACK_PLUGIN_MODEL_DECLARE(Geodesics, Ions);
RACK_PLUGIN_MODEL_DECLARE(Geodesics, BlankLogo);
RACK_PLUGIN_MODEL_DECLARE(Geodesics, BlankInfo);

RACK_PLUGIN_INIT(Geodesics) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/MarcBoule/Geodesics");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/MarcBoule/Geodesics/blob/master/README.md");

	RACK_PLUGIN_MODEL_ADD(Geodesics, BlackHoles);
	RACK_PLUGIN_MODEL_ADD(Geodesics, Pulsars);
	RACK_PLUGIN_MODEL_ADD(Geodesics, Branes);
	RACK_PLUGIN_MODEL_ADD(Geodesics, Ions);
	RACK_PLUGIN_MODEL_ADD(Geodesics, BlankLogo);
	RACK_PLUGIN_MODEL_ADD(Geodesics, BlankInfo);
}
