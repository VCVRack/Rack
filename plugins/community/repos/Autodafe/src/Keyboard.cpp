//**************************************************************************************
//Waves Module for VCV Rack by Autodafe http://www.autodafe.net
//
//Based on code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//And part of code on musicdsp.org: http://musicdsp.org/showArchiveComment.php?ArchiveID=78
//**************************************************************************************


#include "Autodafe.hpp"
#include <stdlib.h>
#include "dsp/digital.hpp"

namespace rack_plugin_Autodafe {

#define FONT_FILE      assetPlugin(plugin, "res/Segment7Standard.ttf")

struct KeyboardModel : Module{
	enum ParamIds {
	
		PARAM_C,
		PARAM_CC,
		PARAM_D,
		PARAM_DD,
		PARAM_E,

		PARAM_F,
		PARAM_FF,
		PARAM_G,
		PARAM_GG,
		PARAM_A,
		PARAM_AA,
		PARAM_B,
		PARAM_C2,
		BTNUP,
		BTNDWN,
		NUM_PARAMS
	};

	enum InputIds {
		NUM_INPUTS
	};

	enum OutputIds {
		GATE_OUT,
		CV_OUT,	

		NUM_OUTPUTS
	};

   enum LightIds {
		NUM_LIGHTS
	};

	KeyboardModel();
	void step();

   SchmittTrigger C;
   SchmittTrigger CC;
   SchmittTrigger D;
   SchmittTrigger DD;
   SchmittTrigger E;
   SchmittTrigger F;
   SchmittTrigger FF;
   SchmittTrigger G;
   SchmittTrigger GG;
   SchmittTrigger A;
   SchmittTrigger AA;
   SchmittTrigger B;
   SchmittTrigger C2;

