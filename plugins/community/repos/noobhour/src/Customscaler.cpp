#include "Noobhour.hpp"
#include "dsp/digital.hpp"
#include <vector>

namespace rack_plugin_noobhour {

// TODO LATER
// scale file loader - Latif Fital
// CustomScale Pro
// - paging? different pages, back/forth, CV for index selection (interpolate between non-empty pages, lights to indicate?)
// - quad
// - button back for randomization

// TODO

// IN PROGRESS
// bus

// DONE
// bsl1r
// context menu switch between 0-10 and -5..5 - (was attenuator - Pyer Cllrd)
// Normalizing the gate input to all following inputs (with nothing plugged in them) would be amazing (instead of 4 copies of the same cable) Patrick McIlveen
// Normalized HIGH and LOW inputs as well
// Normalizing all of the outputs to the last one (a la AS 4ch baby mixer and Audible) Patrick McIlveen
// Performance imporvement CustomScale
// random subset (randomize activity of individual tones) - Pyer Cllrd
// ended up with lines on I, IV, V, was: change rings around lights for black/white distinction - steve baker 




struct GreenBlueYellowLight : GrayModuleLightWidget {
  GreenBlueYellowLight() {
	addBaseColor(COLOR_GREEN);
	addBaseColor(COLOR_BLUE);
	addBaseColor(COLOR_YELLOW);		
  }
};

template <typename BASE>
struct ToneLight : BASE {
  ToneLight() {
	this->box.size = mm2px(Vec(6.0f, 6.0f));
  }
};

/*
struct LEDBezelGray : SVGSwitch, MomentarySwitch {
	LEDBezelGray() {
	  addFrame(SVG::load(assetPlugin(plugin, "res/LEDBezelGray.svg")));
	}
};

struct LEDBezelDark : SVGSwitch, MomentarySwitch {
	LEDBezelDark() {
	  addFrame(SVG::load(assetPlugin(plugin, "res/LEDBezelDark.svg")));
	}
};

struct LighterGrayModuleLightWidget : ModuleLightWidget {
	LighterGrayModuleLightWidget() {
		bgColor = nvgRGB(0x9a, 0x9a, 0x9a);
		borderColor = nvgRGBA(0, 0, 0, 0x60);
	}
};

struct LighterGreenBlueYellowLight : LighterGrayModuleLightWidget {
  LighterGreenBlueYellowLight() {
	addBaseColor(COLOR_GREEN);
	addBaseColor(COLOR_BLUE);
	addBaseColor(COLOR_YELLOW);		
  }
};

*/


struct Customscaler : Module {

  static const int NUM_OCTAVES = 5;
  static const int BASE_OCTAVE = 2;
  static const int NUM_TONES = NUM_OCTAVES * 12;
  
  enum InputIds {
	SIGNAL_INPUT,
	TONE_INPUT,
	TOGGLE_TRIGGER_INPUT,
	RESET_TRIGGER_INPUT,
	RANDOMIZE_TRIGGER_INPUT,
	P_INPUT,
	BASE_INPUT,
	  
	NUM_INPUTS
  };
  
  enum OutputIds {
	OUT_OUTPUT,
	CHANGEGATE_OUTPUT,
	  
	NUM_OUTPUTS	  
  };

  enum ParamIds {
	ENUMS(TONE1_PARAM, NUM_TONES),
	RANGE_PARAM,
	P_PARAM,
	// RANDOMIZE_BUTTON_PARAM,
	RESET_BUTTON_PARAM,
	BASE_PARAM,
	MODE_PARAM,
	
	NUM_PARAMS
  };

  enum LightIds {
	ENUMS(TONE1_LIGHT, NUM_TONES * 3),

	NUM_LIGHTS
  };

  SchmittTrigger gateTrigger;
  SchmittTrigger randomizeTrigger;
  SchmittTrigger resetTrigger;
  SchmittTrigger resetButtonTrigger;
  SchmittTrigger paramTrigger[NUM_TONES];
  
  PulseGenerator changePulse;
  
  bool state[NUM_TONES];
  bool candidate[NUM_TONES];
  int lastFinalTone = NUM_TONES;
  int lastStartTone = NUM_TONES;
  int lastSelectedTone = NUM_TONES; 

