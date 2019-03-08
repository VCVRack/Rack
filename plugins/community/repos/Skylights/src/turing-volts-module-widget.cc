#include "turing-volts-module-widget.hh"
#include "turing-volts-module.hh"

namespace rack_plugin_Skylights {

turing_volts_module_widget::turing_volts_module_widget(Module* module) : ModuleWidget(module) {
  setPanel(SVG::load(assetPlugin(plugin, "res/AlanVolts.svg")));

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  
  addInput(Port::create<DavidLTPort>
	   (Vec(25, 45),
	    Port::INPUT,
	    module,
	    turing_volts_module::I_EXPANDER));

  addOutput(Port::create<DavidLTPort>
	   (Vec(25, 300),
	    Port::OUTPUT,
	    module,
	    turing_volts_module::O_VOLTAGE));
  
  for (size_t i = 0;
       i < 5;
       i++)
    {
      addParam(ParamWidget::create<RoundBlackKnob>
	       (Vec(15, 80 + (40 * i)),
		module,
		turing_volts_module::P_VOL1 + i,
		-1.0,
		1.0,
		0.0));
      
      addChild(ModuleLightWidget::create<MediumLight<BlueLight>>
	       (Vec(50, 90 + (40 * i)),
		module,
		turing_volts_module::L_LIGHT1 + i));
    }
}

} // namespace rack_plugin_Skylights

using namespace rack_plugin_Skylights;

RACK_PLUGIN_MODEL_INIT(Skylights, turing_volts_model) {
   Model *turing_volts_model = Model::create<turing_volts_module, turing_volts_module_widget>("Skylights", "SkTuringVolts", "SK Alan (Volts Expander)", UTILITY_TAG);
   return turing_volts_model;
}
