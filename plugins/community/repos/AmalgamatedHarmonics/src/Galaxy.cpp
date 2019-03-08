#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"
#include "dsp/digital.hpp"

#include <iostream>

namespace rack_plugin_AmalgamatedHarmonics {

struct Galaxy : AHModule {

	const static int NUM_PITCHES = 6;
	const static int N_QUALITIES = 6;
	const static int N_NOTES = 12;
	const static int QMAP_SIZE = 20;

	std::string degNames[42] { // Degree * 3 + Quality
		"I",
		"I7",
		"im7",
		"IM7",
		"i",
		"i°",
		"II",
		"II7",
		"iim7",
		"IIM7",
		"ii",
		"ii°",
		"III",
		"III7",
		"iiim7",
		"IIIM7",
		"iii",
		"iii°",
		"IV",
		"IV7",
		"ivm7",
		"IVM7",
		"iv",
		"iv°",
		"V",
		"V7",
		"vm7",
		"VM7",
		"v",
		"v°",
		"VI",
		"VI7",
		"vim7",
		"VIM7",
		"vi",
		"vi°",
		"VII",
		"VII7",
		"viim7",
		"VIIM7",
		"vii",
		"vii°"
	};

	enum ParamIds {
		KEY_PARAM,
		MODE_PARAM,
		BAD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		MOVE_INPUT,
		KEY_INPUT,
		MODE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(PITCH_OUTPUT,6),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(NOTE_LIGHT,72),
		ENUMS(BAD_LIGHT,2),
		NUM_LIGHTS
	};
	
	Galaxy() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	void getFromRandom();
	void getFromKey();
	void getFromKeyMode();

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

	int ChordTable[N_QUALITIES] = { 1, 31, 78, 25, 71, 91 }; // M, 7, m7, M7, m, dim
	int QualityMap[3][QMAP_SIZE] = { 
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,1},
		{4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,2,2,2,1},
		{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5}
	};
	int InversionMap[3][QMAP_SIZE] = { 
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2},
	};


	float outVolts[NUM_PITCHES];
	
	int poll = 50000;

	SchmittTrigger moveTrigger;

	int degree = 0;
	int quality = 0;
	int noteIndex = 0; 
	int inversion = 0;

	int lastQuality = 0;
	int lastNoteIndex = 0; 
	int lastInversion = 0;

	int currRoot = 1;
	int currMode = 1;
	int light = 0;

	bool haveRoot = false;
	bool haveMode = false;

	int offset = 12; 	   // 0 = random, 12 = lower octave, 24 = repeat, 36 = upper octave
	int mode = 1; 	   // 0 = random chord, 1 = chord in key, 2 = chord in mode
	int allowedInversions = 0; // 0 = root only, 1 = root + first, 2 = root, first, second

	std::string rootName = "";
	std::string modeName = "";

	std::string chordName = "";
	std::string chordExtName = "";
};

