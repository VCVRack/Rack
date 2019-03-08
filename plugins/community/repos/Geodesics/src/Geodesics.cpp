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
RACK_PLUGIN_MODEL_DECLARE(Geodesics, Entropia);
RACK_PLUGIN_MODEL_DECLARE(Geodesics, Ions);
RACK_PLUGIN_MODEL_DECLARE(Geodesics, BlankLogo);
RACK_PLUGIN_MODEL_DECLARE(Geodesics, BlankInfo);

RACK_PLUGIN_INIT(Geodesics) {
   RACK_PLUGIN_INIT_ID();
   RACK_PLUGIN_INIT_VERSION("0.6.6");
   RACK_PLUGIN_INIT_WEBSITE("https://github.com/MarcBoule/Geodesics");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/MarcBoule/Geodesics/blob/master/README.md");

	RACK_PLUGIN_MODEL_ADD(Geodesics, BlackHoles);
	RACK_PLUGIN_MODEL_ADD(Geodesics, Pulsars);
	RACK_PLUGIN_MODEL_ADD(Geodesics, Branes);
	RACK_PLUGIN_MODEL_ADD(Geodesics, Entropia);
	RACK_PLUGIN_MODEL_ADD(Geodesics, Ions);
	RACK_PLUGIN_MODEL_ADD(Geodesics, BlankLogo);
	RACK_PLUGIN_MODEL_ADD(Geodesics, BlankInfo);
}

namespace rack_plugin_Geodesics {

// other

int getWeighted1to8random() {
	int	prob = randomu32() % 1000;
	if (prob < 175)
		return 1;
	else if (prob < 330) // 175 + 155
		return 2;
	else if (prob < 475) // 175 + 155 + 145
		return 3;
	else if (prob < 610) // 175 + 155 + 145 + 135
		return 4;
	else if (prob < 725) // 175 + 155 + 145 + 135 + 115
		return 5;
	else if (prob < 830) // 175 + 155 + 145 + 135 + 115 + 105
		return 6;
	else if (prob < 925) // 175 + 155 + 145 + 135 + 115 + 105 + 95
		return 7;
	return 8;
}

} // namespace rack_plugin_Geodesics
