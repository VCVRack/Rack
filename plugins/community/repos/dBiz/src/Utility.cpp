///////////////////////////////////////////////////
//  dBiz Utility
// 
///////////////////////////////////////////////////

#include "dBiz.hpp"
#include <ctime>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

using namespace std;

namespace rack_plugin_dBiz {

/////added fine out /////////////////////////////////////////////////
struct Utility : Module {
  enum ParamIds
  {
    LINK_A_PARAM,
    LINK_B_PARAM,
    ROOT_NOTE_PARAM,
    SCALE_PARAM,
    OCTAVE_SHIFT,
    SEMITONE_SHIFT = OCTAVE_SHIFT + 3,
    FINE_SHIFT = SEMITONE_SHIFT + 3,
    AMOUNT_PARAM = FINE_SHIFT + 3,
    NUM_PARAMS = AMOUNT_PARAM + 3
  };
  enum InputIds {
    ROOT_NOTE_INPUT,
    SCALE_INPUT,
	OCTAVE_INPUT,
    OCTAVE_CVINPUT=OCTAVE_INPUT+3,
    SEMITONE_CVINPUT=OCTAVE_CVINPUT+3,
    FINE_CVINPUT=SEMITONE_CVINPUT+3,
    AMOUNT_CVINPUT=FINE_CVINPUT+3,
    NUM_INPUTS=AMOUNT_CVINPUT+3
	};
	enum OutputIds {
	A_OUTPUT,
    B_OUTPUT,
    C_OUTPUT,
    NUM_OUTPUTS
};

    enum LighIds {
        AMOUNT_LIGHT,
        NUM_LIGHTS=AMOUNT_LIGHT+3
    };


  //copied & fixed these scales http://www.grantmuller.com/MidiReference/doc/midiReference/ScaleReference.html
	int SCALE_AEOLIAN        [7] = {0, 2, 3, 5, 7, 8, 10};
	int SCALE_BLUES          [6] = {0, 3, 5, 6, 7, 10}; //FIXED!
	int SCALE_CHROMATIC      [12]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	int SCALE_DIATONIC_MINOR [7] = {0, 2, 3, 5, 7, 8, 10};
	int SCALE_DORIAN         [7] = {0, 2, 3, 5, 7, 9, 10};
	int SCALE_HARMONIC_MINOR [7] = {0, 2, 3, 5, 7, 8, 11};
	int SCALE_INDIAN         [7] = {0, 1, 1, 4, 5, 8, 10};
	int SCALE_LOCRIAN        [7] = {0, 1, 3, 5, 6, 8, 10};
	int SCALE_LYDIAN         [7] = {0, 2, 4, 6, 7, 9, 10};
	int SCALE_MAJOR          [7] = {0, 2, 4, 5, 7, 9, 11};
	int SCALE_MELODIC_MINOR  [9] = {0, 2, 3, 5, 7, 8, 9, 10, 11};
	int SCALE_MINOR          [7] = {0, 2, 3, 5, 7, 8, 10};
	int SCALE_MIXOLYDIAN     [7] = {0, 2, 4, 5, 7, 9, 10};
	int SCALE_NATURAL_MINOR  [7] = {0, 2, 3, 5, 7, 8, 10};
	int SCALE_PENTATONIC     [5] = {0, 2, 4, 7, 9};
	int SCALE_PHRYGIAN       [7] = {0, 1, 3, 5, 7, 8, 10};
	int SCALE_TURKISH        [7] = {0, 1, 3, 5, 7, 10, 11};

  enum Notes
  {
    NOTE_C,
    NOTE_C_SHARP,
    NOTE_D,
    NOTE_D_SHARP,
    NOTE_E,
    NOTE_F,
    NOTE_F_SHARP,
    NOTE_G,
    NOTE_G_SHARP,
    NOTE_A,
    NOTE_A_SHARP,
    NOTE_B,
    NUM_NOTES
  };

