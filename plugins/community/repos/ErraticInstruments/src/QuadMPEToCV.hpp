#include "global_pre.hpp"
#include "Erratic.hpp"
#include "midi.hpp"
#include "dsp/digital.hpp"
#include "MPEBaseWidget.hpp"
#include "global_ui.hpp"

struct MidiValue {
	int val = 0; // Controller value
	// TransitionSmoother tSmooth;
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

struct MidiNote {
	int pitch = 60;
	int vel = 0; // velocity
	bool gate = false;
	int channel ;
	bool noteOn, noteOff ;
	bool changed = false;
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

struct MPEChannel { // This contains the required info for each channel, each note in practical terms
	int MIDIChannel; // must initialize
	MidiNote note;
	MidiValue mod;
	MidiValue afterTouch;
	MidiValue pitchWheel;
	MidiValue Yaxis ;
	MPEPlusValue MPEPlusyAxis, MPEPluszAxis;
	bool changed = false;
};

struct QuadMPEToCV : Module {
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
		GATE_OUTPUT = 4,
		VELOCITY_OUTPUT = 8,
		PRESSURE_OUTPUT = 12,
		Y_OUTPUT = 16,
		PEDAL_OUTPUT = 20,
		NUM_OUTPUTS = 21
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

	int polyphony = 4;
	std::vector<MPEChannel> mpeChannels;
	// MPEChannel mpeChannels[4] ; // using this here instead of a container
	
	bool noteOffReset = true; // Our default
    int baseMIDIChannel = 2 ;
	int bendRange = 48; // our default is 48 (common for ROLI), Continuum defaults to 96. This is a global parameter (for now)
	int globalMIDIChannel = 16; // Our default channel is 16. ROLI users will want to set this is 1
	bool MPEPlus = false ; // This is specially useful for Haken Continuum
	int YaxisCC = 74 ;

	bool pedal = false;
	// int note = 60; // C4, most modules should use 261.626 Hz
	int vel = 0;

	MidiPedalValue midiPedalOne ;

	SchmittTrigger resetTrigger;

	QuadMPEToCV() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		// for (int p=0; p < polyphony ; p++) {
		// 	pitchWheel[p].val = 64;
		// 	//pitchWheel[p].tSmooth.set(0, 0);
		// }
		mpeChannels.reserve(polyphony);
		midiPedalOne.cc = 12; // By default we use 12 (barrel i on the Haken Continuum)		
		this->setupMIDIChannels();
	}

	~QuadMPEToCV() {
	};

	void step() override;

	void pressNote(MidiNote note);

	void releaseNote(MidiNote note);

	void processMessage(MidiMessage msg) ;