void Galaxy::step() {
	
	AHModule::step();

	int badLight = 0;

	// Get inputs from Rack
	bool move = moveTrigger.process(inputs[MOVE_INPUT].value);

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

	if (mode == 1) {
		rootName = CoreUtil().noteNames[currRoot];
		modeName = "";
	} else if (mode == 2) {
		rootName = CoreUtil().noteNames[currRoot];
		modeName = CoreUtil().modeNames[currMode];
	} else {
		rootName = "";
		modeName = "";
		chordExtName = "";
	}

	if (move) {

		bool changed = false;
		bool haveMode = false;

		// std::cout << "Str position: Root: " << currRoot << 
		// 	" Mode: " << currMode << 
		// 	" degree: " << degree << 
		// 	" quality: " << quality <<
		// 	" noteIndex: " << noteIndex << std::endl;

		if (mode == 0) {
			getFromRandom();
		} else if (mode == 1) {

			if (randomUniform() < params[BAD_PARAM].value) {
				badLight = 2;
				getFromRandom();
			} else {
				getFromKey();
			}

		} else if (mode == 2) {

			float excess = params[BAD_PARAM].value - randomUniform();

			if (excess < 0.0) {
				getFromKeyMode();
				haveMode = true;
			} else {
				if (excess < 0.2) {
					badLight = 1;
					getFromKey();
				} else {
					badLight = 2;
					getFromRandom();
				}
			}

		}

		inversion = InversionMap[allowedInversions][rand() % QMAP_SIZE];
		int chord = ChordTable[quality];

		// Determine which chord corresponds to the grid position
		int *chordArray;
		switch(inversion) {
			case 0: 	chordArray = CoreUtil().ChordTable[chord].root; 	break;
			case 1: 	chordArray = CoreUtil().ChordTable[chord].first; 	break;
			case 2: 	chordArray = CoreUtil().ChordTable[chord].second;	break;
			default: 	chordArray = CoreUtil().ChordTable[chord].root;
		}

		// std::cout << "End position: Root: " << currRoot << 
		// 	" Mode: " << currMode << 
		// 	" Degree: " << degree << 
		// 	" Quality: " << quality <<
		// 	" Inversion: " << inversion << " " << chordArray <<
		// 	" NoteIndex: " << noteIndex << std::endl << std::endl;

		if (quality != lastQuality) {
			changed = true;
			lastQuality = quality;
		}

		if (noteIndex != lastNoteIndex) {
			changed = true;
			lastNoteIndex = noteIndex;
		}

		if (inversion != lastInversion) {
			changed = true;
			lastInversion = inversion;
		}

		// Determine which notes corresponds to the chord
		for (int j = 0; j < NUM_PITCHES; j++) {
 
		// Set the pitches for this step. If the chord has less than 6 notes, the empty slots are
		// filled with repeated notes. These notes are identified by a  24 semi-tome negative
		// offset. We correct for that offset now, pitching thaem back into the original octave.
		// They could be pitched into the octave above (or below)
			if (chordArray[j] < 0) {
				int off = offset;
				if (off == 0) {
					off = (rand() % 3 + 1) * 12;
				}
				outVolts[j] = CoreUtil().getVoltsFromPitch(chordArray[j] + off, noteIndex);			
			} else {
				outVolts[j] = CoreUtil().getVoltsFromPitch(chordArray[j], noteIndex);			
			}	
		}

		int newlight = noteIndex + (quality * N_NOTES);

		if (changed) {

			int chordIndex = ChordTable[quality];

			chordName = 
				CoreUtil().noteNames[noteIndex] + 
				CoreUtil().ChordTable[chordIndex].quality + " " + 
				CoreUtil().inversionNames[inversion];

			if (mode == 2) {
				if (haveMode) {
					chordExtName = degNames[degree * 6 + quality];
				} else {
					chordExtName = "";
				} 
			}

			lights[NOTE_LIGHT + light].value = 0.0f;
			lights[NOTE_LIGHT + newlight].value = 1.0f;
			light = newlight;

		}


	}

	if (badLight == 1) { // Green (scale->key)
		lights[BAD_LIGHT].setBrightnessSmooth(1.0f);
		lights[BAD_LIGHT + 1].setBrightnessSmooth(0.0f);
	} else if (badLight == 2) { // Red (->random)
		lights[BAD_LIGHT].setBrightnessSmooth(0.0f);
		lights[BAD_LIGHT + 1].setBrightnessSmooth(1.0f);
	} else { // No change
		lights[BAD_LIGHT].setBrightnessSmooth(0.0f);
		lights[BAD_LIGHT + 1].setBrightnessSmooth(0.0f);
	}

	// Set the output pitches and lights
	for (int i = 0; i < NUM_PITCHES; i++) {
		outputs[PITCH_OUTPUT + i].value = outVolts[i];
	}

}

void Galaxy::getFromRandom() {

	int rotSign = rand() % 2 ? 1 : -1;
	int rotateInput = rotSign * (rand() % 1 + 1); // -2 to 2

	int radSign = rand() % 2 ? 1 : -1;
	int radialInput = radSign * (rand() % 2 + 1); // -2 to 2

	// std::cout << "Rotate: " << rotateInput << "  Radial: " << radialInput << std::endl;

	// Determine move around the grid
	quality += rotateInput;
	if (quality < 0) {
		quality += N_QUALITIES;
	} else if (quality >= N_QUALITIES) {
		quality -= N_QUALITIES;
	}

	noteIndex += radialInput;
	if (noteIndex < 0) {
		noteIndex += N_NOTES;
	} else if (noteIndex >= N_NOTES) {
		noteIndex -= N_NOTES;
	}

}

