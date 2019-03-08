#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"
#include "dsp/digital.hpp"

#include <iostream>

namespace rack_plugin_AmalgamatedHarmonics {

struct BombeChord {
	int rootNote;
	int quality;
	int chord;
	int modeDegree;
	int inversion;
	float outVolts[6];
	BombeChord() : rootNote(0), quality(0), chord(0), modeDegree(0), inversion(0) {
		for (int j = 0; j < 6; j++) {
			outVolts[j] = 0.0f;
		}
	}
};

struct Bombe : AHModule {

	const static int NUM_PITCHES = 6;
	const static int N_NOTES = 12;
	const static int N_DEGREES = 7;
	const static int N_QUALITIES = 6;
	const static int QMAP_SIZE = 20;

	enum ParamIds {
		KEY_PARAM,
		MODE_PARAM,
		LENGTH_PARAM,
		X_PARAM,
		Y_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		KEY_INPUT,
		MODE_INPUT,
		FREEZE_INPUT,
		Y_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(PITCH_OUTPUT,6),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(LOCK_LIGHT,2),
		NUM_LIGHTS
	};
	
	Bombe() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for(int i = 0; i < BUFFERSIZE; i++) {
			buffer[i].chord = 1;
			int *chordArray = CoreUtil().ChordTable[buffer[i].chord].root;
			for (int j = 0; j < 6; j++) {
				buffer[i].outVolts[j] = CoreUtil().getVoltsFromPitch(chordArray[j], buffer[i].rootNote);			
			}
		}
	}

	void step() override;
	void modeRandom(BombeChord lastValue, float y);
	void modeSimple(BombeChord lastValue, float y);
	void modeKey(BombeChord lastValue, float y);
	void modeGalaxy(BombeChord lastValue, float y);
	void modeComplex(BombeChord lastValue, float y);

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// offset
		json_t *offsetJ = json_integer((int) offset);
		json_object_set_new(rootJ, "offset", offsetJ);

		// mode
		json_t *modeJ = json_integer((int) mode);
		json_object_set_new(rootJ, "mode", modeJ);

		// inversions
		json_t *inversionsJ = json_integer((int) allowedInversions);
		json_object_set_new(rootJ, "inversions", inversionsJ);

		return rootJ;
	}
	
	void fromJson(json_t *rootJ) override {

		// offset
		json_t *offsetJ = json_object_get(rootJ, "offset");
		if (offsetJ)
			offset = json_integer_value(offsetJ);

		// mode
		json_t *modeJ = json_object_get(rootJ, "mode");
		if (modeJ)
			mode = json_integer_value(modeJ);

		// mode
		json_t *inversionsJ = json_object_get(rootJ, "inversions");
		if (inversionsJ)
			allowedInversions = json_integer_value(inversionsJ);

	}

	Core core;

	const static int N_CHORDS = 98;

	int ChordMap[N_CHORDS] = {1,2,26,29,71,28,72,91,31,97,25,44,54,61,78,95,10,14,15,17,48,79,81,85,11,30,89,94,24,3,90,98,96,60,55,86,5,93,7,56,92,16,32,46,62,77,18,49,65,68,70,82,20,22,23,45,83,87,6,21,27,42,80,9,52,69,76,13,37,88,53,58,8,41,57,47,64,73,19,50,59,66,74,12,35,38,63,33,34,51,4,36,40,43,84,67,39,75};
 	int MajorScale[7] = {0,2,4,5,7,9,11};
	int Quality2Chord[N_QUALITIES] = { 1, 71, 91 }; // M, m, dim
	int QualityMap[3][QMAP_SIZE] = { 
		{01,01,01,01,01,01,01,01,01,01,25,25,25,25,25,25,25,25,31,31},
		{71,71,71,71,71,71,71,71,71,71,78,78,78,78,78,78,78,78,31,31},
		{91,91,91,91,91,91,91,91,91,91,94,94,94,94,94,94,94,94,94,94}
	};

	int InversionMap[3][QMAP_SIZE] = { 
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2},
	};
	
	int poll = 50000;

	SchmittTrigger clockTrigger;

	int currRoot = 1;
	int currMode = 1;
	int currInversion = 0;
	int length = 16;

	int offset = 12; 	   // 0 = random, 12 = lower octave, 24 = repeat, 36 = upper octave
	int mode = 1; 	   // 0 = random chord, 1 = chord in key, 2 = chord in mode
	int allowedInversions = 0; // 0 = root only, 1 = root + first, 2 = root, first, second

	std::string rootName = "";
	std::string modeName = "";

	const static int BUFFERSIZE = 16;
	BombeChord buffer[BUFFERSIZE];
	BombeChord displayBuffer[BUFFERSIZE];

};