  enum Scales
  {
    AEOLIAN,
    BLUES,
    CHROMATIC,
    DIATONIC_MINOR,
    DORIAN,
    HARMONIC_MINOR,
    INDIAN,
    LOCRIAN,
    LYDIAN,
    MAJOR,
    MELODIC_MINOR,
    MINOR,
    MIXOLYDIAN,
    NATURAL_MINOR,
    PENTATONIC,
    PHRYGIAN,
    TURKISH,
    NONE,
    NUM_SCALES
  };

  int rootNote = 0;
  int curScaleVal = 0;
  float octave_out[3] {};
  float semitone_out[3] {};
  float fine_out[3] {};
  

  Utility() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


  void step() override;

  
  
// Quantization based on JW quantizer module!!!

  float closestVoltageInScale(float voltsIn)
  {
    rootNote = params[ROOT_NOTE_PARAM].value + rescale(inputs[ROOT_NOTE_INPUT].value, 0,10,0, Utility::NUM_NOTES - 1);
    curScaleVal = params[SCALE_PARAM].value + rescale(inputs[SCALE_INPUT].value, 0,10,0, Utility::NUM_SCALES - 1);
    int *curScaleArr;
    int notesInScale = 0;
    switch (curScaleVal)
    {
    case AEOLIAN:
      curScaleArr = SCALE_AEOLIAN;
      notesInScale = LENGTHOF(SCALE_AEOLIAN);
      break;
    case BLUES:
      curScaleArr = SCALE_BLUES;
      notesInScale = LENGTHOF(SCALE_BLUES);
      break;
    case CHROMATIC:
      curScaleArr = SCALE_CHROMATIC;
      notesInScale = LENGTHOF(SCALE_CHROMATIC);
      break;
    case DIATONIC_MINOR:
      curScaleArr = SCALE_DIATONIC_MINOR;
      notesInScale = LENGTHOF(SCALE_DIATONIC_MINOR);
      break;
    case DORIAN:
      curScaleArr = SCALE_DORIAN;
      notesInScale = LENGTHOF(SCALE_DORIAN);
      break;
    case HARMONIC_MINOR:
      curScaleArr = SCALE_HARMONIC_MINOR;
      notesInScale = LENGTHOF(SCALE_HARMONIC_MINOR);
      break;
    case INDIAN:
      curScaleArr = SCALE_INDIAN;
      notesInScale = LENGTHOF(SCALE_INDIAN);
      break;
    case LOCRIAN:
      curScaleArr = SCALE_LOCRIAN;
      notesInScale = LENGTHOF(SCALE_LOCRIAN);
      break;
    case LYDIAN:
      curScaleArr = SCALE_LYDIAN;
      notesInScale = LENGTHOF(SCALE_LYDIAN);
      break;
    case MAJOR:
      curScaleArr = SCALE_MAJOR;
      notesInScale = LENGTHOF(SCALE_MAJOR);
      break;
    case MELODIC_MINOR:
      curScaleArr = SCALE_MELODIC_MINOR;
      notesInScale = LENGTHOF(SCALE_MELODIC_MINOR);
      break;
    case MINOR:
      curScaleArr = SCALE_MINOR;
      notesInScale = LENGTHOF(SCALE_MINOR);
      break;
    case MIXOLYDIAN:
      curScaleArr = SCALE_MIXOLYDIAN;
      notesInScale = LENGTHOF(SCALE_MIXOLYDIAN);
      break;
    case NATURAL_MINOR:
      curScaleArr = SCALE_NATURAL_MINOR;
      notesInScale = LENGTHOF(SCALE_NATURAL_MINOR);
      break;
    case PENTATONIC:
      curScaleArr = SCALE_PENTATONIC;
      notesInScale = LENGTHOF(SCALE_PENTATONIC);
      break;
    case PHRYGIAN:
      curScaleArr = SCALE_PHRYGIAN;
      notesInScale = LENGTHOF(SCALE_PHRYGIAN);
      break;
    case TURKISH:
      curScaleArr = SCALE_TURKISH;
      notesInScale = LENGTHOF(SCALE_TURKISH);
      break;
    case NONE:
      return voltsIn;
    }

    float closestVal = 10.0;
    float closestDist = 10.0;
    float scaleNoteInVolts = 0;
    float distAway = 0;
    int octaveInVolts = int(floorf(voltsIn));
    float voltMinusOct = voltsIn - octaveInVolts;
    		for (int i=0; i < notesInScale; i++) {
			scaleNoteInVolts = curScaleArr[i] / 12.0;
			distAway = fabs(voltMinusOct - scaleNoteInVolts);
			if(distAway < closestDist){
				closestVal = scaleNoteInVolts;
				closestDist = distAway;
			}
    }
    return octaveInVolts + rootNote/12.0 + closestVal;
  }
  
};




/////////////////////////////////////////////////////
void Utility::step() {

if(params[LINK_A_PARAM].value ==1.0 )
  inputs[OCTAVE_INPUT + 1].value = inputs[OCTAVE_INPUT + 0].value;

if (params[LINK_B_PARAM].value == 1.0)
  inputs[OCTAVE_INPUT + 2].value = inputs[OCTAVE_INPUT + 1].value;

for (int i = 0; i < 3; i++)
{
  octave_out[i] = inputs[OCTAVE_INPUT + i].value + round(params[OCTAVE_SHIFT + i].value) + round(inputs[OCTAVE_CVINPUT + i].value / 2);
  semitone_out[i] = octave_out[i] + round(params[SEMITONE_SHIFT + i].value) * (1.0 / 12.0) + round(inputs[SEMITONE_CVINPUT + i].value / 2) * (1.0 / 12.0);
  fine_out[i] = semitone_out[i] + (params[FINE_SHIFT + i].value) * (1.0 / 12.0) + (inputs[FINE_CVINPUT + i].value / 2) * (1.0 / 2.0);
    }

    float out_a = closestVoltageInScale(fine_out[0]);
    float out_b = closestVoltageInScale(fine_out[1]);
    float out_c = closestVoltageInScale(fine_out[2]);

    outputs[A_OUTPUT].value = out_a;
    outputs[B_OUTPUT].value = out_b;
    outputs[C_OUTPUT].value = out_c;

}


struct UtilityDisplay : TransparentWidget
{
  Utility *module;
  int frame = 0;
  shared_ptr<Font> font;

