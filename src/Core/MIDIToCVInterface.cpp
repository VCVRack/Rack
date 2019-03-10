
#ifdef USE_VST2
extern void vst2_get_timing_info (int *_retPlaying, float *_retBPM, float *_retSongPosPPQ);
#endif // USE_VST2

#include "global_pre.hpp"
#include "Core.hpp"
#include "midi.hpp"
#include "dsp/digital.hpp"
#include "dsp/filter.hpp"

#include <algorithm>
#include "global.hpp"

#ifdef USE_VST2
#include "vstmidi.hpp"
#endif // USE_VST2


struct MIDIToCVInterface : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		GATE_OUTPUT,
		VELOCITY_OUTPUT,
		AFTERTOUCH_OUTPUT,
		PITCH_OUTPUT,
		MOD_OUTPUT,
		RETRIGGER_OUTPUT,
		CLOCK_1_OUTPUT,
		CLOCK_2_OUTPUT,
		START_OUTPUT,
		STOP_OUTPUT,
		CONTINUE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MidiInputQueue midiInput;

	uint8_t mod = 0;
	ExponentialFilter modFilter;
	uint16_t pitch = 8192;
	ExponentialFilter pitchFilter;
	PulseGenerator retriggerPulse;
	PulseGenerator clockPulses[2];
	PulseGenerator startPulse;
	PulseGenerator stopPulse;
	PulseGenerator continuePulse;
	int clock = 0;
   int last_clock = -1;  // when clock is derived from ppqPos
	int divisions[2];
#ifdef USE_VST2
   int b_vst_transport_playing = 0;
   float vst_timing_clock_samples = 0.0f;
