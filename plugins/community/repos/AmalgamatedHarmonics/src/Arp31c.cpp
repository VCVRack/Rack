#include "AH.hpp"
#include "Core.hpp"
#include "UI.hpp"
#include "componentlibrary.hpp"
#include "dsp/digital.hpp"

#include <iostream>

namespace rack_plugin_AmalgamatedHarmonics {

struct Arpeggio {

	virtual std::string getName() = 0;

	virtual void initialise(int nPitches, int offset) = 0;
	
	virtual void advance() = 0;
	
	virtual int getPitch() = 0;
	
	virtual bool isArpeggioFinished() = 0;

	// For RL and LR arps we have the following logic
	// Convert from npitch (1-6) to index (0 -> 9), but do no repeat first note
	// 1,2,3,4,5,6 (6) -> 
	// 0 (1)
	// 1 (2)
	// 2 (3)
	// 3 (4)
	// 4 (5)
	// 5 (6)
	// 6 (5)
	// 7 (3)
	// 8 (2)
	// 9 (END, do not repeat 1)

};

struct RightArp : Arpeggio {

	int index = 0;
	int nPitches = 0;

	std::string getName() override {
		return "Right";
	};

	void initialise(int np, int offset) override {

		if (offset != 0) {
			index = offset % np;
		} else {
			index = 0;
		}

		nPitches = np;

//		std::cout << nPitches << " " << offset << " " << index << std::endl;

	}
	
	void advance() override {
		index++;
	}
	
	int getPitch() override {
		return index;
	}
	
	bool isArpeggioFinished() override {
		return (index >= nPitches - 1);
	}
	
};

struct LeftArp : Arpeggio {

	int index = 0;
	int nPitches = 0;

	std::string getName() override {
		return "Left";
	};
	
	void initialise(int np, int offset) override {

		if (offset != 0) {
			offset = offset % np;
			index = np - offset - 1;
		} else {
			index = np - 1;
		}

		nPitches = np;

//		std::cout << nPitches << " " << offset << " " << index << std::endl;

	}
	
	void advance() override {
		index--;
	}
	
	int getPitch() override {
		return index;
	}

	bool isArpeggioFinished() override {
		return (index == 0);
	}
	
};

struct RightLeftArp : Arpeggio {

	int currSt = 0;
	int mag = 0; // index of last pitch
	int end = 0; // index of end of arp
	int nPitches = 0;
	
	std::string getName() override {
		return "RightLeft";
	};	

	void initialise(int np, int offset) override {

		nPitches = np;
		mag = np - 1;
		end = 2 * mag - 1; 

		if (end < 1) {
			end = 1;
		}

		currSt = offset;

		if (end < currSt) {
			end = currSt;
		} else {
			if (offset > 0) {
				end++;
			}
		}

	}
	
	void advance() override {
		currSt++;
	}
	
	int getPitch() override {

		int p = abs((mag - abs(mag - currSt)) % nPitches);
		return p;
	}

	bool isArpeggioFinished() override {
		return(currSt == end);
	}
	
};

struct LeftRightArp : Arpeggio {

	int currSt = 0;
	int mag = 0;
	int end = 0;
	int nPitches = 0;
	
	std::string getName() override {
		return "LeftRight";
	};	

	void initialise(int np, int offset) override {

		nPitches = np;
		mag = np - 1;
		end = 2 * mag - 1;

		if (end < 1) {
			end = 1;
		}

		currSt = offset;

		if (end < currSt) {
			end = currSt;
		} else {
			if (offset > 0) {
				end++;
			}
		}

	}
	
	void advance() override {
		currSt++;
	}
	
	int getPitch() override {

		int p = abs(abs(mag - currSt) % nPitches);
		return p;

	}

	bool isArpeggioFinished() override {
		return(currSt == end);
	}
	
};


struct Arp31 : AHModule {
	
	const static int MAX_STEPS = 16;
	const static int MAX_DIST = 12; //Octave
	const static int NUM_PITCHES = 6;