	void setupMIDIChannels();

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "midi", midiInput.toJson());
		json_object_set_new(rootJ, "bendRange", json_integer(bendRange));
		json_object_set_new(rootJ, "baseMIDIChannel", json_integer(baseMIDIChannel));
		json_object_set_new(rootJ, "globalMidiChannel", json_integer(globalMIDIChannel));
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
		json_t *baseMIDIChannelJ = json_object_get(rootJ, "baseMIDIChannel");
		if (baseMIDIChannelJ) {
			baseMIDIChannel = json_integer_value(baseMIDIChannelJ);
		}
		json_t *globalMidiChannelJ = json_object_get(rootJ, "globalMidiChannel");
		if (globalMidiChannelJ) {
			globalMIDIChannel = json_integer_value(globalMidiChannelJ);
		}
		json_t *MPEModeJ = json_object_get(rootJ, "MPEMode");
		if (MPEModeJ) {
			MPEPlus = json_integer_value(MPEModeJ);
		}
		json_t *noteOffResetJ = json_object_get(rootJ, "noteOffReset");
		if (noteOffResetJ) {
			noteOffReset = json_integer_value(noteOffResetJ);
		}
	}


	// void fromJson(json_t *rootJ) override {
	// 	json_t *bendRangeJ = json_object_get(rootJ, "bendRange");
	// 	if (bendRangeJ)
	// 		bendRange = json_integer_value(bendRangeJ);
		
	// 	json_t *baseMIDIChannelJ = json_object_get(rootJ, "baseMIDIChannel");
	// 	if (baseMIDIChannelJ)
	// 		baseMIDIChannel = json_integer_value(baseMIDIChannelJ);
		
	// 	json_t *globalMIDIChannelJ = json_object_get(rootJ, "globalMIDIChannel");
	// 	if (globalMIDIChannelJ)
	// 		globalMIDIChannel = json_integer_value(globalMIDIChannelJ);

	// 	json_t *noteOffResetJ = json_object_get(rootJ, "noteOffReset");
	// 	if (noteOffResetJ)
	// 		noteOffReset = json_boolean_value(noteOffResetJ);

	// 	json_t *MPEPlusModeJ = json_object_get(rootJ, "MPEPlusMode");
	// 	if (MPEPlusModeJ)
	// 		MPEPlus = json_boolean_value(MPEPlusModeJ);

	// 	this->setupMIDIChannels();
	// }

	// json_t *toJson() override {
	// 	json_t *rootJ = json_object();
	// 	// Semitones
	// 	// std::cout<< "We set bendRange to " << bendRange << std::endl;
	// 	json_object_set_new(rootJ, "noteOffReset", json_boolean(noteOffReset));
	// 	json_object_set_new(rootJ, "MPEPlusMode", json_boolean(MPEPlus));
	// 	json_object_set_new(rootJ, "bendRange", json_integer(bendRange));
	// 	json_object_set_new(rootJ, "baseMIDIChannel", json_integer(baseMIDIChannel));
	// 	json_object_set_new(rootJ, "globalMIDIChannel", json_integer(globalMIDIChannel));
	// 	return rootJ;
	// }


	// void reset() override {
	// 	resetMidi();
	// }

	// void resetMidi() override;

};

struct QuadMPEToCVWidget : ModuleWidget {
	QuadMPEToCVWidget(QuadMPEToCV *module);

	// Reset Notes or not	
	void appendContextMenu(Menu *menu) override {
	QuadMPEToCV *module = dynamic_cast<QuadMPEToCV*>(this->module);

		struct ResetNoteItem : MenuItem {
			QuadMPEToCV *module;
			bool resetNoteBool;
			void onAction(EventAction &e) override {
				module->noteOffReset = ! resetNoteBool;
			}
		};

		
		ResetNoteItem *item = MenuItem::create<ResetNoteItem>("Reset Note", CHECKMARK(module->noteOffReset == true));
		item->module = module;
		item->resetNoteBool = module->noteOffReset;
		menu->addChild(item);

	}


};


void QuadMPEToCV::setupMIDIChannels() {
	// std::cout << " Setting up MIDI channels with baseMIDIChannel set to " << baseMIDIChannel << std::endl;
	for (int p=0 ; p < polyphony ; p++) {
		// std::cout << " p MIDIChannel " << p << " " << p + baseMIDIChannel - 1 << std::endl;
		mpeChannels[p].MIDIChannel = p + baseMIDIChannel - 1; // MPE channels start at 2 onwards. We are using MIDI channel starting at 0
	}
}

// void QuadMPEToCV::resetMidi() {
// 	for (int p=0; p < polyphony ; p++) {
// 		mpeChannels[p].mod.val = 0;
// 		mpeChannels[p].pitchWheel.val = 0;
// 		mpeChannels[p].afterTouch.val = 0;
// 	}



// 	// mod.val = 0;
// 	// mod.tSmooth.set(0, 0);
// 	// pitchWheel.val = 64;
// 	// pitchWheel.tSmooth.set(0, 0);
// 	// afterTouch.val = 0;
// 	// afterTouch.tSmooth.set(0, 0);
// 	//vel = 0;
// 	//gate = false;
// 	//notes.clear();
// }