void Bombe::modeSimple(BombeChord lastValue, float y) {

	// Recalculate new value of buffer[0].outVolts from lastValue
	int shift = (rand() % (N_DEGREES - 1)) + 1; // 1 - 6 - always new chord
	buffer[0].modeDegree = (lastValue.modeDegree + shift) % N_DEGREES; // FIXME, come from mode2 modeDeg == -1!

	int q;
	int n;
	CoreUtil().getRootFromMode(currMode,currRoot,buffer[0].modeDegree,&n,&q);
	buffer[0].rootNote = n;
	buffer[0].quality = q; // 0 = Maj, 1 = Min, 2 = Dim

	if (randomUniform() < y) {
		buffer[0].chord = QualityMap[q][rand() % QMAP_SIZE]; // Get the index into the main chord table
	} else {
		buffer[0].chord = Quality2Chord[q]; // Get the index into the main chord table
	}

	buffer[0].inversion = InversionMap[allowedInversions][rand() % QMAP_SIZE];

}

void Bombe::modeRandom(BombeChord lastValue, float y) {

	// Recalculate new value of buffer[0].outVolts from lastValue
	float p = randomUniform();
	if (p < y) {
		buffer[0].rootNote = rand() % 12; 
	} else {
		buffer[0].rootNote = MajorScale[rand() % 7]; 
	}

	buffer[0].modeDegree = -1; // FIXME
	buffer[0].quality = -1; // FIXME

	int maxChord = (int)((float)N_CHORDS * y) + 1;
	buffer[0].chord = ChordMap[rand() % maxChord]; 
	buffer[0].inversion = InversionMap[allowedInversions][rand() % QMAP_SIZE];

}

void Bombe::modeKey(BombeChord lastValue, float y) {

	int shift = (rand() % (N_DEGREES - 1)) + 1; // 1 - 6 - always new chord
	buffer[0].modeDegree = (lastValue.modeDegree + shift) % N_DEGREES; // FIXME, come from mode2 modeDeg == -1!

	int q;
	int n;
	CoreUtil().getRootFromMode(currMode,currRoot,buffer[0].modeDegree,&n,&q);

	buffer[0].rootNote = n;
	buffer[0].quality = -1; // FIXME
	buffer[0].chord = (rand() % (CoreUtil().NUM_CHORDS - 1)) + 1; // Get the index into the main chord table
	buffer[0].inversion = InversionMap[allowedInversions][rand() % QMAP_SIZE];

}

void Bombe::modeGalaxy(BombeChord lastValue, float y) {

	float excess = y - randomUniform();

	if (excess < 0.0) {
		modeSimple(lastValue, y);
	} else {
		if (excess < 0.2) {
			modeKey(lastValue, y);
		} else {
			modeRandom(lastValue, y);
		}
	}

}

void Bombe::modeComplex(BombeChord lastValue, float y) {
	modeGalaxy(lastValue, y); // Default to Galaxy
}