	enum ParamIds {
		ARP_PARAM,
		OFFSET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		ENUMS(PITCH_INPUT,6),
		ARP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		GATE_OUTPUT,
		EOC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(CURR_LIGHT,6),
		NUM_LIGHTS
	};
	
	Arp31() : AHModule(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		reset();
		id = rand();
        debugFlag = false;
	}

	void step() override;
	
	void reset() override {
		isRunning = false;
	}
	
	json_t *toJson() override {
		json_t *rootJ = json_object();

		// gateMode
		json_t *gateModeJ = json_integer((int) gateMode);
		json_object_set_new(rootJ, "gateMode", gateModeJ);

		return rootJ;
	}
	
	void fromJson(json_t *rootJ) override {
		// gateMode
		json_t *gateModeJ = json_object_get(rootJ, "gateMode");
		
		if (gateModeJ) {
			gateMode = (GateMode)json_integer_value(gateModeJ);
		}
	}
	
	enum GateMode {
		TRIGGER,
		RETRIGGER,
		CONTINUOUS,
	};
	GateMode gateMode = TRIGGER;
	
	SchmittTrigger clockTrigger; // for clock
	
	PulseGenerator gatePulse;
	PulseGenerator eocPulse;

	bool locked = false;

	float outVolts = 0;
	bool isRunning = false;
	int error = 0;
	
	int inputArp = 0;
	int arp = 0;
	
	int poll = 5000;
		
	float semiTone = 1.0 / 12.0;

	RightArp 		arp_right;
	LeftArp 		arp_left;
	RightLeftArp 	arp_rightleft;
	LeftRightArp 	arp_leftright;

	RightArp 		ui_arp_right;
	LeftArp 		ui_arp_left;
	RightLeftArp 	ui_arp_rightleft;
	LeftRightArp 	ui_arp_leftright;

	Arpeggio *currArp = &arp_right;
	Arpeggio *uiArp = &arp_right;
	
