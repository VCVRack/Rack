#include "Erratic.hpp"
#include "MPEToCV.hpp"

MPEToCV::MPEToCV() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
	pitchWheel.val = 64;
	// pitchWheel.tSmooth.set(0, 0);
	midiPedalOne.cc = 12; // By default we use 12 (barrel i on the Haken Continuum)
}


void MPEToCV::step() {
	MidiMessage msg;
	while (midiInput.shift(&msg)) {
		Ddebug("Processmsg");
		processMessage(msg);
	}
	
	// if (resetTrigger.process(params[RESET_PARAM].value)) {
	// 	resetMidi();
	// 	return;
	// }

	// lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / 0.55 / engineGetSampleRate(); // fade out light

	outputs[GATE_OUTPUT].value = gate ? 10.0 : 0.0;
	
	outputs[VELOCITY_OUTPUT].value = vel / 127.0 * 10.0;

	/* NOTE: I'll leave out value smoothing for after touch for now. I currently don't
	 * have an after touch capable device around and I assume it would require different
	 * smoothing*/

	// Pressure
	if (MPEPlus) {
		if (MPEPluszAxis.changed) {
			// Combine two 7 bit into 14bit
			MPEPluszAxis.val = ( (uint16_t)MPEPluszAxis.MSB << 7) | ( (uint16_t)MPEPluszAxis.LSB ) ;
			outputs[PRESSURE_OUTPUT].value = MPEPluszAxis.val / 16384.0 * 10.f;
			MPEPluszAxis.changed = false;
		}
		if (MPEPlusyAxis.changed) {
			// Combine two 7 bit into 14bit
			MPEPlusyAxis.val = ( (uint16_t)MPEPlusyAxis.MSB << 7) | ( (uint16_t)MPEPlusyAxis.LSB ) ;
			outputs[Y_OUTPUT].value = MPEPlusyAxis.val / 16384.0 * 10.f;
			//std::cout << "Y axis is " << outputs[Y_OUTPUT].value << std::endl;
			MPEPlusyAxis.changed = false;
		}
	} else {  // Standard resolution MPE
		if (afterTouch.changed) {
			outputs[PRESSURE_OUTPUT].value = afterTouch.val / 127.0 * 10;
			afterTouch.changed = false;
		}
		if (Yaxis.changed) {
			outputs[Y_OUTPUT].value = Yaxis.val / 127.0 * 10;
			Yaxis.changed = false;
		}
	}
	
	// Pedal
	if (midiPedalOne.changed) {
		outputs[PEDAL_OUTPUT].value = midiPedalOne.val / 127.f * 10.f ;
		midiPedalOne.changed = false;
	}

	// 1/V incorporates pitch wheel changes
	if (pitchWheel.changed && gate) {
		outputs[PITCH_OUTPUT].value = (((note - 60)) / 12.0) + ((pitchWheel.val - 8192 ) / 8192.0 / 12.0 * (float)bendRange ) ;
		Ddebug("%d",(pitchWheel.val - 8192 ));
		Ddebug("note=%d, pitchWheel.val=%d, bendRange=%d",note, pitchWheel.val, bendRange );
		Ddebug("outputs[PITCH_OUTPUT].value is %f",outputs[PITCH_OUTPUT].value);
		pitchWheel.changed = false;
		this->newNote = false;
	}

	if (resetNoteNow) {
		this->note = 60;
		this->pitchWheel.val = 8192;
		outputs[PITCH_OUTPUT].value = 0 ;
		resetNoteNow = false;
	}
}

// Currently only support one note
void MPEToCV::pressNote(int note) {
	// Remove existing similar note
	// auto it = std::find(notes.begin(), notes.end(), note);
	// if (it != notes.end())
	// 	notes.erase(it);
	// // Push note
	// notes.push_back(note);
	this->note = note;
	gate = true;
	this->newNote = true;
}

void MPEToCV::releaseNote(int note) {
	gate = false;

	if (noteOffReset) {					
		Ddebug("We execute the note off reset");
		resetNoteNow = true;
		afterTouch.val = 0;
		afterTouch.changed = true;
		Yaxis.val = 0;
		Yaxis.changed = true;

	}	

	// // Remove the note
	// auto it = std::find(notes.begin(), notes.end(), note);
	// if (it != notes.end())
	// 	notes.erase(it);

	// if (pedal) {
	// 	// Don't release if pedal is held
	// 	gate = true;
	// } else if (!notes.empty()) {
	// 	// Play previous note
	// 	auto it2 = notes.end();
	// 	it2--;
	// 	this->note = *it2;
	// 	gate = true;
	// } else {
	// 	gate = false;
	// }
}