void QuadMPEToCV::step() {
	// if (isPortOpen()) {
	// 	std::vector<unsigned char> message;
	// 	int msgsProcessed = 0;

	// 	// midiIn->getMessage returns empty vector if there are no messages in the queue
	// 	// Original Midi to CV limits processing to 4 midi msgs, we should log how many we do at a time to look for
	// 	// potential issues, specially with MPE+
	// 	getMessage(&message);
	// 	while (msgsProcessed < 4 && message.size() > 0) {
	// 		processMidi(message);
	// 		getMessage(&message);
	// 		msgsProcessed++;
	// 	}
	// }
	MidiMessage msg;
	while (midiInput.shift(&msg)) {
		processMessage(msg);
	}

	// if (resetTrigger.process(params[RESET_PARAM].value)) {
	// 	resetMidi();
	// 	return;
	// }

	// lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / 0.55 / engineGetSampleRate(); // fade out light

	for (int ci=0; ci < polyphony; ci++) {
		if (mpeChannels[ci].changed) {
			if (mpeChannels[ci].note.changed) {
				// std::cout << "New note on ci " << ci << std::endl;
				// std::cout << "gate is " << mpeChannels[ci].note.gate << std::endl;
				outputs[GATE_OUTPUT+ci].value = mpeChannels[ci].note.gate ? 10.0 : 0.0;
				// std::cout << "outputs[GATE_OUTPUT+ci].value is " << outputs[GATE_OUTPUT+ci].value << std::endl;
				outputs[VELOCITY_OUTPUT+ci].value = mpeChannels[ci].note.vel / 127.f * 10.f;
				outputs[PITCH_OUTPUT+ci].value = (((mpeChannels[ci].note.pitch - 60)) / 12.0) + ((mpeChannels[ci].pitchWheel.val - 8192 ) / 8192.0 / 12.0 * (float)bendRange ) ;
				// std::cout << "outputs[VELOCITY_OUTPUT+ci].value is " << outputs[VELOCITY_OUTPUT+ci].value << std::endl;
				
				if (mpeChannels[ci].note.noteOff && noteOffReset) { // We reset all info when the note goes off
					mpeChannels[ci].note.noteOff = false;
				
					if (noteOffReset) {					
						//std::cout << "We execute the note off reset" << std::endl;
						mpeChannels[ci].pitchWheel.val = 0;
						mpeChannels[ci].pitchWheel.changed = false;
						outputs[PITCH_OUTPUT+ci].value = 0 ;
						mpeChannels[ci].afterTouch.val = 0;
						mpeChannels[ci].afterTouch.changed = false;
						outputs[PRESSURE_OUTPUT+ci].value = 0 ;
						mpeChannels[ci].Yaxis.val = 0;
						mpeChannels[ci].Yaxis.changed = false;
						outputs[Y_OUTPUT+ci].value = 0 ;
					}	
				}
				mpeChannels[ci].note.changed = false;
			}
			if (mpeChannels[ci].note.gate) {
				if (mpeChannels[ci].pitchWheel.changed && mpeChannels[ci].note.gate ) {
					// std::cout << "mpeChannels[ci].pitch is " << mpeChannels[ci].note.pitch << std::endl;
					outputs[PITCH_OUTPUT+ci].value = (((mpeChannels[ci].note.pitch - 60)) / 12.0) + ((mpeChannels[ci].pitchWheel.val - 8192 ) / 8192.0 / 12.0 * (float)bendRange ) ;
					mpeChannels[ci].pitchWheel.changed = false;
					// std::cout << "Setting pitch on ci " << ci << " to " << outputs[PITCH_OUTPUT+ci].value << std::endl;
				}
				if (MPEPlus) { // We process MPE+ or not here
					if (mpeChannels[ci].MPEPluszAxis.changed) {
						// Combine two 7 bit into 14bit
						mpeChannels[ci].MPEPluszAxis.val = ( (uint16_t)mpeChannels[ci].MPEPluszAxis.MSB << 7) | ( (uint16_t)mpeChannels[ci].MPEPluszAxis.LSB ) ;
						outputs[PRESSURE_OUTPUT+ci].value = mpeChannels[ci].MPEPluszAxis.val / 16384.0 * 10.f;
						mpeChannels[ci].MPEPluszAxis.changed = false;
						//std::cout << "Setting pressure on ci " << ci << " to " << outputs[PRESSURE_OUTPUT+ci].value << std::endl;
					}
					if (mpeChannels[ci].MPEPlusyAxis.changed) {
						// Combine two 7 bit into 14bit
						mpeChannels[ci].MPEPlusyAxis.val = ( (uint16_t)mpeChannels[ci].MPEPlusyAxis.MSB << 7) | ( (uint16_t)mpeChannels[ci].MPEPlusyAxis.LSB ) ;
						outputs[Y_OUTPUT+ci].value = mpeChannels[ci].MPEPlusyAxis.val / 16384.0 * 10.f;
						// std::cout << "Y axis is " << outputs[Y_OUTPUT].value << std::endl;
						mpeChannels[ci].MPEPlusyAxis.changed = false;
					} 
				} else {
					if (mpeChannels[ci].afterTouch.changed ) {
						outputs[PRESSURE_OUTPUT+ci].value = mpeChannels[ci].afterTouch.val / 127.f * 10.f;
						mpeChannels[ci].afterTouch.changed = false;
						// std::cout << "outputs[PRESSURE_OUTPUT+ci].value is " << outputs[PRESSURE_OUTPUT+ci].value << std::endl;
					}
					if (mpeChannels[ci].Yaxis.changed ) {
						outputs[Y_OUTPUT+ci].value = mpeChannels[ci].Yaxis.val / 127.f * 10.f;
						mpeChannels[ci].Yaxis.changed = false;
						// std::cout << "outputs[Y_OUTPUT+ci].value is " << outputs[Y_OUTPUT+ci].value << std::endl;
					}
				}
			}
		
		mpeChannels[ci].changed = false;	
		}
	}

	// Pedal
	if (midiPedalOne.changed) {
		outputs[PEDAL_OUTPUT].value = midiPedalOne.val / 127.f * 10.f ;
		// std::cout << " We set the output outputs[PEDAL_OUTPUT].value to " << outputs[PEDAL_OUTPUT].value << std::endl;
		midiPedalOne.changed = false;
	}
/*
	outputs[GATE_OUTPUT].value = gate ? 10.0 : 0.0;
	outputs[VELOCITY_OUTPUT].value = vel / 127.0 * 10.0;

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
			std::cout << "Y axis is " << outputs[Y_OUTPUT].value << std::endl;
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
	

	// 1/V incorporates pitch wheel changes
	if (pitchWheel.changed | this->newNote) {
		outputs[PITCH_OUTPUT].value = (((note - 60)) / 12.0) + ((pitchWheel.val - 8192 ) / 8192.0 / 12.0 * (float)bendRange ) ;
		pitchWheel.changed = false;
		this->newNote = false;
	}	

	*/
}

