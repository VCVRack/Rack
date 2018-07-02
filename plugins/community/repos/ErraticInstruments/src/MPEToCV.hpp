#include "global_pre.hpp"
#include "Erratic.hpp"
#include "midi.hpp"
#include "MPEBaseWidget.hpp"
#include "global_ui.hpp"

struct MidiValue {
	int val = 0; // Controller value
	// TransitionSmoother tSmooth;
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

struct MPEPlusValue {
	uint16_t val = 0; // Controller value
	int MSB = 0 ;
	int LSB = 0;
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

struct MidiPedalValue {
	int val = 0; // Controller value
	int cc ; // need to set it
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

struct MPEToCV : Module {
	enum ParamIds {
		RESET_PARAM,
		NUM_PARAMS,
        BEND_RANGE_PARAM
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT = 0,
		GATE_OUTPUT,
		VELOCITY_OUTPUT,
		PRESSURE_OUTPUT,
		Y_OUTPUT,
		PEDAL_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RESET_LIGHT,
		VELOCITY_LIGHT,
		PRESSURE_LIGHT,
		Y_AXIS_LIGHT,
		PEDAL_LIGHT,
		NUM_LIGHTS
	};

	MidiInputQueue midiInput;

    int bendRange = 48; // our default is 48 (common for ROLI), Continuum has 96.
	int channel = 2; // Our default channel is 2. ROLI users will want to set this is 2
	int globalChannel = 16; // Our default channel is 16. ROLI users will want to set this is 1
	bool MPEPlus = false ; // This is specially useful for Haken Continuum
	int YaxisCC = 74 ;

	bool newNote = false;
	std::list<int> notes;
	bool pedal = false;
	int note = 60; // C4, most modules should use 261.626 Hz
	int vel = 0;
	MidiValue mod;
	MidiValue afterTouch;
	MidiValue pitchWheel;
	MidiValue Yaxis ;
	MidiPedalValue midiPedalOne ;

	// Used for MPE+ 
	MPEPlusValue MPEPlusyAxis, MPEPluszAxis ;
	bool gate = false;

	// Reset note parameters when you release, ie, receive note off
	bool noteOffReset = true;
	bool resetNoteNow = false;

	// SchmittTrigger resetTrigger;

	MPEToCV();

	// ~MPEToCV() {
	// };

	void step() override;

	void pressNote(int note);

	void releaseNote(int note);

	void processMessage(MidiMessage msg);

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		json_object_set_new(rootJ, "bendRange", json_integer(bendRange));
		json_object_set_new(rootJ, "midiChannel", json_integer(channel));
		json_object_set_new(rootJ, "globalMidiChannel", json_integer(globalChannel));
		json_object_set_new(rootJ, "MPEMode", json_integer(MPEPlus));
		json_object_set_new(rootJ, "noteOffReset", json_boolean(noteOffReset));
		return rootJ;
		
	}

	void fromJson(json_t *rootJ) override {
		json_t *midiJ = json_object_get(rootJ, "midi");
		midiInput.fromJson(midiJ);
		json_t *bendRangeJ = json_object_get(rootJ, "bendRange");
		if (bendRangeJ) {
			bendRange = json_integer_value(bendRangeJ);
		}
		json_t *midiChannelJ = json_object_get(rootJ, "midiChannel");
		if (midiChannelJ) {
			channel = json_integer_value(midiChannelJ);
		}
		json_t *globalMidiChannelJ = json_object_get(rootJ, "globalMidiChannel");
		if (globalMidiChannelJ) {
			globalChannel = json_integer_value(globalMidiChannelJ);
		}
		json_t *MPEModeJ = json_object_get(rootJ, "MPEMode");
		if (MPEModeJ) {
			MPEPlus = json_integer_value(MPEModeJ);
		}
		json_t *noteOffResetJ = json_object_get(rootJ, "noteOffReset");
		if (noteOffResetJ) {
			noteOffReset = json_boolean_value(noteOffResetJ);
		}
	}

};

// MPEMidiWidget stuff

struct BendRangeItem : MenuItem {
	MPEToCV *mpetocv;
	int bendRange ;
	void onAction(EventAction &e) override {
		mpetocv->bendRange = bendRange;
	}
};

struct BendRangeChoice : LedDisplayChoice {
	// MPEToCVWidget *mpetocvwidget;
	MPEToCV *mpetocv;
	
	int bendRange ;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
		Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else
		Menu *menu = gScene->createMenu();
#endif // USE_VST2
			menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Bend Range"));
			std::vector<int> bendRanges = {1,2,3,4,12,24,48,96}; // The bend range we use
			for (auto const& bendRangeValue: bendRanges) {
				BendRangeItem *item = new BendRangeItem();
				item->mpetocv = mpetocv;
				item->text = std::to_string(bendRangeValue);
				item->bendRange = bendRangeValue;
				menu->addChild(item);
			}
		// mpetocv->bendRange = bendRange;
	}
	void step() override {
		color = nvgRGB(0xff, 0x00, 0x00);
		color.a = 0.8f;
		text = stringf("%d", mpetocv->bendRange);
		// text = stringf("Range: %d semitones", mpetocv->bendRange);
		// rightText = (mpetocv->bendRange==bendRange) ? "✔" : "";
	}
};

struct MidiChannelItem : MenuItem {
	MPEToCV *mpetocv;
	int channel ;
	void onAction(EventAction &e) override {
		mpetocv->channel = channel;
	}
};

struct MidiChannelChoice : LedDisplayChoice {
	// MPEToCVWidget *mpetocvwidget;
	MPEToCV *mpetocv;
	