void Bombe::step() {
	
	AHModule::step();

	// Get inputs from Rack
	bool clocked = clockTrigger.process(inputs[CLOCK_INPUT].value);
	bool locked = false;
	bool updated = false;

	if (inputs[MODE_INPUT].active) {
		float fMode = inputs[MODE_INPUT].value;
		currMode = CoreUtil().getModeFromVolts(fMode);
	} else {
		currMode = params[MODE_PARAM].value;
	}

	if (inputs[KEY_INPUT].active) {
		float fRoot = inputs[KEY_INPUT].value;
		currRoot = CoreUtil().getKeyFromVolts(fRoot);
	} else {
		currRoot = params[KEY_PARAM].value;
	}

	float x = params[X_PARAM].value;
	float y = clamp(params[Y_PARAM].value + inputs[Y_PARAM].value * 0.1f, 0.0, 1.0);
	length = params[LENGTH_PARAM].value;

	if ((x >= 1.0f) || (inputs[FREEZE_INPUT].value > 0.000001f)) {
		locked = true;
	}

	switch(mode) {
		case 0: // Random
			rootName = "";
			modeName = "";
			break;
		case 1: // Simple
			rootName = CoreUtil().noteNames[currRoot];
			modeName = CoreUtil().modeNames[currMode];
			break;
		case 2: // Galaxy
			rootName = CoreUtil().noteNames[currRoot];
			modeName = CoreUtil().modeNames[currMode];
			break;
		case 3: // Complex
			rootName = CoreUtil().noteNames[currRoot];
			modeName = CoreUtil().modeNames[currMode];
			break;
		default:
			rootName = "";
			modeName = "";
	}

	if (clocked) {

		// Grab value from last element of sub-array, which will be the new head value
		BombeChord lastValue = buffer[length - 1];

		// Shift buffer
		for(int i = length - 1; i > 0; i--) {
			buffer[i] = buffer[i-1];
		}

		// Set first element
		if (locked) {
			// Buffer is locked
			buffer[0] = lastValue;
		} else {

			if (randomUniform() < x) {
				// Buffer update skipped
				buffer[0] = lastValue;
			} else {
				
				// We are going to update this entry 
				updated = true;

				switch(mode) {
					case 0:	modeRandom(lastValue, y); break;
					case 1:	modeSimple(lastValue, y); break;
					case 2:	modeGalaxy(lastValue, y); break;
					case 3:	modeComplex(lastValue, y); break;
					default: modeSimple(lastValue, y);
				}

				// Determine which chord corresponds to the grid position
				int *chordArray;
				switch(buffer[0].inversion) {
					case 0: 	chordArray = CoreUtil().ChordTable[buffer[0].chord].root; 	break;
					case 1: 	chordArray = CoreUtil().ChordTable[buffer[0].chord].first; 	break;
					case 2: 	chordArray = CoreUtil().ChordTable[buffer[0].chord].second;	break;
					default: 	chordArray = CoreUtil().ChordTable[buffer[0].chord].root;
				}

				// Determine which notes corresponds to the chord
				for (int j = 0; j < NUM_PITCHES; j++) {
					if (chordArray[j] < 0) {
						int off = offset;
						if (offset == 0) { // if offset = 0, randomise offset per note
							off = (rand() % 3 + 1) * 12;
						}
						buffer[0].outVolts[j] = CoreUtil().getVoltsFromPitch(chordArray[j] + off, buffer[0].rootNote);			
					} else {
						buffer[0].outVolts[j] = CoreUtil().getVoltsFromPitch(chordArray[j], buffer[0].rootNote);			
					}	
				}
			}
		}

		for(int i = BUFFERSIZE - 1; i > 0; i--) {
			displayBuffer[i] = displayBuffer[i-1];
		}
		displayBuffer[0] = buffer[0];

		// for(int i = 0; i < BUFFERSIZE; i++) {
		// 	std::cout << buffer[i].rootNote;
		// }
		// std::cout << std::endl;

		// for(int i = 0; i < BUFFERSIZE; i++) {
		// 	std::cout << displayBuffer[i].rootNote;
		// }
		// std::cout << std::endl << std::endl;
		
	}

	if (updated) { // Green Update
		lights[LOCK_LIGHT].setBrightnessSmooth(1.0f);
		lights[LOCK_LIGHT + 1].setBrightnessSmooth(0.0f);
	} else if (locked) { // Yellow locked
		lights[LOCK_LIGHT].setBrightnessSmooth(0.0f);
		lights[LOCK_LIGHT + 1].setBrightnessSmooth(1.0f);
	} else { // No change
		lights[LOCK_LIGHT].setBrightnessSmooth(0.0f);
		lights[LOCK_LIGHT + 1].setBrightnessSmooth(0.0f);
	}

	// Set the output pitches and lights
	for (int i = 0; i < NUM_PITCHES; i++) {
		outputs[PITCH_OUTPUT + i].value = buffer[0].outVolts[i];
	}

}

struct BombeDisplay : TransparentWidget {
	
	Bombe *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	BombeDisplay() {
		font = Font::load(assetPlugin(plugin, "res/EurostileBold.ttf"));
	}

	void draw(NVGcontext *vg) override {
	
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(255, 0, 0, 0xff));
		nvgTextLetterSpacing(vg, -1);