  std::vector<int> activeTones;
  bool activeTonesDirty = true;

  bool bipolarInput = false;


  Customscaler() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	activeTones.reserve(NUM_TONES);
	onReset();	
  }
  
  void step() override;

  float getVOct(int toneIndex) const {
	int octave = toneIndex / 12;
	int tone = toneIndex % 12;
	return tone/12.f + octave - BASE_OCTAVE;
  }

  int getTone(float vOct) const {
	  return static_cast<int>(vOct * 12.f) + 12 * BASE_OCTAVE;
  }	

  void onReset() override {
	for (int i = 0; i < NUM_TONES; i++) {
	  state[i] = false;
	  candidate[i] = false;
	}
	activeTonesDirty = true;
  }

  float getP() {
	float p_input = 0;
	if (inputs[P_INPUT].active)
	  p_input = clamp(inputs[P_INPUT].value / 10.f, -10.f, 10.f);
	return clamp(p_input + params[P_PARAM].value, 0.0f, 1.0f);
  }
  
  void onRandomize() override {
	randomizeTones(getP()); 
  }

  void randomizeTones(float p) {
	for (int i = 0; i < NUM_TONES; i++) {
	  state[i] = (randomUniform() < p);
	  candidate[i] = false;
	}
	activeTonesDirty = true;
  }

  void randomSubset(float p) {
	int activeTones = 0;
	int candidates = 0;
	bool toggle = params[MODE_PARAM].value < 0.5f;
	for (int i = 0; i < NUM_TONES; i++) {
	  if (state[i] || candidate[i]) {
		candidates++;

		if (toggle) {
		  if (randomUniform() < p) {
			state[i] ^= true;
		  }
		  if (state[i]) {
			activeTones++;
			candidate[i] = false;
		  } else {
			candidate[i] = true;
		  }
		} else {		
		  if (randomUniform() < p) {
			activeTones++;
			state[i] = true;
			candidate[i] = false;
		  } else {
			state[i] = false;
			candidate[i] = true;
		  }
		}
	  }
	}

	// if random subset is called without active or candidate tones,
	// let it behave like the normal randomisation: everything is a
	// candidate, retry
	if (candidates == 0) {
	  for (int i = 0; i < NUM_TONES; i++) {
		candidate[i] = true;
	  }
	  randomSubset(p);
	  return;
	}

	// make sure at least one tone is active so we don't return 0 = C4
	// which may be not be a candidate
	if (activeTones == 0) {
	  for (int i = 0; i < NUM_TONES; i++) {
		if (candidate[i]) {
		  state[i] = true;
		  candidate[i] = false;
		  break;
		}
	  }
	}
	activeTonesDirty = true;	
  }

  json_t *toJson() override {
	json_t *rootJ = json_object();

	json_t *statesJ = json_array();
	for (int i = 0; i < NUM_TONES; i++) {
	  json_t *stateJ = json_boolean(state[i]);
	  json_array_append_new(statesJ, stateJ);
	}
	json_object_set_new(rootJ, "states", statesJ);

	json_t *candidatesJ = json_array();
	for (int i = 0; i < NUM_TONES; i++) {
	  json_t *candidateJ = json_boolean(candidate[i]);
	  json_array_append_new(candidatesJ, candidateJ);
	}
	json_object_set_new(rootJ, "candidates", candidatesJ);
	
	json_t *bipolarInputJ = json_boolean(bipolarInput);
	json_object_set_new(rootJ, "bipolarInput", bipolarInputJ);
	
	return rootJ;
  }
  
  void fromJson(json_t *rootJ) override {
	json_t *statesJ = json_object_get(rootJ, "states");
	if (statesJ) {
	  for (int i = 0; i < NUM_TONES; i++) {
		json_t *stateJ = json_array_get(statesJ, i);
		if (stateJ)
		  state[i] = json_boolean_value(stateJ);
	  }
	}

	json_t *candidatesJ = json_object_get(rootJ, "candidates");
	if (candidatesJ) {
	  for (int i = 0; i < NUM_TONES; i++) {
		json_t *candidateJ = json_array_get(candidatesJ, i);
		if (candidateJ)
		  candidate[i] = json_boolean_value(candidateJ);
	  }
	}

	json_t *bipolarInputJ = json_object_get(rootJ, "bipolarInput");
	bipolarInput = json_boolean_value(bipolarInputJ);
	  
	
	activeTonesDirty = true;
  }

};