#endif // USE_VST2

	struct NoteData {
		uint8_t velocity = 0;
		uint8_t aftertouch = 0;
	};

	NoteData noteData[128];
	std::vector<uint8_t> heldNotes;
	uint8_t lastNote;
	bool pedal;
	bool gate;

	MIDIToCVInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), heldNotes(128) {
		onReset();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();

		json_t *divisionsJ = json_array();
		for (int i = 0; i < 2; i++) {
			json_t *divisionJ = json_integer(divisions[i]);
			json_array_append_new(divisionsJ, divisionJ);
		}
		json_object_set_new(rootJ, "divisions", divisionsJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *divisionsJ = json_object_get(rootJ, "divisions");
		if (divisionsJ) {
			for (int i = 0; i < 2; i++) {
				json_t *divisionJ = json_array_get(divisionsJ, i);
				if (divisionJ)
					divisions[i] = json_integer_value(divisionJ);
			}
		}

		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}

	void onReset() override {
		heldNotes.clear();
		lastNote = 60;
		pedal = false;
		gate = false;
		clock = 0;
      last_clock = -1;
		divisions[0] = 24;
		divisions[1] = 6;
#ifdef USE_VST2
      b_vst_transport_playing = 0;
      vst_timing_clock_samples = 0.0f;
#endif // USE_VST2
	}

	void pressNote(uint8_t note) {
		// Remove existing similar note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Push note
		heldNotes.push_back(note);
		lastNote = note;
		gate = true;
		retriggerPulse.trigger(1e-3f);
	}

	void releaseNote(uint8_t note) {
		// Remove the note
		auto it = std::find(heldNotes.begin(), heldNotes.end(), note);
		if (it != heldNotes.end())
			heldNotes.erase(it);
		// Hold note if pedal is pressed
		if (pedal)
			return;
		// Set last note
		if (!heldNotes.empty()) {
			lastNote = heldNotes[heldNotes.size() - 1];
			gate = true;
		}
		else {
			gate = false;
		}
	}

	void pressPedal() {
		pedal = true;
	}

	void releasePedal() {
		pedal = false;
		releaseNote(255);
	}

#ifdef USE_VST2
   void handleVSTClock(void) {
      // (note) calling this _per sample_ is quite excessive (but should not be a problem)
      // (note) the VST host might not update this per sample, though
      float songPosPPQ = -1.0f;
      float bpm = -1.0f;
      int bPlaying = 0;
      vst2_get_timing_info(&bPlaying, &bpm, &songPosPPQ);
      // printf("xxx songPosPPQ=%f bpm=%f bPlaying=%d\n", songPosPPQ, bpm, bPlaying);

      if(b_vst_transport_playing ^ bPlaying)
      {
         if(bPlaying)
         {
            startPulse.trigger(1e-3f);
            clock = 0;
            last_clock = -1;
            vst_timing_clock_samples = 0.0f;
         }
         else
         {
            stopPulse.trigger(1e-3f);
            // Reset timing
            clock = 0;
            last_clock = -1;
         }
         b_vst_transport_playing = bPlaying;
      }

      if(bPlaying)
      {
         // 24 clock ticks per quarter note (MIDI timing clock)
         if(-1.0f != songPosPPQ)
         {
            clock = int(songPosPPQ * 24.0f);

            if(clock != last_clock)
            {
               last_clock = clock;

               if (clock % divisions[0] == 0) {
                  clockPulses[0].trigger(1e-3f);
               }
               if (clock % divisions[1] == 0) {
                  clockPulses[1].trigger(1e-3f);
               }
            }
         }
         else if(bpm > 0.0f)
         {
            float secondsPerQuarter = 60.0f / bpm;
            float samplesPerTimingClockTick = (engineGetSampleRate() * secondsPerQuarter) / 24.0f;

            if (clock % divisions[0] == 0) {
               clockPulses[0].trigger(1e-3);
            }
            if (clock % divisions[1] == 0) {
               clockPulses[1].trigger(1e-3);
            }

            vst_timing_clock_samples += 1.0f;
            if(vst_timing_clock_samples >= samplesPerTimingClockTick)
            {
               vst_timing_clock_samples -= samplesPerTimingClockTick;

               // if(++clock < 0) clock = 0  (may be optimized away by a C compiler)
               union {
                  int s;
                  unsigned int u;
               } uclock;
               uclock.s = clock;
               uclock.u++;
               clock = (uclock.s < 0) ? 0 : uclock.s;
            }
         }
      } // if bPlaying
   }
#endif // USE_VST2

	void step() override {
		MidiMessage msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}
		float deltaTime = engineGetSampleTime();

#ifdef USE_VST2
      handleVSTClock();
#endif // USE_VST2

		outputs[CV_OUTPUT].value = (lastNote - 60) / 12.f;
		outputs[GATE_OUTPUT].value = gate ? 10.f : 0.f;
		outputs[VELOCITY_OUTPUT].value = rescale(noteData[lastNote].velocity, 0, 127, 0.f, 10.f);

		outputs[AFTERTOUCH_OUTPUT].value = rescale(noteData[lastNote].aftertouch, 0, 127, 0.f, 10.f);
		pitchFilter.lambda = 100.f * deltaTime;
		outputs[PITCH_OUTPUT].value = pitchFilter.process(rescale(pitch, 0, 16384, -5.f, 5.f));
		modFilter.lambda = 100.f * deltaTime;
		outputs[MOD_OUTPUT].value = modFilter.process(rescale(mod, 0, 127, 0.f, 10.f));

		outputs[RETRIGGER_OUTPUT].value = retriggerPulse.process(deltaTime) ? 10.f : 0.f;
		outputs[CLOCK_1_OUTPUT].value = clockPulses[0].process(deltaTime) ? 10.f : 0.f;
		outputs[CLOCK_2_OUTPUT].value = clockPulses[1].process(deltaTime) ? 10.f : 0.f;

		outputs[START_OUTPUT].value = startPulse.process(deltaTime) ? 10.f : 0.f;
		outputs[STOP_OUTPUT].value = stopPulse.process(deltaTime) ? 10.f : 0.f;
		outputs[CONTINUE_OUTPUT].value = continuePulse.process(deltaTime) ? 10.f : 0.f;
	}

	void processMessage(MidiMessage msg) {
		// debug("MIDI: %01x %01x %02x %02x", msg.status(), msg.channel(), msg.note(), msg.value());

		switch (msg.status()) {
			// note off
			case 0x8: {
				releaseNote(msg.note());
			} break;
			// note on
			case 0x9: {
				if (msg.value() > 0) {
					noteData[msg.note()].velocity = msg.getData2();
					pressNote(msg.note());
				}
				else {
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					releaseNote(msg.note());
				}
			} break;
			// polyphonic aftertouch
			case 0xa: {
				uint8_t note = msg.note();
				noteData[note].aftertouch = msg.getData2();
			} break;
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			// channel aftertouch
			case 0xd: {
				for(uint8_t noteIdx = 0u; noteIdx < 128u; noteIdx++)
               noteData[noteIdx].aftertouch = msg.getData1();
			} break;
			// pitch wheel
			case 0xe: {
				pitch = msg.value() * 128 + msg.note();
			} break;
			case 0xf: {
				processSystem(msg);
			} break;
			default: break;
		}
	}

	void processCC(MidiMessage msg) {
		switch (msg.note()) {
			// mod
			case 0x01: {
				mod = msg.value();
			} break;
			// sustain
			case 0x40: {
				if (msg.value() >= 64)
					pressPedal();
				else
					releasePedal();
			} break;
			default: break;
		}
	}

	void processSystem(MidiMessage msg) {
#ifndef USE_VST2
		switch (msg.channel()) {
			// Timing
			case 0x8: {
				if (clock % divisions[0] == 0) {
					clockPulses[0].trigger(1e-3);
				}
				if (clock % divisions[1] == 0) {
					clockPulses[1].trigger(1e-3);
				}
				if (++clock >= (24*16*16)) {
					// Avoid overflowing the integer
					clock = 0;
				}
			} break;
			// Start
			case 0xa: {
				startPulse.trigger(1e-3);
				clock = 0;
            last_clock = -1;
			} break;
			// Continue
			case 0xb: {
				continuePulse.trigger(1e-3);
			} break;
			// Stop
			case 0xc: {
				stopPulse.trigger(1e-3);
				// Reset timing
				clock = 0;
            last_clock = -1;
			} break;
			default: break;
		}
#else
      (void)msg;
#endif // USE_VST2
	}
};