		char text[128];

		for (int i = 0; i < 7; i++)  {

			std::string chordName = "";
			std::string chordExtName = "";

			if (module->displayBuffer[i].chord != 0) {

				chordName = 
					CoreUtil().noteNames[module->displayBuffer[i].rootNote] + " " + 
					CoreUtil().ChordTable[module->displayBuffer[i].chord].quality + " " + 
					CoreUtil().inversionNames[module->displayBuffer[i].inversion];

				switch(module->mode) {
					case 0:
						chordExtName = "";
						break;
					case 1:
					case 2:
					case 3:
						if (module->displayBuffer[i].modeDegree != -1 && module->displayBuffer[i].quality != -1) { // FIXME
							int index = module->displayBuffer[i].modeDegree * 3 + module->displayBuffer[i].quality;
							chordExtName = CoreUtil().degreeNames[index];
						}
						break;
					default:
						chordExtName = "";
				}
			}

			snprintf(text, sizeof(text), "%s %s", chordName.c_str(), chordExtName.c_str());
			nvgText(vg, box.pos.x + 5, box.pos.y + i * 14, text, NULL);
			nvgFillColor(vg, nvgRGBA(255 - i * 32, 0, 0, 0xff));

		}

		nvgFillColor(vg, nvgRGBA(255, 0, 0, 0xff));

		nvgTextAlign(vg, NVG_ALIGN_RIGHT);
		snprintf(text, sizeof(text), "%s", module->rootName.c_str());
		nvgText(vg, box.size.x - 5, box.pos.y, text, NULL);

		snprintf(text, sizeof(text), "%s", module->modeName.c_str());
		nvgText(vg, box.size.x - 5, box.pos.y + 11, text, NULL);

	}
	
};

struct BombeWidget : ModuleWidget {

	Menu *createContextMenu() override;

	BombeWidget(Bombe *module) : ModuleWidget(module) {
	
		UI ui;
		
		box.size = Vec(240, 380);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Bombe.svg")));
			addChild(panel);
		}

		{
			BombeDisplay *display = new BombeDisplay();
			display->module = module;
			display->box.pos = Vec(0, 20);
			display->box.size = Vec(240, 230);
			addChild(display);
		}

		for (int i = 0; i < 6; i++) {
			addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, i, 5, true, false), Port::OUTPUT, module, Bombe::PITCH_OUTPUT + i));
		}	

		addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 0, 4, true, false), module, Bombe::KEY_PARAM, 0.0, 11.0, 0.0)); 
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 1, 4, true, false), Port::INPUT, module, Bombe::KEY_INPUT));

		addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 2, 4, true, false), module, Bombe::MODE_PARAM, 0.0, 6.0, 0.0)); 
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 3, 4, true, false), Port::INPUT, module, Bombe::MODE_INPUT));

		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 4, 4, true, false), Port::INPUT, module, Bombe::CLOCK_INPUT));
		addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 5, 4, true, false), module, Bombe::LENGTH_PARAM, 2.0, 16.0, 0.0)); 

		Vec XParamPos;
		XParamPos.x = 33;
		XParamPos.y = 160;
		addParam(ParamWidget::create<AHBigKnobNoSnap>(XParamPos, module, Bombe::X_PARAM, 0.0, 1.0001, 0.5)); 

		Vec XFreezePos;
		XFreezePos.x = XParamPos.x - 12;
		XFreezePos.y = XParamPos.y + 60;
		addInput(Port::create<PJ301MPort>(XFreezePos, Port::INPUT, module, Bombe::FREEZE_INPUT));

		Vec XLightPos;
		XLightPos.x = XParamPos.x + 63;
		XLightPos.y = XParamPos.y + 68;
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(XLightPos, module, Bombe::LOCK_LIGHT));

		Vec YParamPos;
		YParamPos.x = 137;
		YParamPos.y = 160;
		addParam(ParamWidget::create<AHBigKnobNoSnap>(YParamPos, module, Bombe::Y_PARAM, 0.0, 1.0001, 0.5)); 

		Vec YInputPos;
		YInputPos.x = YParamPos.x - 12;
		YInputPos.y = YParamPos.y + 60;
		addInput(Port::create<PJ301MPort>(YInputPos, Port::INPUT, module, Bombe::Y_INPUT));

	}

};

