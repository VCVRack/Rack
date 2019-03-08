#include "turing-vactrol-module-widget.hh"
#include "turing-vactrol-module.hh"

namespace rack_plugin_Skylights {

turing_vactrol_module_widget::turing_vactrol_module_widget(Module* module) : ModuleWidget(module) {
   setPanel(SVG::load(assetPlugin(plugin, "res/AlanVactrol.svg")));

   addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
   addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
   addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

   addInput(Port::create<DavidLTPort>
	    (Vec(10, 40),
	     Port::INPUT,
	     module,
	     turing_vactrol_module::I_EXPANDER));
   
   for (size_t i = 0;
	i < 4;
	i++)
   {
      addInput(Port::create<DavidLTPort>
	       (Vec(10, 100 + (30 * i)),
		Port::INPUT,
		module,
		turing_vactrol_module::I_INPUT1 + i));
   }

   size_t y = 0;
   for (size_t i = 0;
	i < 4;
	i++)
   {
      addParam(ParamWidget::create<RoundBlackKnob>
	       (Vec(60, 80 + (50 * i)),
		module,
		turing_vactrol_module::P_VOL1 + i,
		-1.0,
		1.0,
		0.0));

      addChild(ModuleLightWidget::create<MediumLight<BlueLight>>
	       (Vec(63, 65 + (50 * i)),
		module,
		turing_vactrol_module::L_GATE1 + y++));
      addChild(ModuleLightWidget::create<MediumLight<BlueLight>>
	       (Vec(78, 65 + (50 * i)),
		module,
		turing_vactrol_module::L_GATE1 + y++));
   }
  
   addOutput(Port::create<DavidLTPort>
	     (Vec(10, 300),
	      Port::OUTPUT,
	      module,
	      turing_vactrol_module::O_LEFT));
   addOutput(Port::create<DavidLTPort>
	     (Vec(40, 300),
	      Port::OUTPUT,
	      module,
	      turing_vactrol_module::O_RIGHT));
}

} // namespace rack_plugin_Skylights

using namespace rack_plugin_Skylights;

RACK_PLUGIN_MODEL_INIT(Skylights, turing_vactrol_model) {
   Model *turing_vactrol_model =
      Model::create<turing_vactrol_module,
                    turing_vactrol_module_widget>
      ("Skylights",
       "SkTuringVactrol",
       "SK Alan (Vactrol Mix Expander)",
       UTILITY_TAG);
   return turing_vactrol_model;
}
