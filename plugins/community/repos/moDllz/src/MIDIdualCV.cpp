#include "dsp/digital.hpp"
#include "moDllz.hpp"
#include "midi.hpp"
#include "dsp/filter.hpp"

/*
 * MIDIdualCV converts upper/lower midi note on/off events, velocity , channel aftertouch, pitch wheel,  mod wheel breath cc and expression to CV
 */

namespace rack_plugin_moDllz {

struct MidiNoteData {
	uint8_t velocity = 0;
	uint8_t aftertouch = 0;
};

struct MIDIdualCV :  Module {
	enum ParamIds {
		RESETMIDI_PARAM,
		LWRRETRGGMODE_PARAM,
		UPRRETRGGMODE_PARAM,
		SUSTAINHOLD_PARAM,
		PBPOS_UPPER_PARAM,
		PBNEG_UPPER_PARAM,
		PBPOS_LOWER_PARAM,
		PBNEG_LOWER_PARAM,
		SLEW_LOWER_PARAM,
		SLEW_UPPER_PARAM,
		SLEW_LOWER_MODE_PARAM,
		SLEW_UPPER_MODE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT_Lwr,
		PITCH_OUTPUT_Upr,
		VELOCITY_OUTPUT_Lwr,
		VELOCITY_OUTPUT_Upr,
		RETRIGGATE_OUTPUT_Lwr,
		RETRIGGATE_OUTPUT_Upr,
		GATE_OUTPUT,
		PBEND_OUTPUT,
		MOD_OUTPUT,
		EXPRESSION_OUTPUT,
		BREATH_OUTPUT,
		SUSTAIN_OUTPUT,
		PRESSURE_OUTPUT,
		PBENDPOS_OUTPUT,
		PBENDNEG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RESETMIDI_LIGHT,
		NUM_LIGHTS
	};
 
	MidiInputQueue midiInput;
	
	uint8_t mod = 0;
	ExponentialFilter modFilter;
	uint8_t breath = 0;
	ExponentialFilter breathFilter;
	uint8_t expression = 0;
	ExponentialFilter exprFilter;
	uint16_t pitch = 8192;
	ExponentialFilter pitchFilter;
	uint8_t sustain = 0;
	ExponentialFilter sustainFilter;
	uint8_t pressure = 0;
	ExponentialFilter pressureFilter;

	MidiNoteData noteData[128];
	
	SlewLimiter slewlimiterLwr;
	SlewLimiter slewlimiterUpr;
	
	float slewLwr = 0.f;
	float slewUpr = 0.f;
	
	struct noteLive{
		int note = 0;
		uint8_t vel = 0;
		float volt = 0.f;
	};
	noteLive lowerNote;
	noteLive upperNote;
	bool anynoteGate = false;
	bool sustpedal = false;
	bool firstNoGlideLwr = false;
	bool firstNoGlideUpr = false;
	uint8_t lastLwr = 128;
	uint8_t lastUpr = -1;
	
	PulseGenerator gatePulseLwr;
	PulseGenerator gatePulseUpr;
	SchmittTrigger resetMidiTrigger;
	
	
	MIDIdualCV() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
   
	bool noteupdated = false;
	
	
	///////////////////         ////           ////          ////         /////////////////////
	/////////////////   ///////////////  /////////  ////////////  //////  ////////////////////
	/////////////////         ////////  /////////       ///////         /////////////////////
	///////////////////////   ///////  /////////  ////////////  ////////////////////////////
	//////////////          ////////  /////////         /////  ////////////////////////////
	
	void step() override {
		MidiMessage msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}
		
		pitchFilter.lambda = 100.f * engineGetSampleTime();
		float pitchwheel;
		float pitchtocvLWR = 0.f;
		float pitchtocvUPR = 0.f;
		if (pitch < 8192){
			pitchwheel = pitchFilter.process(rescale(pitch, 0, 8192, -5.f, 0.f));
			outputs[PBENDNEG_OUTPUT].value = pitchwheel * 2.f;
			outputs[PBENDPOS_OUTPUT].value = 0.f;
			pitchtocvLWR = pitchwheel * params[PBNEG_LOWER_PARAM].value / 60.f;
			pitchtocvUPR = pitchwheel * params[PBNEG_UPPER_PARAM].value / 60.f;
		} else {
			pitchwheel = pitchFilter.process(rescale(pitch, 8192, 16383, 0.f, 5.f));
			outputs[PBENDPOS_OUTPUT].value = pitchwheel * 2.f;
			outputs[PBENDNEG_OUTPUT].value = 0.f;
			pitchtocvLWR = pitchwheel * params[PBPOS_LOWER_PARAM].value / 60.f;
			pitchtocvUPR = pitchwheel * params[PBPOS_UPPER_PARAM].value / 60.f;
		}
		outputs[PBEND_OUTPUT].value = pitchwheel;
		
