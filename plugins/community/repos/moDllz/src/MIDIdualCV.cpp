
#include <list>
#include <algorithm>
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
        PBDUALPOLARITY_PARAM,
        SUSTAINHOLD_PARAM,
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
    
    struct noteLive{
        int note = 0;
        uint8_t vel = 0;
        bool trig = false;
    };
    noteLive lowerNote;
    noteLive upperNote;
    bool anynoteGate = false;
    bool sustpedal = false;
    
    PulseGenerator gatePulse;
    SchmittTrigger resetMidiTrigger;
    
    
    MIDIdualCV() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
   
    bool noteupdated = false;
    
    
    
    ///////////////////         ////           ////          ////         /////////////////////
    /////////////////   ///////////////  /////////  ////////////  //////  ////////////////////
    /////////////////         ////////  /////////       ///////        //////////////////////
    ///////////////////////   ///////  /////////  ////////////  ////////////////////////////
    //////////////         /////////  /////////         /////  ////////////////////////////
    
    void step() override {
        MidiMessage msg;
        while (midiInput.shift(&msg)) {
            processMessage(msg);
        }
        ///reset triggers...
        lowerNote.trig = false;
        upperNote.trig = false;
        ///////////////////////
        if (noteupdated){
            anynoteGate = false;
            noteupdated = false;
            ///LOWER///
            for (int i = 0; i < 128; i++){
                if (noteData[i].velocity > 0){
                    anynoteGate = true;
                    /////// trigger !!
                    gatePulse.trigger(1e-3);
                    /////////
                    if (i < lowerNote.note) lowerNote.trig = true;
                    lowerNote.note = i;
                    lowerNote.vel = noteData[i].velocity;
                    break;
                }
            }
            if (anynoteGate){
                    ///UPPER///
                    for (int i = 127; i > -1; i--){
                        if (noteData[i].velocity > 0){
                            if (i > upperNote.note) upperNote.trig = true;
                            upperNote.note = i;
                            upperNote.vel = noteData[i].velocity;
                            break;
                        }
                    }
            }else{
                lowerNote.note = 128;
                upperNote.note = -1;
            }
            if (lowerNote.note < 128)
                outputs[PITCH_OUTPUT_Lwr].value = static_cast<float>(lowerNote.note - 60) / 12.0f;
            
            if (upperNote.note > -1)
                outputs[PITCH_OUTPUT_Upr].value = static_cast<float>(upperNote.note - 60) / 12.0f;
            
            if (lowerNote.vel > 0)
            outputs[VELOCITY_OUTPUT_Lwr].value = static_cast<float>(lowerNote.vel) / 127.0f * 10.0f;

            if (upperNote.vel > 0)
            outputs[VELOCITY_OUTPUT_Upr].value = static_cast<float>(upperNote.vel) / 127.0 * 10.0;
        }
        
        bool retriggLwr = lowerNote.trig || params[LWRRETRGGMODE_PARAM].value > 0.5;
        bool retriggUpr = upperNote.trig || params[UPRRETRGGMODE_PARAM].value > 0.5;
        bool pulse = gatePulse.process(1.0 / engineGetSampleRate());
        bool gateout = anynoteGate || sustpedal;
        
        outputs[RETRIGGATE_OUTPUT_Lwr].value = gateout && !(retriggLwr && pulse)? 10.0f : 0.0f ;
        outputs[RETRIGGATE_OUTPUT_Upr].value = gateout && !(retriggUpr && pulse)? 10.0f : 0.0f ;
        outputs[GATE_OUTPUT].value = gateout ? 10.0f : 0.0f ;
        
        pitchFilter.lambda = 100.f * engineGetSampleTime();
        if (params[PBDUALPOLARITY_PARAM].value < 0.5f)
        outputs[PBEND_OUTPUT].value = pitchFilter.process(rescale(pitch, 0, 16384, -5.f, 5.f));
        else
        outputs[PBEND_OUTPUT].value = pitchFilter.process(rescale(pitch, 0, 16384, 0.f, 10.f));
        
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
//                      * * *         E  N  D      O  F     S  T  E  P          * * *
/////////////////////// * * * ///////////////////////////////////////////////// * * *

void processMessage(MidiMessage msg) {
   // debug("MIDI: %01x %01x %02x %02x", msg.status(), msg.channel(), msg.data1, msg.data2);
    if ((midiInput.channel < 0) || (midiInput.channel == msg.channel())){
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
//          case 0xf: ///realtime clock etc
//              processSystem(msg);
//              break;
            default: break;
        }
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

//void processSystem(MidiMessage msg) {
//    switch (msg.channel()) {
//        case 0x8: {
//            debug("timing clock");
//        } break;
//        case 0xa: {
//            debug("start");
//        } break;
//        case 0xb: {
//            debug("continue");
//        } break;
//        case 0xc: {
//            debug("stop");
//        } break;
//        default: break;
//    }
//}
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


struct overMidiDisplay : TransparentWidget {
    
    void draw(NVGcontext* vg)
    {
        NVGcolor backgroundColor = nvgRGBA(0x80, 0x80, 0x80, 0x24);
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, 113, 48, 5.0f);
        nvgFillColor(vg, backgroundColor);
        nvgFill(vg);
        nvgBeginPath(vg);
        NVGcolor linecolor = nvgRGB(0x50, 0x50, 0x50);
        nvgRect(vg, 61, 0, 1, 16);
        nvgFillColor(vg, linecolor);
        nvgFill(vg);
    }
};


struct MIDIdualCVWidget : ModuleWidget {
    
    MIDIdualCVWidget(MIDIdualCV *module): ModuleWidget(module){
       setPanel(SVG::load(assetPlugin(plugin, "res/MIDIdualCV.svg")));
        
        //Screws
        addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
        addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 0)));
        addChild(Widget::create<ScrewBlack>(Vec(0, 365)));
        addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 365)));
        