struct BombeOffsetItem : MenuItem {
	Bombe *bombe;
	int offset;
	void onAction(EventAction &e) override {
		bombe->offset = offset;
	}
	void step() override {
		rightText = (bombe->offset == offset) ? "✔" : "";
	}
};

struct BombeModeItem : MenuItem {
	Bombe *bombe;
	int mode;
	void onAction(EventAction &e) override {
		bombe->mode = mode;
	}
	void step() override {
		rightText = (bombe->mode == mode) ? "✔" : "";
	}
};

struct BombeInversionsItem : MenuItem {
	Bombe *bombe;
	int allowedInversions;
	void onAction(EventAction &e) override {
		bombe->allowedInversions = allowedInversions;
	}
	void step() override {
		rightText = (bombe->allowedInversions == allowedInversions) ? "✔" : "";
	}
};

Menu *BombeWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Bombe *bombe = dynamic_cast<Bombe*>(module);
	assert(bombe);

	MenuLabel *offsetLabel = new MenuLabel();
	offsetLabel->text = "Repeat Notes";
	menu->addChild(offsetLabel);

	BombeOffsetItem *offsetLowerItem = new BombeOffsetItem();
	offsetLowerItem->text = "Lower";
	offsetLowerItem->bombe = bombe;
	offsetLowerItem->offset = 12;
	menu->addChild(offsetLowerItem);

	BombeOffsetItem *offsetRepeatItem = new BombeOffsetItem();
	offsetRepeatItem->text = "Repeat";
	offsetRepeatItem->bombe = bombe;
	offsetRepeatItem->offset = 24;
	menu->addChild(offsetRepeatItem);

	BombeOffsetItem *offsetUpperItem = new BombeOffsetItem();
	offsetUpperItem->text = "Upper";
	offsetUpperItem->bombe = bombe;
	offsetUpperItem->offset = 36;
	menu->addChild(offsetUpperItem);

	BombeOffsetItem *offsetRandomItem = new BombeOffsetItem();
	offsetRandomItem->text = "Random";
	offsetRandomItem->bombe = bombe;
	offsetRandomItem->offset = 0;
	menu->addChild(offsetRandomItem);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Mode";
	menu->addChild(modeLabel);

	BombeModeItem *modeRandomItem = new BombeModeItem();
	modeRandomItem->text = "Random";
	modeRandomItem->bombe = bombe;
	modeRandomItem->mode = 0;
	menu->addChild(modeRandomItem);

	BombeModeItem *modeSimpleItem = new BombeModeItem();
	modeSimpleItem->text = "Simple";
	modeSimpleItem->bombe = bombe;
	modeSimpleItem->mode = 1;
	menu->addChild(modeSimpleItem);

	BombeModeItem *modeGalaxyItem = new BombeModeItem();
	modeGalaxyItem->text = "Galaxy";
	modeGalaxyItem->bombe = bombe;
	modeGalaxyItem->mode = 2;
	menu->addChild(modeGalaxyItem);

	BombeModeItem *modeComplexItem = new BombeModeItem();
	modeComplexItem->text = "Complex";
	modeComplexItem->bombe = bombe;
	modeComplexItem->mode = 3;
	// menu->addChild(modeComplexItem);

	MenuLabel *invLabel = new MenuLabel();
	invLabel->text = "Allowed Chord Inversions";
	menu->addChild(invLabel);

	BombeInversionsItem *invRootItem = new BombeInversionsItem();
	invRootItem->text = "Root only";
	invRootItem->bombe = bombe;
	invRootItem->allowedInversions = 0;
	menu->addChild(invRootItem);

	BombeInversionsItem *invFirstItem = new BombeInversionsItem();
	invFirstItem->text = "Root and First";
	invFirstItem->bombe = bombe;
	invFirstItem->allowedInversions = 1;
	menu->addChild(invFirstItem);

	BombeInversionsItem *invSecondItem = new BombeInversionsItem();
	invSecondItem->text = "Root, First and Second";
	invSecondItem->bombe = bombe;
	invSecondItem->allowedInversions = 2;
	menu->addChild(invSecondItem);

	return menu;
}

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, Bombe) {
   Model *modelBombe = Model::create<Bombe, BombeWidget>( "Amalgamated Harmonics", "Bombe", "Bombe", SEQUENCER_TAG);
   return modelBombe;
}

// ♯♭
