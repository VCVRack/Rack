#include "turing-module-widget.hh"
#include "turing-module.hh"

namespace rack_plugin_Skylights {

turing_module_widget::turing_module_widget(Module* module) : ModuleWidget(module) {
  setPanel(SVG::load(assetPlugin(plugin, "res/Alan.svg")));

  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  
  addInput(Port::create<DavidLTPort>
	   (Vec(63, 167),
	    Port::INPUT,
	    module,
	    turing_module::I_MODE));
  
  addInput(Port::create<DavidLTPort>
	   (Vec(4.5, 173),
	    Port::INPUT,
	    module,
	    turing_module::I_CLOCK));

  addOutput(Port::create<DavidLTPort>
	    (Vec(64, 317),
	     Port::OUTPUT,
	     module,
	     turing_module::O_VOLTAGE));

  addOutput(Port::create<DavidLTPort>
	    (Vec(116, 317),
	     Port::OUTPUT,
	     module,
	     turing_module::O_EXPANSION));

  addOutput(Port::create<DavidLTPort>
	    (Vec(90, 274),
	     Port::OUTPUT,
	     module,
	     turing_module::O_PULSE));
  
  addOutput(Port::create<DavidLTPort>
	    (Vec(90, 317),
	     Port::OUTPUT,
	     module,
	     turing_module::O_GATE));

  addParam(ParamWidget::create<RoundHugeBlackKnob>
	   (Vec(47, 80),
	    module,
	    turing_module::P_MODE,
	    0.0,
	    1.0,
	    1.0));

  addParam(ParamWidget::create<RoundSmallBlackKnob>
	   (Vec(34, 320),
	    module,
	    turing_module::P_SCALE,
	    0.0,
	    10.0,
	    1.0));

  addParam(ParamWidget::create<RoundBlackSnapKnob>
	   (Vec(61, 215),
	    module,
	    turing_module::P_LENGTH,
	    2.0,
	    16.0,
	    8.0));

  addParam(ParamWidget::create<CKSS>
	   (Vec(10, 320),
	    module,
	    turing_module::P_POLE,
	    0.0,
	    1.0,
	    0.0));

  addParam(ParamWidget::create<CKSS>
	   (Vec(121, 175),
	    module,
	    turing_module::P_WRITE,
	    0.0,
	    1.0,
	    0.0));

  for (size_t i = 0;
       i < 8;
       i++)
  {
     addChild(ModuleLightWidget::create<MediumLight<BlueLight>>
	      (Vec(18 + (15 * i), 50),
	       module,
	       turing_module::L_LIGHT1 + i));
  }
}

} // namespace rack_plugin_Skylights

using namespace rack_plugin_Skylights;

RACK_PLUGIN_MODEL_INIT(Skylights, turing_model) {
   Model *turing_model = Model::create<turing_module, turing_module_widget>("Skylights", "SkTuringV2", "SK Alan (Turing Machine)", SEQUENCER_TAG);
   return turing_model;
}