///MIDI
	float yPos = 21.0f;
    float xPos = 11.0f;
   
        
        MidiWidget *midiWidget = Widget::create<MidiWidget>(Vec(xPos,yPos));
        midiWidget->box.size = Vec(113,48);
        midiWidget->midiIO = &module->midiInput;
        
        midiWidget->driverChoice->box.size = Vec(56,16);
        midiWidget->deviceChoice->box.size = Vec(113,16);
        midiWidget->channelChoice->box.size = Vec(113,16);
        
        midiWidget->driverChoice->box.pos = Vec(0, 0);
        midiWidget->deviceChoice->box.pos = Vec(0, 16);
        midiWidget->channelChoice->box.pos = Vec(0, 32);
        
        midiWidget->driverSeparator->box.pos = Vec(0, 16);
        midiWidget->deviceSeparator->box.pos = Vec(0, 32);
        
        midiWidget->driverChoice->font = Font::load(mFONT_FILE);
        midiWidget->deviceChoice->font = Font::load(mFONT_FILE);
        midiWidget->channelChoice->font = Font::load(mFONT_FILE);
        
        midiWidget->driverChoice->textOffset = Vec(4,12);
        midiWidget->deviceChoice->textOffset = Vec(4,12);
        midiWidget->channelChoice->textOffset = Vec(4,12);
        
        midiWidget->driverChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
        midiWidget->deviceChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
        midiWidget->channelChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
        addChild(midiWidget);
        
        overMidiDisplay *overmididisplay = Widget::create<overMidiDisplay>(Vec(xPos,yPos));
        addChild(overmididisplay);
        
        
//    //reset button
        xPos = 77;
        yPos = 22;
        addParam(ParamWidget::create<moDllzMidiPanic>(Vec(xPos, yPos), module, MIDIdualCV::RESETMIDI_PARAM, 0.0f, 1.0f, 0.0f));
        addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos+35.f, yPos+4.f), module, MIDIdualCV::RESETMIDI_LIGHT));
    
    yPos = 94.0f;

    //Lower-Upper Outputs
    for (int i = 0; i < 3; i++)
    {
        addOutput(Port::create<moDllzPort>(Vec(20, yPos), Port::OUTPUT, module, i * 2));
        addOutput(Port::create<moDllzPort>(Vec(93, yPos), Port::OUTPUT, module, i * 2 + 1));
        yPos += 24;
    }
    //Retrig Switches
    addParam(ParamWidget::create<moDllzSwitch>(Vec(21, 168), module, MIDIdualCV::LWRRETRGGMODE_PARAM, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<moDllzSwitch>(Vec(105, 168), module, MIDIdualCV::UPRRETRGGMODE_PARAM, 0.0, 1.0, 0.0));
    
    yPos = 200;
    xPos = 72;
    //Common Outputs
    for (int i = 6; i < MIDIdualCV::NUM_OUTPUTS; i++)
    {
        addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, i));
        yPos += 23;
    }
    ///PitchWheel +- or +
    addParam(ParamWidget::create<moDllzSwitch>(Vec(105, 225), module, MIDIdualCV::PBDUALPOLARITY_PARAM, 0.0, 1.0, 0.0));
    ///Sustain hold notes
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(105, 318), module, MIDIdualCV::SUSTAINHOLD_PARAM, 0.0, 1.0, 1.0));
 
    }
};

} // namespace rack_plugin_moDllz

using namespace rack_plugin_moDllz;

RACK_PLUGIN_MODEL_INIT(moDllz, MIDIdualCV) {
   Model *modelMIDIdualCV = Model::create<MIDIdualCV, MIDIdualCVWidget>("moDllz", "MIDIdualCV", "MIDI to dual CV interface", MIDI_TAG, DUAL_TAG, EXTERNAL_TAG);
   return modelMIDIdualCV;
}