void QuadMPEToCV::processMessage(MidiMessage msg) {
	int8_t channel = msg.channel(); // starts at 0
	int8_t status = msg.status(); //(msg[0] >> 4) & 0xf;
	int8_t data1 = msg.data1;
	int8_t data2 = msg.data2;
	
	if (status == 0xb && ( data1 == 111 || data1 == 118)) {
		return;
	}

	// fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);

	// Filter only the channels of our polyphony, it must be within our boundaries :)
	// std::cout << "MIDI channel and mpeChannels[0].MIDIChannel " << channel << " " << mpeChannels[0].MIDIChannel << std::endl;

	// std::cout << "polyphony is " << polyphony << std::endl;
	// std::cout << "mpeChannels[0].MIDIChannel is " << mpeChannels[0].MIDIChannel << " and mpeChannels[polyphony].MIDIChannel " 
	// << mpeChannels[polyphony-1].MIDIChannel << std::endl;
	// for (int p=0; p < polyphony ; p++ ) {
	// 	std::cout << " mpeChannels[" << p << "].MIDIChannel: " << mpeChannels[p].MIDIChannel << std::endl;
	// }
	if ( channel >= mpeChannels[0].MIDIChannel && channel <= mpeChannels[polyphony-1].MIDIChannel   ) { // Only process the channel we want
		// std::cout << "We process" << std::endl;
		// std::cout << "channel is " << channel << " baseMIDIChannel is " << baseMIDIChannel << " ci is " << ci << std::endl;
		// start	ci	channel
		// 1		0	0
		// 1		1	1
		// 2		0	1
		// 2		1	2
		// 3		0	2
		// 3		1	3

		int ci = channel - baseMIDIChannel + 1;
		
		switch (status) {
			// note off
			case 0x8: {
				// std::cout << "Note off" << std::endl;
				mpeChannels[ci].note.gate= false;
				mpeChannels[ci].note.vel = data2;
				mpeChannels[ci].note.gate = false;
				mpeChannels[ci].note.noteOff = true;
				mpeChannels[ci].note.changed = true;
				mpeChannels[ci].changed = true;
			}
				break;
			case 0x9: // note on
				// std::cout << "Note on" << std::endl;
				// fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);
				// std::cout << "ci is " << ci << std::endl;
				// for (int p=0 ; p < polyphony ; p++) {
				// 	std::cout << " p MIDIChannel " << p << " " << mpeChannels[p].MIDIChannel << std::endl;
				// 	// mpeChannels[p].MIDIChannel = p + baseMIDIChannel - 1; // MPE channels start at 2 onwards. We are using MIDI channel starting at 0
				// }
				if (data2 > 0) { // note ON
					mpeChannels[ci].note.gate= true;
					mpeChannels[ci].note.pitch = data1;
					mpeChannels[ci].note.vel = data2;
					mpeChannels[ci].note.changed = true;
					mpeChannels[ci].changed = true;
				} else { // note off
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					mpeChannels[ci].note.gate= false;
					mpeChannels[ci].note.vel = 0;
					mpeChannels[ci].note.noteOff = true;
					mpeChannels[ci].note.changed = true;
					mpeChannels[ci].changed = true;
				}
				break;
			case 0xe: // pitch wheel, we combine two 7 bit in two bytes into a 14bit msg
				{
				
				// We want 2 bytes but variable size may change with platform, maybe we should do a more robust way
				uint16_t twoBytes ; // Initialize our final pitchWheel variable.
				// we don't need to shift the first byte because it's 7 bit (always starts with 0)
				twoBytes = ( (uint16_t)msg.data2 << 7) | ( (uint16_t)msg.data1 ) ;
				// std::cout << "Pitch wheel " << twoBytes << " on channel and -1 : " << channel << " " << channel -1 << std::endl;
				mpeChannels[ci].pitchWheel.val = twoBytes;
				mpeChannels[ci].changed = true;
				mpeChannels[ci].pitchWheel.changed = true;
				}
				break;
		}
		if (MPEPlus) { // Processing MPE+ data
			// Note from the Haken Continuum Manual:
			// (To avoid the glitches, the synthesizer can do synchronous 14-bit updates with output from the Continuum: 
			// simply save the least significant data, and do not apply it until the most significant data is received.)
			switch (data1) {
				case 74: // Y axis
					mpeChannels[ci].MPEPlusyAxis.MSB = data2;
					mpeChannels[ci].MPEPlusyAxis.changed = true;
					mpeChannels[ci].changed = true;
					break;
				case 106:
					mpeChannels[ci].MPEPlusyAxis.LSB = data2;
					mpeChannels[ci].MPEPlusyAxis.changed = true;
					mpeChannels[ci].changed = true;
					break;
				case 70: // Z or Pressure
					mpeChannels[ci].MPEPluszAxis.MSB = data2;
					mpeChannels[ci].MPEPluszAxis.changed = true;
					mpeChannels[ci].changed = true;
					break;
				case 102:
					mpeChannels[ci].MPEPluszAxis.LSB = data2;
					mpeChannels[ci].MPEPluszAxis.changed = true;
					mpeChannels[ci].changed = true;
					break;
			}
		} else { // Non MPE+ data
			if (status == 0xd) { // Channel Pressure
				// std::cout << " We parse channel aftertouch data that is " << data1 << std::endl;
				mpeChannels[ci].afterTouch.val = data1;
				mpeChannels[ci].afterTouch.changed = true;
				mpeChannels[ci].changed = true;
			}
			if (status == 0xb && data1 == 0x4a) { // CC (oxb) #74 <- we should probably make this assignable if neeed.
				// std::cout << " We parse CC 74 data that is " << data1 << std::endl;
				mpeChannels[ci].Yaxis.val = data2;
				mpeChannels[ci].Yaxis.changed = true;
				mpeChannels[ci].changed = true;
			}
		} // End MPE or MPE+ switch
	} // End note processing

	if (this->globalMIDIChannel == (channel + 1) ) { // If we're on global midi channel
		// std::cout <<"Global channel!" << std::endl;
		if (data1 == midiPedalOne.cc) {
			// std::cout <<"Pedal One value is " << data2 << std::endl;
			midiPedalOne.val = data2;
			midiPedalOne.changed = true;
		}
	}

	/*
		switch (status) {
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
				int nBytes;
				// double stamp;
				nBytes = msg.size();
				// for ( i=0; i<nBytes; i++ )
				//     std::cout << "Byte " << i << " = " << (int)msg[i] << ", ";
				// if ( nBytes > 0 )
				//     std::cout << "stamp = " << stamp << std::endl;
		
				// We want 2 bytes but variable size may change with platform, maybe we should do a more robust way
				uint16_t twoBytes ; // Initialize our final pitchWheel variable.
				// we don't need to shift the first byte because it's 7 bit (always starts with 0)
				twoBytes = ( (uint16_t)msg[2] << 7) | ( (uint16_t)msg[1] ) ;
				// std::cout << sizeof(int) << std::endl;
				// std::bitset<8> msgOne(msg[1]);
				// std::bitset<8> msgTwo(msg[2]);
				// std::bitset<16> x(twoBytes);
				//std::cout << "msg[1] and 2 are " << msgOne << " " << msgTwo << " and shifted is " << x << std::endl;
				//std::cout << "twoBytes is " << (int)twoBytes << std::endl;
				pitchWheel.val = twoBytes;
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
	if (this->globalMIDIChannel == (channel + 1) ) {
		std::cout <<"Global channel!" << std::endl;
		if (data1 == midiPedalOne.cc) {
			std::cout <<"Pedal One value is " << data2 << std::endl;
			midiPedalOne.val = data2;
			midiPedalOne.changed = true;
		}
	}
*/
}