	int channel ;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
	   	Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else  
 	   	Menu *menu = gScene->createMenu();
#endif // USE_VST2
			menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Midi channel"));
			std::vector<int> bendRanges = {1,2,3,4,12,24,48,96}; // The bend range we use
			for (int c=1; c <= 16 ; c++) {
				MidiChannelItem *item = new MidiChannelItem();
				item->mpetocv = mpetocv;
				item->text = std::to_string(c);
				item->channel = c;
				menu->addChild(item);
			}
	}
	void step() override {
		color = nvgRGB(0xff, 0x00, 0x00);
		color.a = 0.8f;
		text = std::to_string(mpetocv->channel);
	}
};

struct GlobalMidiChannelItem : MenuItem {
	MPEToCV *mpetocv;
	int channel ;
	void onAction(EventAction &e) override {
		mpetocv->globalChannel = channel;
	}
};

struct GlobalMidiChannelChoice : LedDisplayChoice {
	// MPEToCVWidget *mpetocvwidget;
	MPEToCV *mpetocv;
	
	int channel ;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
		Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else
		Menu *menu = gScene->createMenu();
#endif // USE_VST2
			menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Global Midi channel"));
			for (int c=1; c <= 16 ; c++) {
				GlobalMidiChannelItem *item = new GlobalMidiChannelItem();
				item->mpetocv = mpetocv;
				item->text = std::to_string(c);
				item->channel = c;
				menu->addChild(item);
			}
	}
	void step() override {
		color = nvgRGB(0xff, 0x00, 0x00);
		color.a = 0.8f;
		text = std::to_string(mpetocv->globalChannel);
	}
};

struct MPEModeItem : MenuItem {
	MPEToCV *mpetocv;
	bool MPEPlus ;
	void onAction(EventAction &e) override {
		mpetocv->MPEPlus = MPEPlus;
	}
};

struct MPEModeChoice : LedDisplayChoice {
	// MPEToCVWidget *mpetocvwidget;
	MPEToCV *mpetocv;
	
	bool MPEPlus ;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
		Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else
		Menu *menu = gScene->createMenu();
#endif // USE_VST2
			menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MPE mode"));			
			// MPE
			MPEModeItem *MPE = new MPEModeItem();
			MPE->mpetocv = mpetocv;
			MPE->text = "MPE - Standard (ROLI, etc)";
			MPE->MPEPlus = false;
			menu->addChild(MPE);
			// MPE Plus
			MPEModeItem *MPEPlus = new MPEModeItem();
			MPEPlus->mpetocv = mpetocv;
			MPEPlus->text = "MPE+ - High Res for Haken Continuum";
			MPEPlus->MPEPlus = true;
			menu->addChild(MPEPlus);

	}
	void step() override {
		// color = nvgRGB(0xff, 0x00, 0x00);
		// color.a = 0.8f;
		if (mpetocv->MPEPlus) {
			text = "MPE+";
		} else {
			text = "MPE";
		}
	}
};

// We extend the midi to follow similar design
struct MPEMidiWidget : MPEBaseWidget {
	LedDisplaySeparator *hSeparators[2];
	LedDisplaySeparator *vSeparators[3];
	// LedDisplayChoice *ccChoices[4][4];
	MPEToCV *mpetocv ;
	BendRangeChoice *bendRangeChoice ;
	MidiChannelChoice *midiChannelChoice ;
	GlobalMidiChannelChoice *globalMidiChannelChoice ;
	MPEModeChoice *mpeModeChoice ;

	MPEMidiWidget() {
	}

	void initialize(MPEToCV *mpetocv) {
		this->mpetocv = mpetocv;
		Vec pos = deviceChoice->box.getBottomLeft();
		for (int y = 0; y < 2; y++) {
			hSeparators[y] = Widget::create<LedDisplaySeparator>(pos);
			addChild(hSeparators[y]);
		}

		midiChannelChoice = Widget::create<MidiChannelChoice>(pos);
		midiChannelChoice->mpetocv = mpetocv ;
		addChild(midiChannelChoice);

		globalMidiChannelChoice = Widget::create<GlobalMidiChannelChoice>(pos);
		globalMidiChannelChoice->mpetocv = mpetocv ;
		addChild(globalMidiChannelChoice);

		bendRangeChoice = Widget::create<BendRangeChoice>(pos);
		bendRangeChoice->mpetocv = mpetocv ;
		addChild(bendRangeChoice);

		mpeModeChoice = Widget::create<MPEModeChoice>(pos);
		mpeModeChoice->mpetocv = mpetocv ;
		addChild(mpeModeChoice);

		for (int x = 0; x < 3; x++) {
			vSeparators[x] = Widget::create<LedDisplaySeparator>(pos);
			vSeparators[x]->box.size.y = midiChannelChoice->box.size.y;
			addChild(vSeparators[x]);
		}

		// for (int x = 0; x < 3; x++) {
		// }
	}
	void step() override {
		MPEBaseWidget::step();
		
		midiChannelChoice->box.size.x = box.size.x/4;
		midiChannelChoice->box.pos.x = 0;

		globalMidiChannelChoice->box.size.x = box.size.x/4;
		globalMidiChannelChoice->box.pos.x = box.size.x/4;

		bendRangeChoice->box.size.x = box.size.x/4;
		bendRangeChoice->box.pos.x = box.size.x/4 * 2 ;
		
		mpeModeChoice->box.size.x = box.size.x/4;
		mpeModeChoice->box.pos.x = box.size.x/4 * 3 - 5 ;

		for (int y = 0; y < 2; y++) {
			hSeparators[y]->box.size.x = box.size.x;
		}
		
		for (int x = 0; x < 3; x++) {
			vSeparators[x]->box.pos.x = box.size.x / 4 * (x+1);
		}
		
	}


};