   SchmittTrigger btnup;
   SchmittTrigger btndwn;
   int note;
   int octave=3;
};

KeyboardModel::KeyboardModel()  : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

void KeyboardModel::step() {
   outputs[GATE_OUT].value = 0.0f;

   if (btnup.process(params[BTNUP].value))
   { 
      if (octave<8) {
         octave++;  
      }
      else
      {
         octave=8;
      }
   }


   if (btndwn.process(params[BTNDWN].value))
   { 
      if (octave>0) {
         octave--;  
      }
      else
      {
         octave=0;
      }
   }
	
   if(C.process(params[PARAM_C].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=0;

	}


   if(CC.process(params[PARAM_CC].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=1;
	}

   if(D.process(params[PARAM_D].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=2;
	}

	if(DD.process(params[PARAM_DD].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=3;
	}

   if(E.process(params[PARAM_E].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=4;

	}

	if(F.process(params[PARAM_F].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=5;
	}

   if(FF.process(params[PARAM_FF].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=6;
	}

	if(G.process(params[PARAM_G].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=7;

	}

	if(GG.process(params[PARAM_GG].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=8;

	}

	if(A.process(params[PARAM_A].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=9;
	}

	if(AA.process(params[PARAM_AA].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=10;

	}

	if(B.process(params[PARAM_B].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=11;

	}

	if(C2.process(params[PARAM_C2].value))
	{
		outputs[GATE_OUT].value=10.0;
		note=12;
	}

   outputs[CV_OUT].value = ((note+(octave*12) - 60)) / 12.0;
}
 

struct KeyboardModelDisplay : TransparentWidget {
   int *value;
   std::shared_ptr<Font> font;

   KeyboardModelDisplay() {
      font = Font::load(FONT_FILE);
   }

   void draw(NVGcontext *vg) {
      // Background
      NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
      NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
      nvgBeginPath(vg);
      nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
      nvgFillColor(vg, backgroundColor);
      nvgFill(vg);
      nvgStrokeWidth(vg, 1.0);
      nvgStrokeColor(vg, borderColor);
      nvgStroke(vg);

      nvgFontSize(vg, 36);
      nvgFontFaceId(vg, font->handle);
      nvgTextLetterSpacing(vg, 2.5);

      std::string to_display = std::to_string(*value);
      Vec textPos = Vec(7.0f, 35.0f);

      NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
      nvgFillColor(vg, nvgTransRGBA(textColor, 16));
      nvgText(vg, textPos.x, textPos.y, "~~~", NULL);

      textColor = nvgRGB(0xda, 0xe9, 0x29);
      nvgFillColor(vg, nvgTransRGBA(textColor, 16));
      nvgText(vg, textPos.x, textPos.y, "\\\\\\", NULL);

      textColor = nvgRGB(0xf0, 0x00, 0x00);
      nvgFillColor(vg, textColor);

   
      std::string z;
      if(to_display.length()==1){z="00"+to_display;}
		else {z="0"+to_display;}

      nvgText(vg, textPos.x, textPos.y, z.c_str(), NULL);
   }
};




struct OctaveDisplay : TransparentWidget {
   int *value;

   std::shared_ptr<Font> font;

   OctaveDisplay() {
      font = Font::load(FONT_FILE);
   }

   void draw(NVGcontext *vg) {
      // Background
      NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
      NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
      nvgBeginPath(vg);
      nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
      nvgFillColor(vg, backgroundColor);
      nvgFill(vg);
      nvgStrokeWidth(vg, 1.0);
      nvgStrokeColor(vg, borderColor);
      nvgStroke(vg);

      nvgFontSize(vg, 12);
      nvgFontFaceId(vg, font->handle);
      nvgTextLetterSpacing(vg, 2.5);

      std::string to_display = std::to_string(*value);
      Vec textPos = Vec(7.0f, 35.0f);

      NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
      nvgFillColor(vg, nvgTransRGBA(textColor, 16));
      nvgText(vg, textPos.x, textPos.y-20, "~~~", NULL);

      textColor = nvgRGB(0xda, 0xe9, 0x29);
      nvgFillColor(vg, nvgTransRGBA(textColor, 16));
      nvgText(vg, textPos.x, textPos.y-20, "\\\\\\", NULL);

      textColor = nvgRGB(0xf0, 0x00, 0x00);
      nvgFillColor(vg, textColor);
	
      //ADD LEADING ZEROS
      std::string z;
      if(to_display.length()==1){z="00"+to_display;}
		else {z="0"+to_display;}

      nvgText(vg, textPos.x, textPos.y-20, z.c_str(), NULL);
   }
};


struct KeyboardModelWidget : ModuleWidget{
	KeyboardModelWidget(KeyboardModel *module);
};

KeyboardModelWidget::KeyboardModelWidget(KeyboardModel *module) : ModuleWidget(module) {
	box.size = Vec(15 * 23 ,380); 

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Keyboard.svg")));
		addChild(panel);
	} 

   {
      OctaveDisplay *octavedisplay = new OctaveDisplay();
      octavedisplay->box.pos = Vec(25,310);
      octavedisplay->box.size = Vec(41, 21);
      octavedisplay->value = &module->octave;
      addChild(octavedisplay);
   }

   addParam(createParam<BtnUp>(Vec(40, 297), module, KeyboardModel::BTNUP, 0.0, 1.0, 0.0));
   addParam(createParam<BtnDwn>(Vec(40, 331), module, KeyboardModel::BTNDWN, 0.0, 1.0, 0.0));
  
	addChild(createScrew<ScrewSilver>(Vec(1, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 0)));
	addChild(createScrew<ScrewSilver>(Vec(1, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));
  
   addParam(createParam<WhiteKey>(Vec(10, 100), module, KeyboardModel::PARAM_C, 0.0, 1.0, 0.0));
   addParam(createParam<WhiteKey>(Vec(50, 100), module, KeyboardModel::PARAM_D, 0.0, 1.0, 0.0));
   addParam(createParam<WhiteKey>(Vec(90, 100), module, KeyboardModel::PARAM_E, 0.0, 1.0, 0.0));
   addParam(createParam<WhiteKey>(Vec(130, 100), module, KeyboardModel::PARAM_F, 0.0, 1.0, 0.0));
   addParam(createParam<WhiteKey>(Vec(170, 100), module, KeyboardModel::PARAM_G, 0.0, 1.0, 0.0));
   addParam(createParam<WhiteKey>(Vec(210, 100), module, KeyboardModel::PARAM_A, 0.0, 1.0, 0.0));
   addParam(createParam<WhiteKey>(Vec(250, 100), module, KeyboardModel::PARAM_B, 0.0, 1.0, 0.0));
   addParam(createParam<WhiteKey>(Vec(290, 100), module, KeyboardModel::PARAM_C2, 0.0, 1.0, 0.0));

   addParam(createParam<BlackKey>(Vec(40, 100), module, KeyboardModel::PARAM_CC, 0.0, 1.0, 0.0));
   addParam(createParam<BlackKey>(Vec(80, 100), module, KeyboardModel::PARAM_DD, 0.0, 1.0, 0.0));
   addParam(createParam<BlackKey>(Vec(160, 100), module, KeyboardModel::PARAM_FF, 0.0, 1.0, 0.0));
   addParam(createParam<BlackKey>(Vec(200, 100), module, KeyboardModel::PARAM_GG, 0.0, 1.0, 0.0));
   addParam(createParam<BlackKey>(Vec(240, 100), module, KeyboardModel::PARAM_AA, 0.0, 1.0, 0.0));

   addOutput(createOutput<PJ301MPort>(Vec(270, 310), module, KeyboardModel::GATE_OUT));
   addOutput(createOutput<PJ301MPort>(Vec(300, 310), module, KeyboardModel::CV_OUT));
}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, Keyboard) {
   return Model::create<KeyboardModel, KeyboardModelWidget>("Autodafe",  "Keyboard", "Keyboard", UTILITY_TAG);
}
