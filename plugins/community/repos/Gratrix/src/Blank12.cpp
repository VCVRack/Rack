//============================================================================================================
//!
//! \file Blank12.cpp
//!
//! \brief Blank 12 is a simple do nothing 12-hole high quality blank.
//!
//============================================================================================================


#include "Gratrix.hpp"

namespace rack_plugin_Gratrix {

//============================================================================================================
//! \brief The module.

struct GtxModule_Blank_12 : Module
{
	GtxModule_Blank_12() : Module(0, 0, 0) {}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_Blank_12 : ModuleWidget
{
	GtxWidget_Blank_12(GtxModule_Blank_12 *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(12*15, 380);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Blank12.svg"), box.size);
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Blank12.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, Blank_12) {
   Model *model = Model::create<GtxModule_Blank_12, GtxWidget_Blank_12>("Gratrix", "Blank12", "Blank 12", BLANK_TAG);
   return model;
}