	float pitches[6];
	int pitchIndex[NUM_PITCHES];
	int nPitches = 0;
	int id = 0;
	int currLight = 0;

};

void Arp31::step() {
	
	AHModule::step();

	// Wait a few steps for the inputs to flow through Rack
	if (stepX < 10) { 
		return;
	}
	
	// Get inputs from Rack
	float clockInput	= inputs[CLOCK_INPUT].value;
	bool  clockActive	= inputs[CLOCK_INPUT].active;
	
	if (inputs[ARP_INPUT].active) {
		inputArp = inputs[ARP_INPUT].value;
	} else {
		inputArp = params[ARP_PARAM].value;
	}	

	int offset = params[OFFSET_PARAM].value;

	// Process inputs
	bool clockStatus	= clockTrigger.process(clockInput);
	
	// If there is no clock input, then force that we are not running
	if (!clockActive) {
		isRunning = false;
	}

	bool restart = false;
	int oldLight = 0;
	// Have we been clocked?

	if (clockStatus) {

		// If we are already running, process cycle
		if (isRunning) {

			if (debugEnabled()) { std::cout << stepX << " " << id  << " Advance Cycle: " << currArp->getPitch() << " " << pitches[currArp->getPitch()] << std::endl; }

			// Reached the end of the pattern?
			if (currArp->isArpeggioFinished()) {

				// Pulse the EOC gate
				eocPulse.trigger(Core::TRIGGER);

				if (debugEnabled()) { std::cout << stepX << " " << id  << " Finished Cycle" << std::endl; }
				restart = true;

			} 
							
			// Finally set the out voltage
			int i = currArp->getPitch();
			outVolts = clamp(pitches[i], -10.0f, 10.0f);
			oldLight = currLight;
			currLight = pitchIndex[i];

			if (debugEnabled()) { std::cout << stepX << " " << id  << " Index: " << i << " V: " << outVolts << " Light: " << currLight << std::endl; }

			// Pulse the output gate
			gatePulse.trigger(Core::TRIGGER);

			// Completed 1 step
			currArp->advance();

		} else {

			// Start a cycle
			restart = true;

		}

	}
	
	// If we have been triggered, start a new sequence
	if (restart) {

		// Read input pitches and assign to pitch array
		int nValidPitches = 0;
		float inputPitches[NUM_PITCHES];
		for (int p = 0; p < NUM_PITCHES; p++) {
			int index = PITCH_INPUT + p;
			if (inputs[index].active) {
				inputPitches[nValidPitches] = inputs[index].value;
				pitchIndex[nValidPitches] = p;
				nValidPitches++;
			} else {
				inputPitches[nValidPitches] = 0.0;
				pitchIndex[nValidPitches] = 0;
			}
		}

		// if (debugEnabled()) {
		// 	for (int p = 0; p < nValidPitches; p++) {
		// 		std::cout << inputPitches[p] << std::endl;
		// 	}
		// }
		
		// Always play something
		if (nValidPitches == 0) {
			if (debugEnabled()) { std::cout << stepX << " " << id  << " No inputs, assume single 0V pitch" << std::endl; }
			inputPitches[0] = 0.0;
			pitchIndex[0] = 0;
			nValidPitches = 1;
		}

		// At the first step of the cycle
		// So this is where we tweak the cycle parameters
		arp = inputArp;
		
		switch(arp) {
			case 0: 	currArp = &arp_right;		break;
			case 1: 	currArp = &arp_left;		break;
			case 2: 	currArp = &arp_rightleft;	break;
			case 3: 	currArp = &arp_leftright;	break;
			default:	currArp = &arp_right;		break; 	
		};
		
		// Copy pitches
		for (int p = 0; p < nValidPitches; p++) {
			pitches[p] = inputPitches[p];
		}
		nPitches = nValidPitches;

		if (debugEnabled()) { std::cout << stepX << " " << id  << " Initiatise new Cycle: Pattern: " << currArp->getName() << " nPitches: " << nPitches << std::endl; }
		
		currArp->initialise(nPitches, offset);

		// Start
		isRunning = true;
		
	} 

	// Update UI
	switch(inputArp) {
		case 0: 	uiArp = &ui_arp_right;		break;
		case 1: 	uiArp = &ui_arp_left;		break;
		case 2: 	uiArp = &ui_arp_rightleft;	break;
		case 3: 	uiArp = &ui_arp_leftright;	break;
		default:	uiArp = &ui_arp_right;		break; 	
	};
	
//	uiArp->initialise(nValidPitches, 1);
	
	// Set the value
	outputs[OUT_OUTPUT].value = outVolts;

	// Set the light
	lights[CURR_LIGHT + oldLight].value = 0.0;
	lights[CURR_LIGHT + currLight].value = 1.0;
	
	bool gPulse = gatePulse.process(delta);
	bool cPulse = eocPulse.process(delta);
	
	bool gatesOn = isRunning;
	if (gateMode == TRIGGER) {
		gatesOn = gatesOn && gPulse;
	} else if (gateMode == RETRIGGER) {
		gatesOn = gatesOn && !gPulse;
	}
	
	outputs[GATE_OUTPUT].value = gatesOn ? 10.0 : 0.0;
	outputs[EOC_OUTPUT].value = cPulse ? 10.0 : 0.0;
	
}

struct Arp31Display : TransparentWidget {
	
	Arp31 *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	Arp31Display() {
		font = Font::load(assetPlugin(plugin, "res/EurostileBold.ttf"));
	}

	void draw(NVGcontext *vg) override {
	
		Vec pos = Vec(0, 15);

		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -1);

		nvgFillColor(vg, nvgRGBA(255, 0, 0, 0xff));
	
		char text[128];
		snprintf(text, sizeof(text), "%s", module->uiArp->getName().c_str());
		nvgText(vg, pos.x + 10, pos.y + 65, text, NULL);

	}
	
};

