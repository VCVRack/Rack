//============================================================================================================
//!
//! \file Blank09.cpp
//!
//! \brief Blank 9 is a simple do nothing 9-hole high quality blank.
//!
//============================================================================================================


#include "Gratrix.hpp"

namespace rack_plugin_Gratrix {

//============================================================================================================
//! \brief The module.

struct GtxModule_Blank_09 : Module
{
	GtxModule_Blank_09() : Module(0, 0, 0) {}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_Blank_09 : ModuleWidget
{
	GtxWidget_Blank_09(GtxModule_Blank_09 *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(9*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Blank09.svg"), box.size);
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Blank09.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, Blank_09) {
   Model *model = Model::create<GtxModule_Blank_09, GtxWidget_Blank_09>("Gratrix", "Blank9", "Blank 9", BLANK_TAG);
   return model;
}