		///////////////////////
		if (noteupdated){
			anynoteGate = false;
			noteupdated = false;
			///LOWER///
			for (int i = 0; i < 128; i++){
				if (noteData[i].velocity > 0){
					anynoteGate = true;
					/////////
					if (i != lastLwr){
						lastLwr = i;
						if (params[LWRRETRGGMODE_PARAM].value > 0.5f)
							gatePulseLwr.trigger(1e-3);
						else if (i < lowerNote.note)
							gatePulseLwr.trigger(1e-3);
					}
					lowerNote.note = i;
					lowerNote.vel = noteData[i].velocity;
					break;
				}
			}
			if (anynoteGate){
					///UPPER///
					for (int i = 127; i > -1; i--){
						if (noteData[i].velocity > 0){
							if (i != lastUpr){
								lastUpr = i;
								if (params[UPRRETRGGMODE_PARAM].value > 0.5f)
									gatePulseUpr.trigger(1e-3);
								else if (i > upperNote.note)
									gatePulseUpr.trigger(1e-3);
							}
							upperNote.note = i;
							upperNote.vel = noteData[i].velocity;
							break;
						}
					}
					lowerNote.volt = static_cast<float>(lowerNote.note - 60) / 12.0f;
					upperNote.volt =static_cast<float>(upperNote.note - 60) / 12.0f;
					outputs[VELOCITY_OUTPUT_Lwr].value = static_cast<float>(lowerNote.vel) / 127.0f * 10.0f;
					outputs[VELOCITY_OUTPUT_Upr].value = static_cast<float>(upperNote.vel) / 127.0 * 10.0;
			}else{
				lowerNote.note = 128;
				upperNote.note = -1;
			}
		}
		if (slewLwr != params[SLEW_LOWER_PARAM].value) {
			slewLwr = params[SLEW_LOWER_PARAM].value;
			float slewfloat = 1.0f/(5.0f + slewLwr * engineGetSampleRate());
			slewlimiterLwr.setRiseFall(slewfloat,slewfloat);
		}

		if (slewLwr > 0.f)
			if (firstNoGlideLwr){
				slewlimiterLwr.setRiseFall(1.f,1.f);
				outputs[PITCH_OUTPUT_Lwr].value = slewlimiterLwr.process(lowerNote.volt) + pitchtocvLWR;
				slewLwr = 0.f; // value to retrigger calc next note
			}else{
				outputs[PITCH_OUTPUT_Lwr].value = slewlimiterLwr.process(lowerNote.volt) + pitchtocvLWR;
			}
		
		else
			 outputs[PITCH_OUTPUT_Lwr].value = lowerNote.volt + pitchtocvLWR;
		
		
		if (slewUpr != params[SLEW_UPPER_PARAM].value) {
			slewUpr = params[SLEW_UPPER_PARAM].value;
			float slewfloat = 1.0f/(5.0f + slewUpr * engineGetSampleRate());
			slewlimiterUpr.setRiseFall(slewfloat,slewfloat);
		}
		if (slewUpr > 0.f)
			if (firstNoGlideUpr){
				slewlimiterUpr.setRiseFall(1.f,1.f);
				outputs[PITCH_OUTPUT_Upr].value = slewlimiterUpr.process(upperNote.volt) + pitchtocvUPR;
				slewUpr = 0.f; // value to retrigger calc next note
			}else{
				outputs[PITCH_OUTPUT_Upr].value = slewlimiterUpr.process(upperNote.volt) + pitchtocvUPR;
			}
		else
			outputs[PITCH_OUTPUT_Upr].value = upperNote.volt + pitchtocvUPR;

		
		bool retriggLwr = gatePulseLwr.process(1.f / engineGetSampleRate());
		bool retriggUpr = gatePulseUpr.process(1.f / engineGetSampleRate());
		bool gateout = anynoteGate || sustpedal;
		
