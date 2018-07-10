//============================================================================================================
//!
//! \file Blank06.cpp
//!
//! \brief Blank 6 is a simple do nothing 6-hole high quality blank.
//!
//============================================================================================================


#include "Gratrix.hpp"

namespace rack_plugin_Gratrix {

//============================================================================================================
//! \brief The module.

struct GtxModule_Blank_06 : Module
{
	GtxModule_Blank_06() : Module(0, 0, 0) {}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_Blank_06 : ModuleWidget
{
	GtxWidget_Blank_06(GtxModule_Blank_06 *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(6*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Blank06.svg"), box.size);
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Blank06.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, Blank_06) {
   Model *model = Model::create<GtxModule_Blank_06, GtxWidget_Blank_06>("Gratrix", "Blank6", "Blank 6", BLANK_TAG);
   return model;
}