  string note, scale;

  UtilityDisplay()
  {
    font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
  }

  void drawMessage(NVGcontext *vg, Vec pos, string note,string scale)
  {
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, -2);
    nvgFillColor(vg, nvgRGBA(75, 199, 75, 0xff));
    nvgFontSize(vg, 14);
    nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));
    nvgText(vg, pos.x + 8, pos.y + 23, note.c_str(), NULL);
    nvgText(vg, pos.x + 30, pos.y + 23, scale.c_str(), NULL);
  
  }

  string displayRootNote(int value)
  {
    switch (value)
    {
    case Utility::NOTE_C:
      return "C";
    case Utility::NOTE_C_SHARP:
      return "C#";
    case Utility::NOTE_D:
      return "D";
    case Utility::NOTE_D_SHARP:
      return "D#";
    case Utility::NOTE_E:
      return "E";
    case Utility::NOTE_F:
      return "F";
    case Utility::NOTE_F_SHARP:
      return "F#";
    case Utility::NOTE_G:
      return "G";
    case Utility::NOTE_G_SHARP:
      return "G#";
    case Utility::NOTE_A:
      return "A";
    case Utility::NOTE_A_SHARP:
      return "A#";
    case Utility::NOTE_B:
      return "B";
    default:
      return "";
    }
  }

  string displayScale(int value)
  {
    switch (value)
    {
    case Utility::AEOLIAN:
      return "Aeolian";
    case Utility::BLUES:
      return "Blues";
    case Utility::CHROMATIC:
      return "Chromatic";
    case Utility::DIATONIC_MINOR:
      return "Diat. Min.";
    case Utility::DORIAN:
      return "Dorian";
    case Utility::HARMONIC_MINOR:
      return "Harm. Min.";
    case Utility::INDIAN:
      return "Indian";
    case Utility::LOCRIAN:
      return "Locrian";
    case Utility::LYDIAN:
      return "Lydian";
    case Utility::MAJOR:
      return "Major";
    case Utility::MELODIC_MINOR:
      return "Melo. Min.";
    case Utility::MINOR:
      return "Minor";
    case Utility::MIXOLYDIAN:
      return "Mixolydian";
    case Utility::NATURAL_MINOR:
      return "Nat. Min.";
    case Utility::PENTATONIC:
      return "Pentatonic";
    case Utility::PHRYGIAN:
      return "Phrygian";
    case Utility::TURKISH:
      return "Turkish";
    case Utility::NONE:
      return "None";
    default:
      return "";
    }
  }

  void draw(NVGcontext *vg) override
  {
    if (++frame >= 4)
    {
      frame = 0;
      note = displayRootNote(module->rootNote);
      scale = displayScale(module->curScaleVal);
    }
    drawMessage(vg, Vec(0, 20), note, scale);
  }
};