		outputs[RETRIGGATE_OUTPUT_Lwr].value = gateout && !(retriggLwr)? 10.f : 0.f ;
		outputs[RETRIGGATE_OUTPUT_Upr].value = gateout && !(retriggUpr)? 10.f : 0.f ;
		outputs[GATE_OUTPUT].value = gateout ? 10.f : 0.f ;
		
			
		modFilter.lambda = 100.f * engineGetSampleTime();
		outputs[MOD_OUTPUT].value = modFilter.process(rescale(mod, 0, 127, 0.f, 10.f));
		
		breathFilter.lambda = 100.f * engineGetSampleTime();
		outputs[BREATH_OUTPUT].value = breathFilter.process(rescale(breath, 0, 127, 0.f, 10.f));
		
		exprFilter.lambda = 100.f * engineGetSampleTime();
		outputs[EXPRESSION_OUTPUT].value = exprFilter.process(rescale(expression, 0, 127, 0.f, 10.f));
		
		sustainFilter.lambda = 100.f * engineGetSampleTime();
		outputs[SUSTAIN_OUTPUT].value = sustainFilter.process(rescale(sustain, 0, 127, 0.f, 10.f));
		
		pressureFilter.lambda = 100.f * engineGetSampleTime();
		outputs[PRESSURE_OUTPUT].value = pressureFilter.process(rescale(pressure, 0, 127, 0.f, 10.f));
	
		///// RESET MIDI LIGHT
		if (resetMidiTrigger.process(params[RESETMIDI_PARAM].value)) {
			lights[RESETMIDI_LIGHT].value= 1.0f;
			MidiPanic();
			return;
		}
		if (lights[RESETMIDI_LIGHT].value > 0.0001f){
			lights[RESETMIDI_LIGHT].value -= 0.0001f ; // fade out light
		}
	}
/////////////////////// * * * ///////////////////////////////////////////////// * * *
//					  * * *		 E  N  D	  O  F	 S  T  E  P		  * * *
/////////////////////// * * * ///////////////////////////////////////////////// * * *

void processMessage(MidiMessage msg) {
		switch (msg.status()) {
			case 0x8: { // note off
					uint8_t note = msg.data1 & 0x7f;
					noteData[note].velocity = 0;
					noteData[note].aftertouch = 0;
					noteupdated = true;
				}
				break;
			case 0x9: { // note on
					uint8_t note = msg.data1 & 0x7f;
					noteData[note].velocity = msg.data2;
					noteData[note].aftertouch = 0;
					noteupdated = true;
				firstNoGlideLwr = (!anynoteGate && (params[SLEW_LOWER_MODE_PARAM].value > 0.5));
				firstNoGlideUpr = (!anynoteGate  && (params[SLEW_UPPER_MODE_PARAM].value > 0.5));
				}
				break;
			case 0xb: // cc
				processCC(msg);
				break;
			case 0xe: // pitch wheel
				pitch = msg.data2 * 128 + msg.data1;
				break;
			case 0xd: // channel aftertouch
				pressure = msg.data1;
				break;
//		  case 0xf: ///realtime clock etc
//			  processSystem(msg);
//			  break;
			default: break;
		}
}

void processCC(MidiMessage msg) {
	switch (msg.data1) {
		case 0x01: // mod
			mod = msg.data2;
			break;
		case 0x02: // breath
			breath = msg.data2;
			break;
		case 0x0B: // Expression
			expression = msg.data2;
			break;
		case 0x40: { // sustain
			 sustain = msg.data2;
			 if ((params[SUSTAINHOLD_PARAM].value > 0.5) && anynoteGate) sustpedal = (msg.data2 >= 64);
			 else sustpedal = false;
			}
			break;
		default: break;
	}
}