void Customscaler::step() {
  // RESET
  if (inputs[RESET_TRIGGER_INPUT].active) {
	if (resetTrigger.process(rescale(inputs[RESET_TRIGGER_INPUT].value, 0.1f, 2.f, 0.f, 1.f))) {
	  onReset();
	}	
  }

  if (resetButtonTrigger.process(params[RESET_BUTTON_PARAM].value)) {
	onReset();
  }
  

  // RANDOMIZE
  if (inputs[RANDOMIZE_TRIGGER_INPUT].active) {
	if (randomizeTrigger.process(rescale(inputs[RANDOMIZE_TRIGGER_INPUT].value, 0.1f, 2.f, 0.f, 1.f))) {
	  randomSubset(getP());
	}		
  }

  /*
  if (randomizeButtonTrigger.process(params[RANDOMIZE_BUTTON_PARAM].value)) {
	randomizeTones(getP());
  }
  */

  // TOGGLE
  if (inputs[TONE_INPUT].active) {
	float gate = 0.0;
	if (inputs[TOGGLE_TRIGGER_INPUT].active)
	  gate = inputs[TOGGLE_TRIGGER_INPUT].value;
	if (gateTrigger.process(rescale(gate, 0.1f, 2.f, 0.f, 1.f))) {
	  int toneIndex = getTone(inputs[TONE_INPUT].value);
	  if (toneIndex >= 0 && toneIndex < NUM_TONES) {
		state[toneIndex] ^= true;
		candidate[toneIndex] = false;
		activeTonesDirty = true;
	  }
	}
  }

  // OCTAVE RANGE
  int startTone = 0;
  int endTone = 0;
  if (params[RANGE_PARAM].value < 0.5f) {
	startTone = 0;
	endTone = NUM_TONES - 1;
  } else if (params[RANGE_PARAM].value < 1.5f) {
	startTone = 12;
	endTone = NUM_TONES - 13;
  } else {
	startTone = 24;
	endTone = NUM_TONES - 25;
  }
  if (startTone != lastStartTone) {
	activeTonesDirty = true;
	lastStartTone = startTone;
  }

  // CHECK TONE TOGGLES
  for (int i = 0; i < NUM_TONES; i++) {
	if (paramTrigger[i].process(params[i].value)) {
	  state[i] ^= true;
	  candidate[i] = false;
	  activeTonesDirty = true;
	}
  }

  // GATHER CANDIDATES
  if (activeTonesDirty) {	
	activeTones.clear();
	for (int i = 0; i < NUM_TONES; i++) {
	  if (state[i] && i >= startTone && i <= endTone) {
		activeTones.push_back(i);
	  } 	 
	}
  }

  // FETCH BASE TONE
  float baseTone = params[BASE_PARAM].value;
  if (inputs[BASE_INPUT].active) {
	baseTone +=  inputs[BASE_INPUT].value / 10.f * 11.f;
  }
  int baseToneDiscrete = static_cast<int>(clamp(baseTone, 0.f, 11.f));

  // SELECT TONE  
  float output = 0;
  int selectedTone = NUM_TONES;
  int finalTone = NUM_TONES;
  if (inputs[SIGNAL_INPUT].active && activeTones.size() > 0) {
	float inp = inputs[SIGNAL_INPUT].value;
	if (bipolarInput)
	  inp += 5.f;
	unsigned int selectedIndex = static_cast<int>(activeTones.size() * (clamp(inp, 0.f, 10.f)) / 10.f);
	if (selectedIndex == activeTones.size())
	  selectedIndex--;
	selectedTone = activeTones[selectedIndex];
	finalTone = selectedTone + baseToneDiscrete;
	output = getVOct(finalTone);
  }

  // DETECT TONE CHANGE
  if (finalTone != lastFinalTone) {
	changePulse.trigger(0.001f);
	lastFinalTone = finalTone;
  }  

  // LIGHTS
  if (activeTonesDirty || selectedTone != lastSelectedTone) {
	for (int i = 0; i < NUM_TONES; i++) {
	  float green = 0.f;
	  float blue = 0.f;
	  float yellow = 0.f;
	  if (state[i]) {
		if (i==selectedTone) {
		  blue = 0.9f;
		} else {
		  if (i >= startTone && i <= endTone) {
			green = 0.9f; // active tone but not selected
		  } else {
			green = 0.1f; // active but in inactive octave
		  }		
		}
	  } else {
		if (candidate[i]) {
		  if (i >= startTone && i <= endTone) {
			yellow = 0.3f; // candidate 
		  } else {
			yellow = 0.1f; // candidate but in inactive octave
		  }		
		}		  
	  }
	  lights[i * 3].setBrightness(green);
	  lights[i * 3 + 1].setBrightness(blue);
	  lights[i * 3 + 2].setBrightness(yellow);	  
	}
  }
  activeTonesDirty = false; // only reset after check for lights has been done
  lastSelectedTone = selectedTone; 
  
  // OUTPUT
  outputs[OUT_OUTPUT].value = output;
  outputs[CHANGEGATE_OUTPUT].value = (changePulse.process(1.0f / engineGetSampleRate()) ? 10.0f : 0.0f);

}