// MPEMidiWidget stuff

struct QuadBendRangeItem : MenuItem {
	QuadMPEToCV *quadmpetocv;
	int bendRange ;
	void onAction(EventAction &e) override {
		// debug("We trigger action with %d", bendRange);
		quadmpetocv->bendRange = bendRange;
	}
};

struct QuadBendRangeChoice : LedDisplayChoice {
	// QuadMPEToCVWidget *quadmpetocvwidget;
	QuadMPEToCV *quadmpetocv;
	
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
				QuadBendRangeItem *item = new QuadBendRangeItem();
				item->quadmpetocv = quadmpetocv;
				item->text = std::to_string(bendRangeValue);
				item->bendRange = bendRangeValue;
				menu->addChild(item);
			}
		// quadmpetocv->bendRange = bendRange;
	}
	void step() override {
		color = nvgRGB(0xff, 0x00, 0x00);
		color.a = 0.8f;
		text = stringf("%d", quadmpetocv->bendRange);
	}
};

struct QuadMidiChannelItem : MenuItem {
	QuadMPEToCV *quadmpetocv;
	int channel ;
	void onAction(EventAction &e) override {
		quadmpetocv->baseMIDIChannel = channel;
	}
};

struct QuadMidiChannelChoice : LedDisplayChoice {
	// QuadMPEToCVWidget *quadmpetocvwidget;
	QuadMPEToCV *quadmpetocv;
	
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
				QuadMidiChannelItem *item = new QuadMidiChannelItem();
				item->quadmpetocv = quadmpetocv;
				item->text = std::to_string(c);
				item->channel = c;
				menu->addChild(item);
			}
	}
	void step() override {
		color = nvgRGB(0xff, 0x00, 0x00);
		color.a = 0.8f;
		text = std::to_string(quadmpetocv->baseMIDIChannel);
	}
};