	void MidiPanic() {
		pitch = 8192;
		outputs[PBEND_OUTPUT].value = 0.0f;
		mod = 0;
		outputs[MOD_OUTPUT].value = 0.0f;
		breath = 0;
		outputs[BREATH_OUTPUT].value = 0.0f;
		expression = 0;
		outputs[EXPRESSION_OUTPUT].value = 0.0f;
		pressure = 0;
		outputs[PRESSURE_OUTPUT].value = 0.0f;
		sustain = 0;
		outputs[SUSTAIN_OUTPUT].value = 0.0f;
		sustpedal = false;
	}
	
json_t *toJson() override {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "midi", midiInput.toJson());
	return rootJ;
}

void fromJson(json_t *rootJ) override {
	json_t *midiJ = json_object_get(rootJ, "midi");
	midiInput.fromJson(midiJ);
}

};


///////////

struct MIDIdualCVWidget : ModuleWidget {
	
	MIDIdualCVWidget(MIDIdualCV *module): ModuleWidget(module){
	   setPanel(SVG::load(assetPlugin(plugin, "res/MIDIdualCV.svg")));
		
		//Screws
		addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(0, 365)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 365)));
		
///MIDI
		float xPos = 8.0f;
		float yPos = 19.0f;

		
		MidiWidget *midiWidget = Widget::create<MidiWidget>(Vec(xPos,yPos));
		midiWidget->box.size = Vec(119,36);
		midiWidget->midiIO = &module->midiInput;
		
		midiWidget->driverChoice->box.size.y = 12.f;
		midiWidget->deviceChoice->box.size.y = 12.f;
		midiWidget->channelChoice->box.size.y = 12.f;
		
		midiWidget->driverChoice->box.pos = Vec(0.f, 0.f);
		midiWidget->deviceChoice->box.pos = Vec(0.f, 12.f);
		midiWidget->channelChoice->box.pos = Vec(0.f, 24.f);
		
		midiWidget->driverSeparator->box.pos = Vec(0.f, 12.f);
		midiWidget->deviceSeparator->box.pos = Vec(0.f, 24.f);
		
		midiWidget->driverChoice->font = Font::load(mFONT_FILE);
		midiWidget->deviceChoice->font = Font::load(mFONT_FILE);
		midiWidget->channelChoice->font = Font::load(mFONT_FILE);
		
		midiWidget->driverChoice->textOffset = Vec(2.f,10.f);
		midiWidget->deviceChoice->textOffset = Vec(2.f,10.f);
		midiWidget->channelChoice->textOffset = Vec(2.f,10.f);
		
		midiWidget->driverChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
		midiWidget->deviceChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
		midiWidget->channelChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
		addChild(midiWidget);
		