struct CustomscalerWidget : ModuleWidget {
  // generate controls	
  const int yStart = 25;
  const int yRange = 40;
  const int ySeparator = 5;
  const float x = 11.5f;
  const float x2 = 46.5f;
  const float lastY = 329;

  // static const float wKnob = 30.23437f;
  const float wInput = 31.58030f;
  const float wSwitch = 17.94267f;	
  const float offsetKnob = -2.1; 
  const float offsetSwitch = (wInput - wSwitch) / 2.0f - 1.5; // no idea why 1.5, not centered otherwise
  const int offsetTL1005 = 4;

  const bool whiteKey[12] = {true, false, true, false, true, true, false, true, false, true, false, true};
  
  CustomscalerWidget(Customscaler *module) : ModuleWidget(module) {
	
	setPanel(SVG::load(assetPlugin(plugin, "res/Customscaler.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	// upper panel

	addInput(Port::create<PJ301MPort>(Vec(x, yStart + 0 * yRange + 0 * ySeparator), Port::INPUT, module, Customscaler::SIGNAL_INPUT));
	addParam(ParamWidget::create<CKSSThree>(Vec(x2 + offsetSwitch, yStart + 0 * yRange + 0 * ySeparator), module, Customscaler::RANGE_PARAM, 0.f, 2.f, 0.f));
	
	addInput(Port::create<PJ301MPort>(Vec(x, yStart + 1 * yRange + 1 * ySeparator), Port::INPUT, module, Customscaler::BASE_INPUT));
	addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(x2 + offsetKnob, yStart + 1 * yRange + 1 * ySeparator + offsetKnob), module, Customscaler::BASE_PARAM, 0.f, 11.f, 0.f));		
	
	addOutput(Port::create<PJ301MPort>(Vec(x, yStart + 2 * yRange + 2 * ySeparator), Port::OUTPUT, module, Customscaler::OUT_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(x2, yStart + 2 * yRange + 2 * ySeparator), Port::OUTPUT, module, Customscaler::CHANGEGATE_OUTPUT));	


	// lower panel
	
	addInput(Port::create<PJ301MPort>(Vec(x, lastY - (3 * yRange + 2 * ySeparator)), Port::INPUT, module, Customscaler::TONE_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(x2, lastY - (3 * yRange + 2 * ySeparator)), Port::INPUT, module, Customscaler::TOGGLE_TRIGGER_INPUT)); 
	
	// addInput(Port::create<PJ301MPort>(Vec(x, lastY - (1 * yRange + 1 * ySeparator)), Port::INPUT, module, Customscaler::RANDOM_SUBSET_TRIGGER_INPUT));	
	// addParam(ParamWidget::create<TL1105>(Vec(x2 + offsetTL1005, lastY - (3 * yRange + 1 * ySeparator - offsetTL1005)), module, Customscaler::RANDOMIZE_BUTTON_PARAM, 0.0f, 1.0f, 0.0f));

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(x2 + offsetKnob, lastY - (2 * yRange + 1 * ySeparator - offsetKnob)), module, Customscaler::P_PARAM, 0.f, 1.f, 0.5f));
	addInput(Port::create<PJ301MPort>(Vec(x, lastY - (2 * yRange + 1 * ySeparator)), Port::INPUT, module, Customscaler::P_INPUT));
	