struct QuadGlobalMidiChannelItem : MenuItem {
	QuadMPEToCV *quadmpetocv;
	int channel ;
	void onAction(EventAction &e) override {
		quadmpetocv->globalMIDIChannel = channel;
	}
};

struct QuadGlobalMidiChannelChoice : LedDisplayChoice {
	// QuadMPEToCVWidget *quadmpetocvwidget;
	QuadMPEToCV *quadmpetocv;
	
	int channel ;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
         Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else
			Menu *menu = gScene->createMenu();
#endif // USE_VST2
			menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Global Midi channel"));
			for (int c=1; c <= 16 ; c++) {
				QuadGlobalMidiChannelItem *item = new QuadGlobalMidiChannelItem();
				item->quadmpetocv = quadmpetocv;
				item->text = std::to_string(c);
				item->channel = c;
				menu->addChild(item);
			}
	}
	void step() override {
		color = nvgRGB(0xff, 0x00, 0x00);
		color.a = 0.8f;
		text = std::to_string(quadmpetocv->globalMIDIChannel);
	}
};

struct QuadMPEModeItem : MenuItem {
	QuadMPEToCV *quadmpetocv;
	bool MPEPlus ;
	void onAction(EventAction &e) override {
		quadmpetocv->MPEPlus = MPEPlus;
	}
};

