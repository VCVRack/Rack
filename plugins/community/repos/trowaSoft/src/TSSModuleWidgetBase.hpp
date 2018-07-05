#ifndef TROWASOFT_MODULE_TSMODULEWIDGETBASE_HPP
#define TROWASOFT_MODULE_TSMODULEWIDGETBASE_HPP

#include "rack.hpp"
using namespace rack;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSModuleWidgetBase
// Base Module Widget. Remove randomize of parameters.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSModuleWidgetBase : ModuleWidget {
	bool randomizeParameters = false;
	TSSModuleWidgetBase(Module* tsModule) : ModuleWidget(tsModule) { 
		return; 
	}
	TSSModuleWidgetBase(Module* tsModule, bool randomizeParams) : TSSModuleWidgetBase(tsModule)
	{ 
		randomizeParameters = randomizeParams; 
		return; 
	}
	void randomize() override
	{
		if (randomizeParameters)
		{
			for (ParamWidget *param : params) {
				param->randomize();
			}
		}
		if (module) {
			module->randomize();
		}
		return;
	}
};

#endif // end if not defined