//////////////////////////////////////////////////////////////////
struct UtilityWidget : ModuleWidget 
{
UtilityWidget(Utility *module) : ModuleWidget(module)
{
	box.size = Vec(15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin,"res/Utility.svg")));
		addChild(panel);
    }
    
  {
    UtilityDisplay *display = new UtilityDisplay();
    display->module = module;
    display->box.pos = Vec(10, + 240);
    display->box.size = Vec(250, 60);
    addChild(display);
  }

//Screw
  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
    
   int knob=35;  
  


//
for (int i=0;i<3;i++)
{ 
    addParam(ParamWidget::create<FlatASnap>(Vec(10+knob*i, 20), module, Utility::OCTAVE_SHIFT+i, -4.5, 4.5, 0.0));
    addParam(ParamWidget::create<FlatASnap>(Vec(10+knob*i, 60), module, Utility::SEMITONE_SHIFT+i, -5.0 ,5.0, 0.0));
    addParam(ParamWidget::create<FlatA>(Vec(10+knob*i, 100), module, Utility::FINE_SHIFT+i, -1, 1, 0.0));

    addInput(Port::create<PJ301MIPort>(Vec(12.5+knob*i, 100+knob*1.3), Port::INPUT, module, Utility::OCTAVE_INPUT+i));
    addInput(Port::create<PJ301MCPort>(Vec(12.5+knob*i, 130+knob*1.3), Port::INPUT, module, Utility::OCTAVE_CVINPUT+i));
    addInput(Port::create<PJ301MCPort>(Vec(12.5+knob*i, 160+knob*1.3), Port::INPUT, module, Utility::SEMITONE_CVINPUT+i));
    addInput(Port::create<PJ301MCPort>(Vec(12.5+knob*i, 190+knob*1.3), Port::INPUT, module, Utility::FINE_CVINPUT+i));   
}

  addParam(ParamWidget::create<Trimpot>(Vec(65,304), module, Utility::ROOT_NOTE_PARAM, 0.0, Utility::NUM_NOTES - 1 + 0.1, 0));
  addParam(ParamWidget::create<Trimpot>(Vec(90,304), module, Utility::SCALE_PARAM, 0.0, Utility::NUM_SCALES - 1 + 0.1, 0));

  addInput(Port::create<PJ301MPort>(Vec(10,300), Port::INPUT, module, Utility::ROOT_NOTE_INPUT));
  addInput(Port::create<PJ301MPort>(Vec(37,300), Port::INPUT, module, Utility::SCALE_INPUT));

  addOutput(Port::create<PJ301MOPort>(Vec(12.5,340), Port::OUTPUT, module, Utility::A_OUTPUT));
  addOutput(Port::create<PJ301MOPort>(Vec(12.5+knob*1,340), Port::OUTPUT, module, Utility::B_OUTPUT));
  addOutput(Port::create<PJ301MOPort>(Vec(12.5+knob*2,340), Port::OUTPUT, module, Utility::C_OUTPUT));

  addParam(ParamWidget::create<CKSSS>(Vec(39,150), module, Utility::LINK_A_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<CKSSS>(Vec(74.5, 150), module, Utility::LINK_B_PARAM, 0.0, 1.0, 0.0));
}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, Utility) {
   Model *modelUtility = Model::create<Utility, UtilityWidget>("dBiz", "Utility", "Utility", QUANTIZER_TAG);
   return modelUtility;
}