struct QuadMPEModeChoice : LedDisplayChoice {
	// QuadMPEToCVWidget *quadmpetocvwidget;
	QuadMPEToCV *quadmpetocv;
	
	bool MPEPlus ;
	void onAction(EventAction &e) override {
#ifdef USE_VST2
         Menu *menu = rack::global_ui->ui.gScene->createMenu();
#else
			Menu *menu = gScene->createMenu();
#endif // USE_VST2
			menu->addChild(construct<MenuLabel>(&MenuLabel::text, "MPE mode"));			
			// MPE
			QuadMPEModeItem *MPE = new QuadMPEModeItem();
			MPE->quadmpetocv = quadmpetocv;
			MPE->text = "MPE - Standard (ROLI, etc)";
			MPE->MPEPlus = false;
			menu->addChild(MPE);
			// MPE Plus
			QuadMPEModeItem *MPEPlus = new QuadMPEModeItem();
			MPEPlus->quadmpetocv = quadmpetocv;
			MPEPlus->text = "MPE+ - High Res for Haken Continuum";
			MPEPlus->MPEPlus = true;
			menu->addChild(MPEPlus);

	}
	void step() override {
		// color = nvgRGB(0xff, 0x00, 0x00);
		// color.a = 0.8f;
		if (quadmpetocv->MPEPlus) {
			text = "MPE+";
		} else {
			text = "MPE";
		}		
	}
};

// We extend the midi to follow similar design
struct QuadMPEMidiWidget : MPEBaseWidget {
	LedDisplaySeparator *hSeparators[2];
	LedDisplaySeparator *vSeparators[3];
	// LedDisplayChoice *ccChoices[4][4];
	QuadMPEToCV *quadmpetocv ;
	QuadBendRangeChoice *bendRangeChoice ;
	QuadMidiChannelChoice *midiChannelChoice ;
	QuadGlobalMidiChannelChoice *globalMidiChannelChoice ;
	QuadMPEModeChoice *mpeModeChoice ;

	QuadMPEMidiWidget() {
	}

	void initialize(QuadMPEToCV *quadmpetocv) {
		this->quadmpetocv = quadmpetocv;
		Vec pos = deviceChoice->box.getBottomLeft();
		for (int y = 0; y < 2; y++) {
			hSeparators[y] = Widget::create<LedDisplaySeparator>(pos);
			addChild(hSeparators[y]);
		}

		midiChannelChoice = Widget::create<QuadMidiChannelChoice>(pos);
		midiChannelChoice->quadmpetocv = quadmpetocv ;
		addChild(midiChannelChoice);

		globalMidiChannelChoice = Widget::create<QuadGlobalMidiChannelChoice>(pos);
		globalMidiChannelChoice->quadmpetocv = quadmpetocv ;
		addChild(globalMidiChannelChoice);

		bendRangeChoice = Widget::create<QuadBendRangeChoice>(pos);
		bendRangeChoice->quadmpetocv = quadmpetocv ;
		addChild(bendRangeChoice);

		mpeModeChoice = Widget::create<QuadMPEModeChoice>(pos);
		mpeModeChoice->quadmpetocv = quadmpetocv ;
		addChild(mpeModeChoice);


		for (int x = 1; x < 3; x++) {
			vSeparators[x] = Widget::create<LedDisplaySeparator>(pos);
			addChild(vSeparators[x]);
		}

		for (int x = 1; x < 3; x++) {
			vSeparators[x]->box.size.y = midiChannelChoice->box.size.y;

		}
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
		
		for (int x = 1; x < 3; x++) {
			vSeparators[x]->box.pos.x = box.size.x / 4 * x;
		}
		
	}
};