	addInput(Port::create<PJ301MPort>(Vec(x, lastY - (1 * yRange + 1 * ySeparator)), Port::INPUT, module, Customscaler::RANDOMIZE_TRIGGER_INPUT));	
	addParam(ParamWidget::create<CKSS>(Vec(x2 + offsetSwitch, lastY - (1 * yRange + 1 * ySeparator)), module, Customscaler::MODE_PARAM, 0.f, 1.f, 1.f));

	addInput(Port::create<PJ301MPort>(Vec(x, lastY), Port::INPUT, module, Customscaler::RESET_TRIGGER_INPUT));
	addParam(ParamWidget::create<TL1105>(Vec(x2 + offsetTL1005, lastY  + offsetTL1005), module, Customscaler::RESET_BUTTON_PARAM, 0.0f, 1.0f, 0.0f));
	
	// generate lights
	float offsetX = mm2px(Vec(17.32, 18.915)).x - mm2px(Vec(16.57, 18.165)).x; // from Mutes
	float offsetY = mm2px(Vec(17.32, 18.915)).y - mm2px(Vec(16.57, 18.165)).y;	
	for (int octave=0; octave<Customscaler::NUM_OCTAVES; octave++) {
	  float x = 88 + octave * 27;
	  for (int tone=0; tone<12; tone++) {
		float y = -5 + 28 * (12 - tone);
		int index = octave * 12 + tone;

		addParam(ParamWidget::create<LEDBezel>(Vec(x, y), module, Customscaler::TONE1_PARAM + index, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<ToneLight<GreenBlueYellowLight>>(Vec(x + offsetX, y + offsetY), module, Customscaler::TONE1_PARAM + index * 3));

		/*
		if (whiteKey[tone]) {
		  addParam(ParamWidget::create<LEDBezelGray>(Vec(x, y), module, Customscaler::TONE1_PARAM + index, 0.0f, 1.0f, 0.0f));
		  addChild(ModuleLightWidget::create<ToneLight<GreenBlueYellowLight>>(Vec(x + offsetX, y + offsetY), module, Customscaler::TONE1_PARAM + index * 3));
		  
		} else {
		  addParam(ParamWidget::create<LEDBezelDark>(Vec(x, y), module, Customscaler::TONE1_PARAM + index, 0.0f, 1.0f, 0.0f));
		  addChild(ModuleLightWidget::create<ToneLight<GreenBlueYellowLight>>(Vec(x + offsetX, y + offsetY), module, Customscaler::TONE1_PARAM + index * 3));		  
		}
		*/
	  }
	}
  };

  void appendContextMenu(Menu *menu) override {
	Customscaler *customscaler = dynamic_cast<Customscaler*>(module);
	assert(customscaler);
	
	struct UniBiItem : MenuItem {
	  Customscaler *customscaler;
	  void onAction(EventAction &e) override {
		customscaler->bipolarInput ^= true;;
	  }
	  void step() override {
		rightText = customscaler->bipolarInput ? "-5V..5V" : "0V..10V";
		MenuItem::step();
	  }
	};
	
	menu->addChild(construct<MenuLabel>());
	menu->addChild(construct<UniBiItem>(&MenuItem::text, "Signal input", &UniBiItem::customscaler, customscaler));
  };
};

} // namespace rack_plugin_noobhour

using namespace rack_plugin_noobhour;

RACK_PLUGIN_MODEL_INIT(noobhour, Customscaler) {
   Model *modelCustomscaler = Model::create<Customscaler, CustomscalerWidget>("noobhour", "customscale", "Customscaler", QUANTIZER_TAG, RANDOM_TAG);
   return modelCustomscaler;
}
