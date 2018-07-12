///////////////////////////////////////////////////////////////////
//
//  dBiz revisited version of Cartesian seq. by Strum 
// 
///////////////////////////////////////////////////////////////////

#include "dBiz.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_dBiz {

struct BenePads : Module {
    enum ParamIds
    {
        BUTTON_PARAM,
        NUM_PARAMS = BUTTON_PARAM + 16
    };
    enum InputIds
    {
     NUM_INPUTS
    };
	enum OutputIds {
	X_OUT,
  Y_OUT, 
  G_OUT,  
	NUM_OUTPUTS
  };
  enum LightIds
  {
    PAD_LIGHT,
    NUM_LIGHTS =PAD_LIGHT+16
  };

  SchmittTrigger button_triggers[4][4];
  
  int x_position = 0;
  int y_position = 0;
    
	BenePads() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void BenePads::step() {
//int x;
//int y;
bool shot = false;
 
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        if ((params[BUTTON_PARAM + i + j*4].value))
        {
          lights[PAD_LIGHT+i+j*4].value = 1.0;
          shot = true;
          x_position = i;
          y_position = j; 
          outputs[X_OUT].value = i + 1;
          outputs[Y_OUT].value = j + 1; 
        }
        else
        {
        lights[PAD_LIGHT+i+j*4].value=0.0  ;
        }
        if (shot)
        {
          outputs[G_OUT].value = 10.0;
        }
        else
        {
          outputs[G_OUT].value = 0.0;
        }
      } 
    }
    
}

////////////////////////////////

struct BenePadsWidget : ModuleWidget 
{
BenePadsWidget(BenePads *module) : ModuleWidget(module)
{
	box.size = Vec(15*11, 380);
  
	{
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin,"res/BenePad.svg")));
    addChild(panel);
  }
 
  int top = 20;
  int left = 3;
  int column_spacing = 35; 
  int row_spacing = 35;
  int button_offset = 20;

  addOutput(Port::create<PJ301MOrPort>(Vec(130, 10), Port::OUTPUT, module, BenePads::X_OUT));  
  addOutput(Port::create<PJ301MOrPort>(Vec(130, 40), Port::OUTPUT, module, BenePads::Y_OUT));
  addOutput(Port::create<PJ301MOrPort>(Vec(130, 70), Port::OUTPUT, module, BenePads::G_OUT));

      for (int i = 0; i < 4; i++)
  {
    for ( int j = 0 ; j < 4 ; j++)
    {
     
      addParam(ParamWidget::create<PB61303>(Vec(button_offset+left+column_spacing * i-10, top + row_spacing * j + 170 ), module, BenePads::BUTTON_PARAM + i + j * 4, 0.0, 1.0, 0.0));
      addChild(GrayModuleLightWidget::create<BigLight<OrangeLight>>(Vec(button_offset + left + column_spacing * i - 10 + 4.5, top + row_spacing * j + 170 + 4.5), module, BenePads::PAD_LIGHT + i + j * 4));
    }
    
    }  
	
  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, BenePads) {
   Model *modelBenePads = Model::create<BenePads, BenePadsWidget>("dBiz", "BenePads", "BenePads", UTILITY_TAG);
   return modelBenePads;
}