void Galaxy::getFromKey() {

	int rotSign = rand() % 2 ? 1 : -1;
	int rotateInput = rotSign * (rand() % 1 + 1); // -2 to 2

	int radSign = rand() % 2 ? 1 : -1;
	int radialInput = radSign * (rand() % 2 + 1); // -2 to 2

	// std::cout << "Rotate: " << rotateInput << "  Radial: " << radialInput << std::endl;

	// Determine move around the grid
	quality += rotateInput;
	if (quality < 0) {
		quality += N_QUALITIES;
	} else if (quality >= N_QUALITIES) {
		quality -= N_QUALITIES;
	}

	// Just major scale
	int *curScaleArr = CoreUtil().ASCALE_IONIAN;
	int notesInScale = LENGTHOF(CoreUtil().ASCALE_IONIAN);

	// Determine move through the scale
	degree += radialInput; 
	if (degree < 0) {
		degree += notesInScale;
	} else if (degree >= notesInScale) {
		degree -= notesInScale;
	}

	noteIndex = (currRoot + curScaleArr[degree]) % 12;

}

void Galaxy::getFromKeyMode() {

	int rotSign = rand() % 2 ? 1 : -1;
	int rotateInput = rotSign * (rand() % 1 + 1); // -2 to 2

	// Determine move through the scale
	degree += rotateInput;
	if (degree < 0) {
		degree += Core::NUM_DEGREES;
	} else if (degree >= Core::NUM_DEGREES) {
		degree -= Core::NUM_DEGREES;
	}

	// From the input root, mode and degree, we can get the root chord note and quality (Major,Minor,Diminshed)
	int q;
	CoreUtil().getRootFromMode(currMode,currRoot,degree,&noteIndex,&q);
	quality = QualityMap[q][rand() % QMAP_SIZE];

}

struct GalaxyDisplay : TransparentWidget {
	
	Galaxy *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	GalaxyDisplay() {
		font = Font::load(assetPlugin(plugin, "res/EurostileBold.ttf"));
	}

	void draw(NVGcontext *vg) override {
	
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(255, 0, 0, 0xff));
		nvgTextLetterSpacing(vg, -1);

		char text[128];

		snprintf(text, sizeof(text), "%s", module->chordName.c_str());
		nvgText(vg, box.pos.x + 5, box.pos.y, text, NULL);

		snprintf(text, sizeof(text), "%s", module->chordExtName.c_str());
		nvgText(vg, box.pos.x + 5, box.pos.y + 11, text, NULL);

		nvgTextAlign(vg, NVG_ALIGN_RIGHT);
		snprintf(text, sizeof(text), "%s", module->rootName.c_str());
		nvgText(vg, box.size.x - 5, box.pos.y, text, NULL);

		snprintf(text, sizeof(text), "%s", module->modeName.c_str());
		nvgText(vg, box.size.x - 5, box.pos.y + 11, text, NULL);

	}
	
};

struct GalaxyWidget : ModuleWidget {

	Menu *createContextMenu() override;

	GalaxyWidget(Galaxy *module) : ModuleWidget(module) {
	
		UI ui;
		
		box.size = Vec(240, 380);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Galaxy.svg")));
			addChild(panel);
		}

		{
			GalaxyDisplay *display = new GalaxyDisplay();
			display->module = module;
			display->box.pos = Vec(0, 20);
			display->box.size = Vec(240, 230);
			addChild(display);
		}

		float div = (M_PI * 2) / (float)Galaxy::N_QUALITIES;
		float div2 = (M_PI * 2) / (float)(Galaxy::N_QUALITIES * Galaxy::N_QUALITIES);

		for (int q = 0; q < Galaxy::N_QUALITIES; q++) {

			for (int n = 0; n < Galaxy::N_NOTES; n++) {

				float cosDiv = cos(div * q + div2 * n);
				float sinDiv = sin(div * q + div2 * n);

				float xPos  = sinDiv * (32.5 + (7.5 * n));
				float yPos  = cosDiv * (32.5 + (7.5 * n));

				int l = n + (q * Galaxy::N_NOTES);

				addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(xPos + 110.5, 149.5 - yPos), module, Galaxy::NOTE_LIGHT + l));

			}

		}
		
		for (int i = 0; i < 6; i++) {
			addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, i, 5, true, false), Port::OUTPUT, module, Galaxy::PITCH_OUTPUT + i));
		}	

		addInput(Port::create<PJ301MPort>(Vec(102, 140), Port::INPUT, module, Galaxy::MOVE_INPUT));

		addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 0, 4, true, false), module, Galaxy::KEY_PARAM, 0.0, 11.0, 0.0)); 
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 1, 4, true, false), Port::INPUT, module, Galaxy::KEY_INPUT));

		addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 4, 4, true, false), module, Galaxy::MODE_PARAM, 0.0, 6.0, 0.0)); 
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 5, 4, true, false), Port::INPUT, module, Galaxy::MODE_INPUT));

		Vec trim = ui.getPosition(UI::TRIMPOT, 5, 3, true, false);
		trim.x += 15;
		trim.y += 25;

		addParam(ParamWidget::create<AHTrimpotNoSnap>(trim, module, Galaxy::BAD_PARAM, 0.0, 1.0, 0.0)); 

		trim.x += 15;
		trim.y += 20;
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(trim, module, Galaxy::BAD_LIGHT));

	}

};