void MPEToCV::processMessage(MidiMessage msg) {
	//Ddebug("MIDI: %01x %01x %02x %02x", msg.status(), msg.channel(), msg.data1, msg.data2);
	int8_t channel = msg.channel();
	int8_t status = msg.status();
	int8_t data1 = msg.data1;
	int8_t data2 = msg.data2;

	// fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);

	// Filter channels
	if (this->channel == (channel + 1) ) { // Only process the channel we want

		switch (status) {
			// note off
			case 0x8: {
				releaseNote(data1);
			}
				break;
			case 0x9: // note on
				if (data2 > 0) {
					pressNote(data1);
					this->vel = data2;
				} else {
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					releaseNote(data1);
				}
				break;
			case 0xb: // cc
				if (MPEPlus) { // Processing MPE+ data
					// Note from the Haken Continuum Manual:
					// (To avoid the glitches, the synthesizer can do synchronous 14-bit updates with output from the Continuum: 
					// simply save the least significant data, and do not apply it until the most significant data is received.)
					switch (data1) {
						case 74: // Y axis
							MPEPlusyAxis.MSB = data2;
							MPEPlusyAxis.changed = true;
							break;
						case 106:
							MPEPlusyAxis.LSB = data2;
							MPEPlusyAxis.changed = true;
							break;
						case 70: // Z or Pressure
							MPEPluszAxis.MSB = data2;
							MPEPluszAxis.changed = true;
							break;
						case 102:
							MPEPluszAxis.LSB = data2;
							MPEPluszAxis.changed = true;
							break;
					}
				} else { // Non MPE+ data
					switch (data1) {
						case 0x01: // mod
							mod.val = data2;
							mod.changed = true;
							// std::cout << "mod" << std::endl;
							break;
						case 0x4a: // CC 74 <- we should probably make this assignable if neeed.
							Yaxis.val = data2;
							Yaxis.changed = true;
							break ;
					}
				} // End MPE or MPE+ switch
				if (data1== 0x40 ) {
					pedal = (data2 >= 64);
					if (!pedal) {
						releaseNote(-1);
					}
				} // sustain
				break;
			case 0xe: // pitch wheel, we combine two 7 bit in two bytes into a 14bit msg
				{
		
				// We want 2 bytes but variable size may change with platform, maybe we should do a more robust way
				uint16_t twoBytes ; // Initialize our final pitchWheel variable.
				// we don't need to shift the first byte because it's 7 bit (always starts with 0)
				twoBytes = ( (uint16_t)msg.data2 << 7) | ( (uint16_t)msg.data1 ) ;
				pitchWheel.val = twoBytes;
				Ddebug("pitchWheel.val=%d",pitchWheel.val);
				pitchWheel.changed = true;
				}
				break;
			case 0xd: // channel aftertouch
				afterTouch.val = data1;
				afterTouch.changed = true;
				break;
		}
	}

	// std::cout <<" midi input is on " << Global channel!"
	if (this->globalChannel == (channel + 1) ) {
		//std::cout <<"Global channel!" << std::endl;
		if (data1 == midiPedalOne.cc) {
			//std::cout <<"Pedal One value is " << data2 << std::endl;
			midiPedalOne.val = data2;
			midiPedalOne.changed = true;
		}
	}
}

struct MPEToCVWidget : ModuleWidget {
	MPEToCVWidget(MPEToCV *module);
	// Reset Notes or not	
	void appendContextMenu(Menu *menu) override {
		MPEToCV *module = dynamic_cast<MPEToCV*>(this->module);

		struct ResetNoteItem : MenuItem {
			MPEToCV *module;
			bool resetNoteBool;
			void onAction(EventAction &e) override {
				//Ddebug("Set resetNoteBool to %d",!resetNoteBool);
				module->noteOffReset = ! resetNoteBool;
			}
		};
		
		ResetNoteItem *item = MenuItem::create<ResetNoteItem>("Reset Note", CHECKMARK(module->noteOffReset == true));
		item->module = module;
		item->resetNoteBool = module->noteOffReset;
		menu->addChild(item);

	}
};



MPEToCVWidget::MPEToCVWidget(MPEToCV *module) : ModuleWidget(module) {
	// MPEToCV *module = new MPEToCV();
	// setModule(module);
	box.size = Vec(15 * 10, 380);
	
	Vec pos = Vec();
	MPEMidiWidget *mpemidiWidget ;
	// bendRangeChoice = LedDisplayChoice;

	// {
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	// }

	float margin = 5;
	float labelHeight = 15;
	float yPos = margin;
	float yGap = 25;
	// float xPos = 0; // Not sure how to initialize it.

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	{
		Label *label = new Label();
		label->box.pos = Vec(box.size.x - margin - 7 * 15, margin);
		label->text = "MPE to CV";
		label->color = nvgRGB(0x00, 0x00, 0x00);
		addChild(label);
		yPos = labelHeight * 2;
	}

	mpemidiWidget = Widget::create<MPEMidiWidget>(mm2px(Vec(3.41891, 14.8373)));
	mpemidiWidget->initialize(module);
	mpemidiWidget->box.size = mm2px(Vec(44, 28));
	// box.size = mm2px(Vec(44, 28));
	mpemidiWidget->midiIO = &module->midiInput;
	addChild(mpemidiWidget);
	pos = mpemidiWidget->box.getBottomLeft();

	std::string labels[MPEToCV::NUM_OUTPUTS] = {"1V/oct", "Gate", "Velocity", "Pressure", "Y axis","Pedal"
														};
	yPos = mpemidiWidget->box.pos.y + mpemidiWidget->box.size.y + 5*margin ;
	
	for (int i = 0; i < MPEToCV::NUM_OUTPUTS; i++) {
		Label *label = new Label();
		label->box.pos = Vec(margin, yPos);
		label->text = labels[i];
		label->color = nvgRGB(0x00, 0x00, 0x00);
		addChild(label);
		addOutput(Port::create<PJ3410Port>(Vec(15 * 6, yPos - 5), Port::OUTPUT, module, i));

		yPos += yGap + 2*margin;
	}

};

RACK_PLUGIN_MODEL_INIT(ErraticInstruments, MPEToCV) {
   Model *modelMPEToCV = Model::create<MPEToCV, MPEToCVWidget>("Erratic Instruments", "MPEToCV", "MPE to CV", MIDI_TAG, EXTERNAL_TAG);
   return modelMPEToCV;
}
