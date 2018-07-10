//============================================================================================================
//!
//! \file Blank03.cpp
//!
//! \brief Blank 3 is a simple do nothing 3-hole high quality blank.
//!
//============================================================================================================


#include "Gratrix.hpp"

namespace rack_plugin_Gratrix {

//============================================================================================================
//! \brief The module.

struct GtxModule_Blank_03 : Module
{
	GtxModule_Blank_03() : Module(0, 0, 0) {}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_Blank_03 : ModuleWidget
{
	GtxWidget_Blank_03(GtxModule_Blank_03 *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(3*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Blank03.svg"), box.size);
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Blank03.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, Blank_03) {
   Model *model = Model::create<GtxModule_Blank_03, GtxWidget_Blank_03>("Gratrix", "Blank3", "Blank 3", BLANK_TAG);
   return model;
}