struct GalOffsetItem : MenuItem {
	Galaxy *gal;
	int offset;
	void onAction(EventAction &e) override {
		gal->offset = offset;
	}
	void step() override {
		rightText = (gal->offset == offset) ? "✔" : "";
	}
};

struct GalModeItem : MenuItem {
	Galaxy *gal;
	int mode;
	void onAction(EventAction &e) override {
		gal->mode = mode;
	}
	void step() override {
		rightText = (gal->mode == mode) ? "✔" : "";
	}
};

struct GalInversionsItem : MenuItem {
	Galaxy *gal;
	int allowedInversions;
	void onAction(EventAction &e) override {
		gal->allowedInversions = allowedInversions;
	}
	void step() override {
		rightText = (gal->allowedInversions == allowedInversions) ? "✔" : "";
	}
};

Menu *GalaxyWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Galaxy *gal = dynamic_cast<Galaxy*>(module);
	assert(gal);

	MenuLabel *offsetLabel = new MenuLabel();
	offsetLabel->text = "Repeat Notes";
	menu->addChild(offsetLabel);

	GalOffsetItem *offsetLowerItem = new GalOffsetItem();
	offsetLowerItem->text = "Lower";
	offsetLowerItem->gal = gal;
	offsetLowerItem->offset = 12;
	menu->addChild(offsetLowerItem);

	GalOffsetItem *offsetRepeatItem = new GalOffsetItem();
	offsetRepeatItem->text = "Repeat";
	offsetRepeatItem->gal = gal;
	offsetRepeatItem->offset = 24;
	menu->addChild(offsetRepeatItem);

	GalOffsetItem *offsetUpperItem = new GalOffsetItem();
	offsetUpperItem->text = "Upper";
	offsetUpperItem->gal = gal;
	offsetUpperItem->offset = 36;
	menu->addChild(offsetUpperItem);

	GalOffsetItem *offsetRandomItem = new GalOffsetItem();
	offsetRandomItem->text = "Random";
	offsetRandomItem->gal = gal;
	offsetRandomItem->offset = 0;
	menu->addChild(offsetRandomItem);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Chord Selection";
	menu->addChild(modeLabel);

	GalModeItem *modeRandomItem = new GalModeItem();
	modeRandomItem->text = "Random";
	modeRandomItem->gal = gal;
	modeRandomItem->mode = 0;
	menu->addChild(modeRandomItem);

	GalModeItem *modeKeyItem = new GalModeItem();
	modeKeyItem->text = "in Key";
	modeKeyItem->gal = gal;
	modeKeyItem->mode = 1;
	menu->addChild(modeKeyItem);

	GalModeItem *modeModeItem = new GalModeItem();
	modeModeItem->text = "in Mode";
	modeModeItem->gal = gal;
	modeModeItem->mode = 2;
	menu->addChild(modeModeItem);

	MenuLabel *invLabel = new MenuLabel();
	invLabel->text = "Allowed Chord Inversions";
	menu->addChild(invLabel);

	GalInversionsItem *invRootItem = new GalInversionsItem();
	invRootItem->text = "Root only";
	invRootItem->gal = gal;
	invRootItem->allowedInversions = 0;
	menu->addChild(invRootItem);

	GalInversionsItem *invFirstItem = new GalInversionsItem();
	invFirstItem->text = "Root and First";
	invFirstItem->gal = gal;
	invFirstItem->allowedInversions = 1;
	menu->addChild(invFirstItem);

	GalInversionsItem *invSecondItem = new GalInversionsItem();
	invSecondItem->text = "Root, First and Second";
	invSecondItem->gal = gal;
	invSecondItem->allowedInversions = 2;
	menu->addChild(invSecondItem);

	return menu;
}

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, Galaxy) {
   Model *modelGalaxy = Model::create<Galaxy, GalaxyWidget>( "Amalgamated Harmonics", "Galaxy", "Galaxy", SEQUENCER_TAG);
   return modelGalaxy;
}

// ♯♭