//	//reset button
		xPos = 89.f;
		yPos = 43.f;
		addParam(ParamWidget::create<moDllzMidiPanic>(Vec(xPos, yPos), module, MIDIdualCV::RESETMIDI_PARAM, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos + 4.f, yPos + 3.5f), module, MIDIdualCV::RESETMIDI_LIGHT));
	
	//Lower-Upper Mods
		
	yPos = 83.f;
		//PitchBend Direct
		addParam(ParamWidget::create<TTrimSnap>(Vec(11.f,yPos), module, MIDIdualCV::PBNEG_LOWER_PARAM, 24.f, -24.0f, 0.f));
		addParam(ParamWidget::create<TTrimSnap>(Vec(33.f,yPos), module, MIDIdualCV::PBPOS_LOWER_PARAM, -24.f, 24.0f, 0.f));
		addParam(ParamWidget::create<TTrimSnap>(Vec(88.f,yPos), module, MIDIdualCV::PBNEG_UPPER_PARAM, 24.f, -24.0f, 0.f));
		addParam(ParamWidget::create<TTrimSnap>(Vec(110.f,yPos), module, MIDIdualCV::PBPOS_UPPER_PARAM, -24.f, 24.0f, 0.f));
	yPos = 108.f;
		//Glide
		addParam(ParamWidget::create<moDllzKnob22>(Vec(18.f,yPos), module, MIDIdualCV::SLEW_LOWER_PARAM, 0.f, 1.f, 0.f));
		addParam(ParamWidget::create<moDllzKnob22>(Vec(95.f,yPos), module, MIDIdualCV::SLEW_UPPER_PARAM, 0.f, 1.f, 0.f));
	yPos = 135.f;
		addParam(ParamWidget::create<moDllzSwitchLedH>(Vec(20.f,yPos), module, MIDIdualCV::SLEW_LOWER_MODE_PARAM, 0.f, 1.f, 0.f));
		addParam(ParamWidget::create<moDllzSwitchLedH>(Vec(97.f,yPos), module, MIDIdualCV::SLEW_UPPER_MODE_PARAM, 0.f, 1.f, 0.f));

	//Lower-Upper Outputs
	yPos = 150.0f;
		addOutput(Port::create<moDllzPort>(Vec(17.5f, yPos), Port::OUTPUT, module, MIDIdualCV::PITCH_OUTPUT_Lwr));
		addOutput(Port::create<moDllzPort>(Vec(94.5f, yPos), Port::OUTPUT, module, MIDIdualCV::PITCH_OUTPUT_Upr));
	yPos = 177.f;
		addOutput(Port::create<moDllzPort>(Vec(17.5f, yPos), Port::OUTPUT, module, MIDIdualCV::VELOCITY_OUTPUT_Lwr));
		addOutput(Port::create<moDllzPort>(Vec(94.5f, yPos), Port::OUTPUT, module, MIDIdualCV::VELOCITY_OUTPUT_Upr));
	yPos = 204.f;
		addOutput(Port::create<moDllzPort>(Vec(17.5f, yPos), Port::OUTPUT, module, MIDIdualCV::RETRIGGATE_OUTPUT_Lwr));
		addOutput(Port::create<moDllzPort>(Vec(94.5f, yPos), Port::OUTPUT, module, MIDIdualCV::RETRIGGATE_OUTPUT_Upr));
		
	//Retrig Switches
		
	yPos = 243.f;
	addParam(ParamWidget::create<moDllzSwitchH>(Vec(19.f, yPos), module, MIDIdualCV::LWRRETRGGMODE_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<moDllzSwitchH>(Vec(96.f, yPos), module, MIDIdualCV::UPRRETRGGMODE_PARAM, 0.0, 1.0, 0.0));
	
	yPos = 240.f;
	//Common Outputs
		addOutput(Port::create<moDllzPort>(Vec(56.f, yPos), Port::OUTPUT, module, MIDIdualCV::GATE_OUTPUT));
	yPos = 286.f;
		addOutput(Port::create<moDllzPort>(Vec(17.f, yPos), Port::OUTPUT, module, MIDIdualCV::PBEND_OUTPUT));
		addOutput(Port::create<moDllzPort>(Vec(44.f, yPos), Port::OUTPUT, module, MIDIdualCV::MOD_OUTPUT));
		addOutput(Port::create<moDllzPort>(Vec(71.f, yPos), Port::OUTPUT, module, MIDIdualCV::BREATH_OUTPUT));
		addOutput(Port::create<moDllzPort>(Vec(98.f, yPos), Port::OUTPUT, module, MIDIdualCV::PRESSURE_OUTPUT));
		
		addOutput(Port::create<moDllzPort>(Vec(17.f, 310.f), Port::OUTPUT, module, MIDIdualCV::PBENDPOS_OUTPUT));
	yPos = 334.f;
		addOutput(Port::create<moDllzPort>(Vec(17.f, yPos), Port::OUTPUT, module, MIDIdualCV::PBENDNEG_OUTPUT));
		addOutput(Port::create<moDllzPort>(Vec(44.f, yPos), Port::OUTPUT, module, MIDIdualCV::EXPRESSION_OUTPUT));
		addOutput(Port::create<moDllzPort>(Vec(71.f, yPos), Port::OUTPUT, module, MIDIdualCV::SUSTAIN_OUTPUT));
	///Sustain hold notes
		addParam(ParamWidget::create<moDllzSwitchLed>(Vec(104.5f, yPos+4.f), module, MIDIdualCV::SUSTAINHOLD_PARAM, 0.0, 1.0, 1.0));
 
	}
};

} // namespace rack_plugin_moDllz

using namespace rack_plugin_moDllz;

RACK_PLUGIN_MODEL_INIT(moDllz, MIDIdualCV) {
   Model *modelMIDIdualCV = Model::create<MIDIdualCV, MIDIdualCVWidget>("moDllz", "MIDIdualCV", "MIDI to dual CV interface", MIDI_TAG, DUAL_TAG, EXTERNAL_TAG);
   return modelMIDIdualCV;
}