struct Arp31Widget : ModuleWidget {
	Arp31Widget(Arp31 *module);
	Menu *createContextMenu() override;
};

Arp31Widget::Arp31Widget(Arp31 *module) : ModuleWidget(module) {
	
	UI ui;
	
	box.size = Vec(135, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Arp31c.svg")));
		addChild(panel);
	}

	{
		Arp31Display *display = new Arp31Display();
		display->module = module;
		display->box.pos = Vec(40, 100);
		display->box.size = Vec(100, 70);
		addChild(display);
	}

	addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 5, true, false), Port::OUTPUT, module, Arp31::OUT_OUTPUT));
	addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 1, 5, true, false), Port::OUTPUT, module, Arp31::GATE_OUTPUT));
	addOutput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 2, 5, true, false), Port::OUTPUT, module, Arp31::EOC_OUTPUT));

	for (int i = 0; i < 3; i++) {
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, i, 0, true, false), Port::INPUT, module, Arp31::PITCH_INPUT + i));
		Vec v = ui.getPosition(UI::LIGHT, i, 1, true, false);
		v.x = v.x + 2;
		v.y = 75;
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(v, module, Arp31::CURR_LIGHT + i));
	}

	for (int i = 3; i < 6; i++) {
		addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, i - 3, 1, true, false), Port::INPUT, module, Arp31::PITCH_INPUT + i));
		Vec v = ui.getPosition(UI::LIGHT, i - 3, 2, true, false);
		v.x = v.x + 2;
		v.y = 131;
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(v, module, Arp31::CURR_LIGHT + i));
	}

	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 4, true, false), Port::INPUT, module, Arp31::CLOCK_INPUT));
	addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 1, 4, true, false), module, Arp31::OFFSET_PARAM, 0.0, 10.0, 0.0)); 

	addParam(ParamWidget::create<AHKnobSnap>(ui.getPosition(UI::KNOB, 0, 2, true, false), module, Arp31::ARP_PARAM, 0.0, 3.0, 0.0)); 
	addInput(Port::create<PJ301MPort>(ui.getPosition(UI::PORT, 0, 3, true, false), Port::INPUT, module, Arp31::ARP_INPUT));
	
}

struct ArpGateModeItem : MenuItem {
	Arp31 *arp;
	Arp31::GateMode gateMode;
	void onAction(EventAction &e) override {
		arp->gateMode = gateMode;
	}
	void step() override {
		rightText = (arp->gateMode == gateMode) ? "✔" : "";
	}
};

Menu *Arp31Widget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	Arp31 *arp = dynamic_cast<Arp31*>(module);
	assert(arp);

	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Gate Mode";
	menu->addChild(modeLabel);

	ArpGateModeItem *triggerItem = new ArpGateModeItem();
	triggerItem->text = "Trigger";
	triggerItem->arp = arp;
	triggerItem->gateMode = Arp31::TRIGGER;
	menu->addChild(triggerItem);

	ArpGateModeItem *retriggerItem = new ArpGateModeItem();
	retriggerItem->text = "Retrigger";
	retriggerItem->arp = arp;
	retriggerItem->gateMode = Arp31::RETRIGGER;
	menu->addChild(retriggerItem);

	ArpGateModeItem *continuousItem = new ArpGateModeItem();
	continuousItem->text = "Continuous";
	continuousItem->arp = arp;
	continuousItem->gateMode = Arp31::CONTINUOUS;
	menu->addChild(continuousItem);

	return menu;
}

} // namespace rack_plugin_AmalgamatedHarmonics

using namespace rack_plugin_AmalgamatedHarmonics;

RACK_PLUGIN_MODEL_INIT(AmalgamatedHarmonics, Arp31) {
   Model *modelArp31 = Model::create<Arp31, Arp31Widget>( "Amalgamated Harmonics", "Arp31", "Arp 3.1 - Chord", ARPEGGIATOR_TAG);
   return modelArp31;
}