struct MIDIToCVInterfaceWidget : ModuleWidget {
	MIDIToCVInterfaceWidget(MIDIToCVInterface *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetGlobal("res/Core/MIDIToCVInterface.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.61505f, 60.1445f)), Port::OUTPUT, module, MIDIToCVInterface::CV_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(16.214f, 60.1445f)), Port::OUTPUT, module, MIDIToCVInterface::GATE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.8143f, 60.1445f)), Port::OUTPUT, module, MIDIToCVInterface::VELOCITY_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.61505f, 76.1449f)), Port::OUTPUT, module, MIDIToCVInterface::AFTERTOUCH_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(16.214f, 76.1449f)), Port::OUTPUT, module, MIDIToCVInterface::PITCH_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.8143f, 76.1449f)), Port::OUTPUT, module, MIDIToCVInterface::MOD_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.61505f, 92.1439f)), Port::OUTPUT, module, MIDIToCVInterface::RETRIGGER_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(16.214f, 92.1439f)), Port::OUTPUT, module, MIDIToCVInterface::CLOCK_1_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.8143f, 92.1439f)), Port::OUTPUT, module, MIDIToCVInterface::CLOCK_2_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(4.61505f, 108.144f)), Port::OUTPUT, module, MIDIToCVInterface::START_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(16.214f, 108.144f)), Port::OUTPUT, module, MIDIToCVInterface::STOP_OUTPUT));
		addOutput(Port::create<PJ301MPort>(mm2px(Vec(27.8143f, 108.144f)), Port::OUTPUT, module, MIDIToCVInterface::CONTINUE_OUTPUT));

		MidiWidget *midiWidget = Widget::create<MidiWidget>(mm2px(Vec(3.41891f, 14.8373f)));
		midiWidget->box.size = mm2px(Vec(33.840f, 28.f));
		midiWidget->midiIO = &module->midiInput;

#ifdef USE_VST2
      midiWidget->midiIO->setDriverId(VST_DRIVER);
      midiWidget->midiIO->setDeviceId(0);
#endif // USE_VST2

		addChild(midiWidget);
	}

	void appendContextMenu(Menu *menu) override {
		MIDIToCVInterface *module = dynamic_cast<MIDIToCVInterface*>(this->module);

		struct ClockDivisionItem : MenuItem {
			MIDIToCVInterface *module;
			int index;
			int division;
			void onAction(EventAction &e) override {
				module->divisions[index] = division;
			}
		};

		struct ClockItem : MenuItem {
			MIDIToCVInterface *module;
			int index;
			Menu *createChildMenu() override {
				Menu *menu = new Menu();
				std::vector<int> divisions = {24*4, 24*2, 24, 24/2, 24/4, 24/8, 2, 1};
				std::vector<std::string> divisionNames = {"Whole", "Half", "Quarter", "8th", "16th", "32nd", "12 PPQN", "24 PPQN"};
				for (size_t i = 0; i < divisions.size(); i++) {
					ClockDivisionItem *item = MenuItem::create<ClockDivisionItem>(divisionNames[i], CHECKMARK(module->divisions[index] == divisions[i]));
					item->module = module;
					item->index = index;
					item->division = divisions[i];
					menu->addChild(item);
				}
				return menu;
			}
		};

		menu->addChild(construct<MenuLabel>());
		for (int i = 0; i < 2; i++) {
			ClockItem *item = MenuItem::create<ClockItem>(stringf("CLK %d rate", i + 1));
			item->module = module;
			item->index = i;
			menu->addChild(item);
		}
	}
};


RACK_PLUGIN_MODEL_INIT(Core, MIDIToCVInterface) {
   Model *modelMIDIToCVInterface = Model::create<MIDIToCVInterface, MIDIToCVInterfaceWidget>("Core", "MIDIToCVInterface", "MIDI-1", MIDI_TAG, EXTERNAL_TAG);
   return modelMIDIToCVInterface;
}
