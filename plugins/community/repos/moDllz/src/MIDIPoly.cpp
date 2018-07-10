
#include <queue>
#include <list>
#include "midi.hpp"
#include "dsp/filter.hpp"
#include "dsp/digital.hpp"
#include "moDllz.hpp"

namespace rack_plugin_moDllz {

/*
 * MIDIPolyInterface converts midi note on/off events, velocity , pitch wheel and mod wheel to CV
 * with 16 Midi Note Buttons with individual vel & gate
 */
//struct MidiValue {
//    int val = 0; // Controller value
//    //TransitionSmoother tSmooth;
//    bool changed = false; // Value has been changed by midi message (only if it is in sync!)
//};

struct MidiNoteData {
    uint8_t velocity = 0;
    uint8_t aftertouch = 0;
};

struct MIDIPolyInterface : Module {
 static const int numPads = 16;
    
	enum ParamIds {
        KEYBUTTON_PARAM,
		SEQSEND_PARAM = numPads,
        LEARNPAD_PARAM = numPads * 2,
        LOCKPAD_PARAM,
        SEQPAD_PARAM,
        PADXLOCK_PARAM,
        DRIFT_PARAM,
        POLYMODE_PARAM,
        MONOPITCH_PARAM,
        ARCADEON_PARAM,
        ARPEGON_PARAM,
        ARPEGRATIO_PARAM,
        ARPEGOCT_PARAM,
        ARPEGOCTALT_PARAM,
        MONORETRIG_PARAM,
        HOLD_PARAM,
        SEQSPEED_PARAM,
        SEQCLOCKRATIO_PARAM,
        SEQFIRST_PARAM,
        SEQSTEPS_PARAM,
        SEQRUN_PARAM,
        SEQGATERUN_PARAM,
        SEQRETRIG_PARAM,
        SEQRESET_PARAM,
        SEQRUNRESET_PARAM,
        SEQRUNGATE_PARAM,
        SEQCLOCKSRC_PARAM,
        SEQARPSWING_PARAM,
        SWINGTRI_PARAM,
        SEQOCT_PARAM,
        SEQOCTALT_PARAM,
        SEQSWING_PARAM,
        ARPSWING_PARAM,
        SEQTRANUP_PARAM,
        SEQTRANDWN_PARAM,
        POLYTRANUP_PARAM,
        POLYTRANDWN_PARAM,
        POLYUNISON_PARAM,
        LOCKEDPITCH_PARAM,
        LOCKEDRETRIG_PARAM,
        PLAYXLOCKED_PARAM,
        DISPLAYNOTENUM_PARAM,
        RESETMIDI_PARAM,
        TRIMPOLYSHIFT_PARAM,
        TRIMSEQSHIFT_PARAM,
        MUTESEQ_PARAM,
        MUTEMONO_PARAM,
        MUTELOCKED_PARAM,
        MUTEPOLYA_PARAM,
        MUTEPOLYB_PARAM,
        NUM_PARAMS
	};
	enum InputIds {
        POLYUNISON_INPUT,
        POLYSHIFT_INPUT,
        CLOCK_INPUT,
        SEQSPEED_INPUT,
        SEQRATIO_INPUT,
        SEQSTEPS_INPUT,
        SEQFIRST_INPUT,
        SEQRUN_INPUT,
        SEQRESET_INPUT,
        SEQSWING_INPUT,
        SEQSHIFT_INPUT,
        ARPEGRATIO_INPUT,
        ARPMODE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT,
        VEL_OUTPUT = numPads,
        GATE_OUTPUT = numPads * 2,
        SEQPITCH_OUTPUT = numPads * 3,
        SEQVEL_OUTPUT,
        SEQGATE_OUTPUT,
        LOCKEDPITCH_OUTPUT,
        LOCKEDVEL_OUTPUT,
        LOCKEDGATE_OUTPUT,
        MONOPITCH_OUTPUT,
        MONOVEL_OUTPUT,
        MONOGATE_OUTPUT,
        PBEND_OUTPUT,
        MOD_OUTPUT,
        PRESSURE_OUTPUT,
        SUSTAIN_OUTPUT,
        SEQSTART_OUTPUT,
        SEQSTOP_OUTPUT,
        SEQSTARTSTOP_OUTPUT,
        SEQCLOCK_OUTPUT,
        NUM_OUTPUTS
	};
	enum LightIds {
		SEQ_LIGHT,
        RESETMIDI_LIGHT = numPads,
        PLOCK_LIGHT,
        PLEARN_LIGHT,
        PXLOCK_LIGHT,
        PSEQ_LIGHT,
        SEQRUNNING_LIGHT,
        SEQRESET_LIGHT,
        SEQOCT_LIGHT = 23,
        ARPOCT_LIGHT = 28,
		ARCADEON_LIGHT = 33,
        ARPEGON_LIGHT,
        MUTESEQ_LIGHT,
        MUTEMONO_LIGHT,
        MUTELOCKED_LIGHT,
        MUTEPOLY_LIGHT,
        NUM_LIGHTS
	};
    
    
    MidiInputQueue midiInput;
    
    uint8_t mod = 0;
    ExponentialFilter modFilter;
    uint16_t pitch = 8192;
    ExponentialFilter pitchFilter;
    uint8_t sustain = 0;
    ExponentialFilter sustainFilter;
    uint8_t pressure = 0;
    ExponentialFilter pressureFilter;
    
    
    MidiNoteData noteData[128];
    
    enum buttonmodes{
        POLY_MODE,
        SEQ_MODE,
        LOCKED_MODE,
        XLOCK_MODE
    };
    
    struct noteButton{
        int key = 0;
        int vel = 0;
        float drift = 0.0f;
        bool gate = false;
        bool button = false;
        bool newkey = true;
        int mode = 0;
        bool learn = false;
        int velseq = 127; //lastvel for seq
        int stamp = 0;
        bool gateseq = false;
        bool polyretrig = false;
    };
    
    noteButton noteButtons[numPads];
    
    std::list<int> noteBuffer; //buffered notes over polyphony
    

    int polyIndex = 0;
    int polyTopIndex = numPads-1;
    int polymode = 0;
    int lastpolyIndex = 0;
    int polyMaxVoices = 8;
    int stampIx = 0;
    int playingVoices = 0;
    int polyTransParam = 0;
    
    float arpPhase = 0.0f;
    int arpegMode = 0;
    bool arpegStep = false;
    bool arpSwingDwn = true;
    int arpclockRatio = 0;
    int arpegIx = 0;
    int arpegFrame = 0;
    int arpOctIx = 0;
    bool arpegStarted = false;
    int arpegCycle = 0;
    int arpOctValue = 0;
    bool syncArpPhase = false;
    int arpDisplayIx = -1;
    
    const int octaveShift[7][5] =
    {   {0,-1,-2,-1, 8},
        {0,-1,-2, 8, 8},
        {0,-1, 8, 8, 8},
        {0, 8, 8, 8, 8},
        {0, 1, 8, 8, 8},
        {0, 1, 2, 8, 8},
        {0, 1, 2, 1, 8}
    };
    
    int liveMono = 0;
    int lockedMono = 0;
    int lastMono = -1;
    int lastLocked = -1;
 
    bool pedal = false;
    bool sustainhold = true;
    
//    MidiValue mod;
//    MidiValue pitchBend;
//    MidiValue sustain;
//    MidiValue afterTouch;

    float drift[numPads] = {0.0f};
    
    
    bool padSetLearn = false;
    int padSetMode = 0;
    
    float seqPhase = 0.0f;
    bool seqSwingDwn = true;
    bool seqrunning = false;
    bool seqResetNext = false;
    int seqSteps = 16;
    int seqOffset = 0;
    int seqStep = 0;
    int seqi = 0;
    int seqiWoct = 0;
    int seqOctIx = 0;
    int seqOctValue = 3;
    int seqTransParam = 0;
    int seqSampleCount = 0;
    int arpSampleCount = 0;
    int ClockSeqSamples = 1;
    int ClockArpSamples = 1;
    float seqResetLight = 0;
 
    const float ClockRatios[13] ={0.50f, 2.f/3.f,0.75f, 1.0f ,4.f/3.f,1.5f, 2.0f, 8.f/3.f, 3.0f, 4.0f, 6.0f, 8.0f,12.0f};
    const std::string stringClockRatios[13] ={"1/2", "1/4d","1/2t", "1/4", "1/8d", "1/4t","1/8","1/16d","1/8t","1/16","1/16t","1/32","1/32t"};
    const bool swingTriplet[13] = {true,true,false,true,true,false,true,true,false,true,false,true,false};
    
    
    int clockSource;
    int seqclockRatio;
    std::string mainDisplay[4] = {""};
    bool dispNotenumber = false;
    
    SchmittTrigger resetMidiTrigger;
    SchmittTrigger extClockTrigger;
    SchmittTrigger seqRunTrigger;
    SchmittTrigger seqResetTrigger;
    SchmittTrigger setLockTrigger;
    SchmittTrigger setLearnTrigger;
    SchmittTrigger setSeqTrigger;
    SchmittTrigger setPadTrigger;
    SchmittTrigger setArcadeTrigger;
    SchmittTrigger setArpegTrigger;
    SchmittTrigger octUpTrigger;
    SchmittTrigger octDwnTrigger;
    SchmittTrigger seqTransUpTrigger;
    SchmittTrigger seqTransDwnTrigger;
    SchmittTrigger polyTransUpTrigger;
    SchmittTrigger polyTransDwnTrigger;
    
    SchmittTrigger muteSeqTrigger;
    SchmittTrigger muteMonoTrigger;
    SchmittTrigger muteLockedTrigger;
    SchmittTrigger mutePolyATrigger;
    SchmittTrigger mutePolyBTrigger;
    
    bool muteSeq = false;
    bool muteMono = false;
    bool muteLocked = false;
    bool mutePoly = false;

    PulseGenerator gatePulse;
    PulseGenerator keyPulse;
    PulseGenerator monoPulse;
    PulseGenerator lockedPulse;
  
    PulseGenerator clockPulse;
    PulseGenerator startPulse;
    PulseGenerator stopPulse;
    
    bool clkMIDItick = false;
    bool MIDIstart = false;
    bool MIDIstop = false;
    bool MIDIcont = false;
    bool stopped = true;
    bool arpMIDItick = false;
    bool seqMIDItick = false;
    int seqTickCount = 0;
    int arpTickCount = 0;
    int sampleFrames = 0;
    int calcBPM = 0;
    float BPMrate = 0.0f;
    int bpmDecimals = 0;
    bool firstBPM = true; ///to hold if no clock ...and skip first BPM calc...
    bool extBPM = false;
    

    ///////////////
	MIDIPolyInterface() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
     onReset();
	}

	~MIDIPolyInterface() {
	};
    
    void doSequencer();
    
	void step() override;

    void processMessage(MidiMessage msg);
    void processCC(MidiMessage msg);
    void processSystem(MidiMessage msg);
    
    void pressNote(int note, int vel);

	void releaseNote(int note);
    
    void releasePedalNotes();
    
    void setPolyIndex(int note);
    
    void initPolyIndex();
    
    void recoverBufferedNotes();
    
    void setMainDisplay(int ix);

    void getBPM();
    
    float minmaxFit(float val, float minv, float maxv);
 
    void MidiPanic();


	void onReset() override {
		// resetMidi();
        for (int i = 0; i < numPads; i++)
        {
            noteButtons[i].key= 48 + i;
            if (i < 8 ) noteButtons[i].mode = POLY_MODE;
            else if(i < 12) noteButtons[i].mode = SEQ_MODE;
            else noteButtons[i].mode = LOCKED_MODE;
           //// else noteButtons[i].mode = XLOCK_MODE;
        }
        polyIndex = 0;
        polyTopIndex = 7;;
        lastpolyIndex = 0;
        seqTransParam = 0;
        for (int i = 0; i < NUM_OUTPUTS; i++){
        outputs[i].value= 0.0f;
        }
        params[SEQRESET_PARAM].value = 0.0f;
	}
    /* initialize random seed: */
  //  srand (time(NULL));
    
    json_t *toJson() override {
        json_t *rootJ = json_object();
        
         json_object_set_new(rootJ, "midi", midiInput.toJson());
       // addBaseJson(rootJ);
        for (int i = 0; i < numPads; i++) {
            json_object_set_new(rootJ, ("key" + std::to_string(i)).c_str(), json_integer(noteButtons[i].key));
            json_object_set_new(rootJ, ("mode" + std::to_string(i)).c_str(), json_integer(noteButtons[i].mode));
        }
        json_object_set_new(rootJ, "seqtransp", json_integer(seqTransParam));
        json_object_set_new(rootJ, "polytransp", json_integer(polyTransParam));
        json_object_set_new(rootJ, "arpegmode", json_integer(arpegMode));
        json_object_set_new(rootJ, "seqrunning", json_boolean(seqrunning));
        return rootJ;
    }
    
    void fromJson(json_t *rootJ) override {
        
        json_t *midiJ = json_object_get(rootJ, "midi");
        midiInput.fromJson(midiJ);
       // baseFromJson(rootJ);
        for (int i = 0; i < numPads; i++) {
            json_t *keyJ = json_object_get(rootJ,("key" + std::to_string(i)).c_str());
            // if (keyJ)
                noteButtons[i].key = json_integer_value(keyJ);
            json_t *modeJ = json_object_get(rootJ,("mode" + std::to_string(i)).c_str());
            // if (modeJ)
                noteButtons[i].mode = json_integer_value(modeJ);
                noteButtons[i].learn = false;
        }
        json_t *seqtranspJ = json_object_get(rootJ,("seqtransp"));
       //  if (seqtranspJ) {
        seqTransParam = json_integer_value(seqtranspJ);
       // }
        json_t *polytranspJ = json_object_get(rootJ,("polytransp"));
       // if (polytranspJ) {
            polyTransParam = json_integer_value(polytranspJ);
       // }
        json_t *arpegmodeJ = json_object_get(rootJ,("arpegmode"));
       // if (arpegmodeJ) {
            arpegMode = json_integer_value(arpegmodeJ);
       // }
        json_t *seqrunningJ = json_object_get(rootJ,("seqrunning"));
       // if (seqrunningJ) {
            seqrunning = json_is_true(seqrunningJ);
       // }
        
        padSetMode = POLY_MODE;
        padSetLearn = false;
        ///midiInput.rtMidiIn->ignoreTypes(true,false,false);
        MidiPanic();
        initPolyIndex();
    }
};



void MIDIPolyInterface::initPolyIndex(){
    polyTopIndex = -1;
    int iPoly = 0;
    for (int i = 0 ; i < numPads; i++){
        if (noteButtons[i].mode == POLY_MODE){
            iPoly ++;
                polyMaxVoices = iPoly;
                polyTopIndex = i;
        }
    }
    polyIndex = polyTopIndex;
    noteBuffer.clear();
}

void MIDIPolyInterface::pressNote(int note, int vel) {
    stampIx ++ ; // update note press stamp for mono "last"
    if (playingVoices == polyMaxVoices) keyPulse.trigger(1e-3);
    if (params[MONORETRIG_PARAM].value > 0.5f) monoPulse.trigger(1e-3);
    if (params[LOCKEDRETRIG_PARAM].value > 0.5f) lockedPulse.trigger(1e-3);

    bool (Xlockedmatch) = false;
    for (int i = 0; i < numPads; i++){
        noteButtons[i].polyretrig = false;
        if (noteButtons[i].learn) {
            noteButtons[i].key= note;
            noteButtons[i].learn = false;
        }
 /////////play every matching locked note...
            if ((note == noteButtons[i].key)  && (noteButtons[i].mode > 1)){
                noteButtons[i].gate = true;
                noteButtons[i].newkey = true; //same key but true to display velocity
                noteButtons[i].vel = vel;
                noteButtons[i].velseq = vel;
                noteButtons[i].stamp = stampIx;
                //
                if (noteButtons[i].mode == XLOCK_MODE) Xlockedmatch = true;
            }
    }

    //if (params[PLAYXLOCKED_PARAM].value < 0.5f)
        if (Xlockedmatch) return;
    
    ///////////////////////
    if (polyIndex > -1){ //polyIndex -1 if all notes are locked or seq
        setPolyIndex(note);

        ////////////////////////////////////////
        /// if gate is on set retrigg
        noteButtons[polyIndex].polyretrig = noteButtons[polyIndex].gate;
        noteButtons[polyIndex].stamp = stampIx;
        noteButtons[polyIndex].key = note;
        noteButtons[polyIndex].vel = vel;
        noteButtons[polyIndex].velseq = vel;
        noteButtons[polyIndex].newkey = true;
        noteButtons[polyIndex].gate = true;
    }
    return;
}

void MIDIPolyInterface::releaseNote(int note) {
   // auto it = std::find(noteBuffer.begin(), noteBuffer.end(), note);
    //if (it != noteBuffer.end()) noteBuffer.erase(it);
    noteBuffer.remove(note);
    if ((params[MONORETRIG_PARAM].value > 0.5f) && (params[MONOPITCH_PARAM].value != 1.0f)) monoPulse.trigger(1e-3);
    if ((params[LOCKEDRETRIG_PARAM].value > 0.5f) && (params[LOCKEDPITCH_PARAM].value != 1.0f)) lockedPulse.trigger(1e-3);
    for (int i = 0; i < numPads; i++)
    {
        if ((note == noteButtons[i].key) && (noteButtons[i].vel > 0)){
            // polyIndex = i;
            if (!noteButtons[i].button) noteButtons[i].vel = 0;
            
            noteButtons[i].gate = pedal && sustainhold;
            if ((noteButtons[i].mode == POLY_MODE) && (!noteButtons[i].gate) && (!noteBuffer.empty())){
                //recover if buffered over number of voices
                noteButtons[i].key = noteBuffer.front();
                noteBuffer.pop_front();
                noteButtons[i].gate = true;
                noteButtons[i].vel = noteButtons[i].velseq;
            }
        }
    }
    return;
}
void MIDIPolyInterface::releasePedalNotes() {
    //    gatePulse.trigger(1e-3);
    for (int i = 0; i < numPads; i++)
    {
        if (noteButtons[i].vel == 0){
           
            if (!noteBuffer.empty()){//recover if buffered over number of voices
            noteButtons[i].key = noteBuffer.front();
            noteButtons[i].vel = noteButtons[i].velseq;
            noteBuffer.pop_front();
            }else{
            noteButtons[i].gate = false;
            }
        }
    }
    return;
}

/////////////////////////////////////////////   SET POLY INDEX  /////////////////////////////////////////////
void MIDIPolyInterface::setPolyIndex(int note){

     polymode = static_cast<int>(params[POLYMODE_PARAM].value);
     if (polymode < 1) polyIndex ++;
     else if (pedal && sustainhold){///check if note is held to recycle it....
            for (int i = 0; i < numPads; i ++){
                         if ((noteButtons[i].mode == POLY_MODE) && (noteButtons[i].gate) && (noteButtons[i].key == note)){
            //                ///note is already on....
                            lastpolyIndex = i;
                            polyIndex = i;
                            return;
                        }
                    }
            }else if (polymode > 1) polyIndex = 0;
 
    int ii = polyIndex;
    for (int i = 0; i < (polyTopIndex + 1); i ++){
        if (ii > polyTopIndex) ii = 0;
        //if (!(noteButtons[ii].locked)){
          if (noteButtons[ii].mode == POLY_MODE){
            if (!noteButtons[ii].gate){
                lastpolyIndex = ii;
                polyIndex = ii;
                return;
            }
        }
        ii ++;
    }
    
    //////////scan to steal oldest note.......
    lastpolyIndex ++;
    ii = lastpolyIndex;
    for (int i = 0; i < (polyTopIndex + 1); i ++){
        if (ii > polyTopIndex) ii = 0;
      //if (!(noteButtons[ii].locked)){
        if (noteButtons[ii].mode == POLY_MODE){
            lastpolyIndex = ii;
            polyIndex = ii;
            /// pulse for reTrigger
            //noteButtons[ii].polyretrig = false;
            ///// save to Buffer /////

           // int notebffr;
           // notebffr = noteButtons[ii].key;
          if (noteButtons[ii].vel > 0) noteBuffer.push_front(noteButtons[ii].key);          ///////////////
            return;
        }
        ii ++;
    }
}

///////////////////////////////  END SET POLY INDEX  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//        MMM       MMM   III   DDDDDDDD      III
//        MMMM     MMMM   III   DDD    DDD    III
//        MMMMM   MMMMM   III   DDD     DDD   III
//        MMMMMM MMMMMM   III   DDD     DDD   III
//        MMM MMMMM MMM   III   DDD     DDD   III
//        MMM  MMM  MMM   III   DDD    DDD    III
//        MMM   M   MMM   III   DDDDDDDD      III
//////////////////////////////////////////////////////////////////
void MIDIPolyInterface::processMessage(MidiMessage msg) {
    if (msg.status() == 0xf) {
        ///clock
        processSystem(msg);
        return;
    }
    
    
    if ((midiInput.channel < 0) || (midiInput.channel == msg.channel())){
        switch (msg.status()) {
             case 0x8: {
                        releaseNote(msg.data1 & 0x7f);
             }
                             break;
             case 0x9: {// note on
                 if (msg.data2 > 0) {
                     pressNote((msg.data1 & 0x7f), msg.data2);
                 } else {
                     // For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
                     releaseNote(msg.data1 & 0x7f);
                 }
             }
                 break;
             case 0xb: {
                 processCC(msg);
             }
              break;
                 // pitch wheel
             case 0xe: {
                 pitch = msg.data2 * 128 + msg.data1;
             }
              break;
                 // channel aftertouch
             case 0xd: {
                 pressure = msg.data1;
             }
              break;

             default: break;
         }
    }
}

void MIDIPolyInterface::processCC(MidiMessage msg) {
    switch (msg.data1) {
            // mod
        case 0x01: {
            mod = msg.data2;
        } break;
            // sustain
        case 0x40: {
            sustain = msg.data2;
                if (sustain >= 64) {
                    pedal = sustainhold;
                }else {
                    pedal = false;
                    if (sustainhold) releasePedalNotes();
                }
        } break;
        default: break;
    }
}

void MIDIPolyInterface::processSystem(MidiMessage msg) {
        switch (msg.channel()) {
            case 0x8: {
             //   debug("timing clock");
                clkMIDItick = true;
                seqMIDItick = true;
                arpMIDItick = true;
            } break;
            case 0xa: {
              //  debug("start");
                MIDIstart = true;
            } break;
            case 0xb: {
              //  debug("continue");
                MIDIcont = true;
            } break;
            case 0xc: {
              //  debug("stop");
                MIDIstop = true;
            } break;
            default: break;
        }
}

void MIDIPolyInterface::MidiPanic() {
    pitch = 8192;
    outputs[PBEND_OUTPUT].value = 0.0f;
    mod = 0;
    outputs[MOD_OUTPUT].value = 0.0f;
    pressure = 0;
    outputs[PRESSURE_OUTPUT].value = 0.0f;
    sustain = 0;
    outputs[SUSTAIN_OUTPUT].value = 0.0f;
    pedal = false;
    for (int i = 0; i < numPads; i++)
    {
        noteButtons[i].vel = 0;
        noteButtons[i].gate = false;
        noteButtons[i].button = false;
    }
    noteBuffer.clear();
}
/////////////////////////////
////////////////////////////////// DISPLAY
//////////////////////////////////////////
void MIDIPolyInterface::getBPM(){
    if (firstBPM){
        firstBPM = false;
        sampleFrames = 0;
        return;
    }
    float fBPM = 0.25f + engineGetSampleRate() * 60.0f / static_cast<float>(sampleFrames); // /  ;
    sampleFrames = 0;
    calcBPM = static_cast<int>(fBPM);
}

void MIDIPolyInterface::setMainDisplay(int ix)
{
    std::string d[4] = {""};
    std::string bpmstring = "";

    
    std::string s;
    
    if (calcBPM > 0) {
        if (extBPM) bpmstring = " "+ std::to_string (calcBPM)+"bpm ";
        else bpmstring = "*("+ std::to_string (calcBPM)+"bpm) ";
    } else {
        bpmstring = "...no clock.. ";
    }
    switch (ix) {
        case 0:{
            d[0] = "EXT" + bpmstring + stringClockRatios[seqclockRatio];
            } break;
        case 1:{
            std::stringstream stream;
            stream << std::fixed << std::setprecision(bpmDecimals) << BPMrate;
            s = stream.str();
            d[0] = "INT " + s + "bpm "+ stringClockRatios[seqclockRatio];
            }   break;
        case 2:{
            d[0] = "MIDI" + bpmstring + stringClockRatios[seqclockRatio];
            }   break;
        default:
            break;
    }
    d[1] = "Steps: " + std::to_string(seqSteps) + " First: " + std::to_string(seqOffset + 1);
    
    switch (arpegMode){
        case 1:{
            if (outputs[MONOPITCH_OUTPUT].active) d[2] = "Arpeggiator: " + stringClockRatios[arpclockRatio];
            else d[2] = "...connect mono out";
            }break;
        case 2:{
            if (outputs[MONOPITCH_OUTPUT].active) d[2] = "Arpeggiator: Arcade";
            else d[2] = "...connect mono out";
            }break;
        default:{
            d[2] = "";
//             for (std::list<int>::const_iterator i = noteBuffer.begin(); i != (noteBuffer.end()); ++i)
//            {  int x = *i;
//            d[2] = d[2] +" "+ std::to_string(x);
//            }//
            }break;
    }
  
    d[3]= std::to_string(playingVoices)+"/"+std::to_string (polyMaxVoices);
    mainDisplay[0]= d[0];
    mainDisplay[1]= d[1];
    mainDisplay[2]= d[2];
    mainDisplay[3]= d[3];
    return;
}

float MIDIPolyInterface::minmaxFit(float val, float minv, float maxv){
    if (val < minv) val = minv;
    else if (val > maxv) val = maxv;
    return val;
}
//float driftvolt(int maxdrift){///mV drift
//        return   =  ((double) rand() / (RAND_MAX)));
//}

///////////////////         ////           ////          ////         /////////////////////
/////////////////   ///////////////  /////////  ////////////  //////  ////////////////////
/////////////////         ////////  /////////       ///////        //////////////////////
///////////////////////   ///////  /////////  ////////////  ////////////////////////////
//////////////         /////////  /////////         /////  ////////////////////////////

void MIDIPolyInterface::step() {

    //// mono modes and indexes
    int liveMonoMode = static_cast <int>(params[MONOPITCH_PARAM].value);
    int liveIx = 128 - liveMonoMode * 65; //first index -2 for Upper,(63 not used for Last), 128 for Lower
    int liveMonoIx = 0;
    int lockedMonoMode = static_cast <int>(params[LOCKEDPITCH_PARAM].value);
    int lockedIx =  128 - lockedMonoMode * 65; //first index -2 for Upper,(63 not used for Last), 128 for Lower
    int lockedMonoIx = 0;
    ////
    int getclockSource = static_cast<int>(params[SEQCLOCKSRC_PARAM].value);
    if (clockSource != getclockSource)  {
       // calcBPM = 0;
        clockSource = getclockSource;
        
    }
    /////GET INTERNAL BPM with CV
    if (clockSource == 1){
        if (inputs[SEQSPEED_INPUT].active){
            bpmDecimals = 2;
            float vcBPMtime = minmaxFit(inputs[SEQSPEED_INPUT].value,-10.0f,10.0f) * 24.0f;
            BPMrate = minmaxFit(params[SEQSPEED_PARAM].value + vcBPMtime, 12.0f, 360.0f);
        }
        else{
            bpmDecimals = 0;
            BPMrate =  params[SEQSPEED_PARAM].value;
        }
    }
////////// MIDI MESSAGE ////////

   
    MidiMessage msg;
    while (midiInput.shift(&msg)) {
        processMessage(msg);
    }
    
    
    
    
    bool analogdrift = (params[DRIFT_PARAM].value > 0.0001f);
    bool newdrift = false;
    if (analogdrift){
        static int framedrift = 0;
        if (framedrift > engineGetSampleRate())
        {
            newdrift = true;
            framedrift = 4 * rand() % static_cast<int>(engineGetSampleRate()/2.0f);
        }else{
            newdrift = false;
            framedrift++;
        }
    }
    //////////////////////////////
    playingVoices = 0;
    bool monogate = false;
 //   bool retrigLive = false;
    bool lockedgate = false;
 //   bool retrigLocked = false;
    static int lockedM = 0;
    static int liveM = 0;
    for (int i = 0; i < numPads; i++)
    {
        if ((!noteButtons[i].button) && (params[KEYBUTTON_PARAM + i].value > 0.5f)){ ///button ON
            if (noteButtons[i].mode == 0) noteButtons[i].polyretrig = noteButtons[i].gate;
            noteButtons[i].vel = 127;
            noteButtons[i].button = true;
            //noteButtons[i].monoretrig = true;
            /// update note press stamp for mono "last"
            stampIx ++;
            noteButtons[i].stamp = stampIx;
            //if ((noteButtons[i].mode <2)&&((params[MONORETRIG_PARAM].value > 0.5f)||(params[LOCKEDRETRIG_PARAM].value > 0.5f))){
            if (noteButtons[i].mode != SEQ_MODE) {
            keyPulse.trigger(1e-3);
            monoPulse.trigger(1e-3);
            lockedPulse.trigger(1e-3);
            }
            ///////////// SET BUTTON MODE
            if (padSetMode>0) {
                //noteButtons[i].locked = !noteButtons[i].locked;
                if (noteButtons[i].mode != padSetMode) noteButtons[i].mode = padSetMode;
                else noteButtons[i].mode = POLY_MODE;
                initPolyIndex();
            }else if ((padSetLearn) && (noteButtons[i].mode >0)) ///learn if pad locked or Seq
                noteButtons[i].learn = !noteButtons[i].learn;
            ////////////////////////////////////////////
            //if (!noteButtons[i].gate) noteButtons[i].vel = 64;

            
        } else if ((noteButtons[i].button) && (params[KEYBUTTON_PARAM + i].value < 0.5f)){///button off
            if ((noteButtons[i].mode == POLY_MODE) && (liveMonoMode != 1)) monoPulse.trigger(1e-3);
            else if ((noteButtons[i].mode > SEQ_MODE) && (lockedMonoMode != 1)) lockedPulse.trigger(1e-3);
            noteButtons[i].button = false;
            if (!noteButtons[i].gate) noteButtons[i].vel = 0;
        }
        
        bool thisgate = noteButtons[i].gate || noteButtons[i].button;
        //// retrigger ...
        
        bool outgate = thisgate;
        if (noteButtons[i].polyretrig) outgate = !keyPulse.process(engineGetSampleTime());
        outputs[GATE_OUTPUT + i].value =  !mutePoly && outgate ? 10.0f : 0.0f;
        
        if (thisgate){
        
        outputs[VEL_OUTPUT + i].value = noteButtons[i].velseq / 127.0f * 10.0f; //(velocity from seq 'cause it doesn't update to zero)
        if (!padSetLearn) noteButtons[i].learn = false;
        
        // get mono values
        if (noteButtons[i].mode == POLY_MODE){
            playingVoices ++;
            switch (liveMonoMode){
                case 0:{
                    //// get lowest pressed note
                    if (noteButtons[i].key < liveIx) {
                        liveIx = noteButtons[i].key;
                        liveM = i;
                        monogate = true;
                    }
                    }break;
                case 1:{
                    if (noteButtons[i].stamp > liveMonoIx ) {
                        liveMonoIx = noteButtons[i].stamp;
                        liveM = i;
                        monogate = true;
                    }
                    }break;
                default:{
                    //// get highest pressed note
                    if (noteButtons[i].key > liveIx) {
                        liveIx = noteButtons[i].key;
                        liveM = i;
                        monogate = true;
                    }
                    }break;
            }
        }else if ((noteButtons[i].mode == LOCKED_MODE) || ((noteButtons[i].mode == XLOCK_MODE)&&(params[PLAYXLOCKED_PARAM].value>0.5))){
            switch (lockedMonoMode){
                case 0:{
                    //// get lowest pressed note
                    if (noteButtons[i].key < lockedIx) {
                        lockedIx = noteButtons[i].key;
                        lockedM = i;
                        lockedgate = true;
                    }
                    }break;
                case 1:{
                    if (noteButtons[i].stamp > lockedMonoIx ) {
                        lockedMonoIx = noteButtons[i].stamp;
                        lockedM= i;
                        lockedgate = true;
                    }
                    }break;
                default:{
                    //// get highest pressed note
                    if (noteButtons[i].key > lockedIx) {
                        lockedIx = noteButtons[i].key;
                        lockedM = i;
                        lockedgate = true;
                    }
                    }break;
            }
        }

       ///////// POLY PITCH OUTPUT///////////////////////
            if (analogdrift){
                static int bounced[numPads] = {0};
                 float dlimit = (0.1f + params[DRIFT_PARAM].value )/ 26.4f;
                const float driftfactor = 1e-9f;
                if (newdrift) {
                    bounced[i] = 0;
                    int randrange = static_cast<int>(1.0f + params[DRIFT_PARAM].value) * 300;
                    drift[i] = (static_cast<float>(rand() % randrange * 2 - randrange)) * driftfactor * ( 0.5f + params[DRIFT_PARAM].value);
                }       //
                noteButtons[i].drift += drift[i];
                if ((bounced[i] < 1) && (noteButtons[i].drift > dlimit ))  {
                    drift[i] = -drift[i];
                    bounced[i] = 1;
                }else if ((bounced[i] > -1) && (noteButtons[i].drift < - dlimit ))  {
                    drift[i] = -drift[i];
                    bounced[i] = -1;
                }
            
            }else noteButtons[i].drift = 0.0f; // no analog drift
     
            float noteUnison = 0.0f;
            
            if (inputs[POLYUNISON_INPUT].active)
            noteUnison = (minmaxFit(inputs[POLYUNISON_INPUT].value * 0.1f ,-10.0f,10.f)) * params[POLYUNISON_PARAM].value * (noteButtons[liveMono].key - noteButtons[i].key);
            else  noteUnison = params[POLYUNISON_PARAM].value * (noteButtons[liveMono].key - noteButtons[i].key);
            
            outputs[PITCH_OUTPUT + i].value = noteButtons[i].drift + (inputs[POLYSHIFT_INPUT].value/48.0f * params[TRIMPOLYSHIFT_PARAM].value) + (polyTransParam + noteButtons[i].key + noteUnison - 60) / 12.0f;      //
            //////////////////////////////////////////////////

        }
        
        lights[SEQ_LIGHT + i].value = (seqStep == i) ? 1.0f : 0.0f;

       
       }  //// end for i to numPads
    lockedMono = lockedM;
    liveMono = liveM;

    sustainhold = params[HOLD_PARAM].value > 0.5f;
   

    //  Output Live Mono note
   // bool monogate = (noteButtons[liveMono].gate || noteButtons[liveMono].button);
    if (outputs[MONOPITCH_OUTPUT].active) {
     
        //////////////////////////////////////
        /////////////////////////////////  ///
        /// A R C A D E // S C A N ///      //
        ///////////////////////////////   ////
        ////////////////////////////// // ////
        ///// A R P E G G I A T O R //////////
        if (arpegMode > 0) {
            
            int updArpClockRatio = 0;
            if (inputs[ARPEGRATIO_INPUT].active)
            updArpClockRatio = static_cast<int>(minmaxFit(inputs[ARPEGRATIO_INPUT].value + params[ARPEGRATIO_PARAM].value, 0.0f, 11.0f));
            else
            updArpClockRatio = static_cast<int>(params[ARPEGRATIO_PARAM].value);
            
            if (monogate){
                
                if (!arpegStarted) {
                    arpegStarted = true;
                    arpegIx = liveMono;
                    arpegCycle = 0;
                    arpOctIx = 0;
                    arpSampleCount = 0;
                    arpPhase = 0.0f;
                    arpTickCount = 0;
                   /////////////////////////////////
                    syncArpPhase = seqrunning;///// SYNC with seq if running
                   /////////////////////////////////
                }
                
               if (arpegMode > 1){/////// ARCADE / / / / / / /
                   arpegFrame ++;
                   if (arpegFrame > (engineGetSampleRate() / 60.0f)){
                       arpegStep = true;
                       arpegFrame = 0;
                   }
               }else {//if (!seqrunning){ /// set clocks if sequence is not running.....
                float arpswingPhase;
                float  swingknob = params[SEQARPSWING_PARAM].value / 40.0f;
                   bool swingThis ((params[ARPSWING_PARAM].value > 0.5f) && ((swingTriplet[arpclockRatio]) || (params[SWINGTRI_PARAM].value > 0.5f)));
                   if (clockSource < 2) {
                       if (clockSource == 1) ClockArpSamples = static_cast<int>(engineGetSampleRate() * 60.0f/BPMrate / ClockRatios[arpclockRatio] + 0.5f);
                           arpPhase = static_cast<float>(arpSampleCount)/static_cast<float>(ClockArpSamples);
                           arpSampleCount++;
                       
                       if (swingThis){
                               if (arpSwingDwn) arpswingPhase = 1.0f + swingknob;
                               else arpswingPhase = 1.0f - swingknob;
                           } else arpswingPhase = 1.0f;
                           
                           if (arpPhase >= arpswingPhase) {
                               arpPhase = 0.0f;
                               arpSwingDwn = !arpSwingDwn;
                               arpegStep = true;
                               arpSampleCount = 0;
                           }
                   }else{
                       int arpMidiPhase;
                       int swingtick = static_cast<int>(swingknob * (24/ClockRatios[seqclockRatio]));
                       
                       if (arpMIDItick){
                           arpMIDItick = false;
                           arpTickCount ++; /// arpeggiator sync with sequencer
                           if (swingThis){
                               if (arpSwingDwn) arpMidiPhase = 24/ClockRatios[arpclockRatio] + swingtick;
                               else arpMidiPhase = 24/ClockRatios[arpclockRatio] - swingtick;
                           } else arpMidiPhase = 24/ClockRatios[arpclockRatio];
                           
                           if (arpTickCount >= arpMidiPhase) {
                               arpTickCount = 0;
                               arpSwingDwn = !arpSwingDwn;
                               arpegStep = true;
                           }
                       }
                   }
            }// END IF MODE ARP
              /// first gate --> set up indexes
 
            }else{ //////////    GATE OFF //arpeg ON no gate (all notes off) reset..
                arpegStarted = false;
                //if (!seqrunning) {
                    arpSampleCount = 0;
                    arpPhase = 0.0f;
                    arpSwingDwn = true;
               // }
        }
        bool notesFirst = (params[ARPEGOCTALT_PARAM].value > 0.5f);
        
        if ((arpegStep) && (monogate)){
            
            if  (arpclockRatio != updArpClockRatio){
                arpclockRatio = updArpClockRatio;
                arpPhase = 0.0f;
                arpSwingDwn = true;
                arpSampleCount = 0;
            }///update ratio....
            
            int arpOctaveKnob = static_cast<int>(params[ARPEGOCT_PARAM].value);
                 for (int i = 0 ; i < 5; i++){
                   lights[ARPOCT_LIGHT + i].value = 0.0f;
                 }
            arpegStep=false;

                if (notesFirst) {
               ////////// NOTE to arp and CYCLE FLAG
                    arpegIx ++;
                    if (arpegIx > polyTopIndex) arpegIx = 0;
                    for (int i = 0; i <= polyTopIndex; i++){
                        if ((noteButtons[arpegIx].mode == POLY_MODE) && ((noteButtons[arpegIx].gate) || (noteButtons[arpegIx].button))) {
                            arpegCycle++;
                            if (arpegCycle >= playingVoices){
                                arpegCycle = 0;
                                arpOctValue = arpOctaveKnob;
                                if (octaveShift[arpOctValue][arpOctIx + 1] > 2){
                                    arpOctIx = 0;
                                } else {
                                    arpOctIx ++;
                                }
                            }
                            break;
                        }
                        arpegIx ++;
                        if (arpegIx > polyTopIndex) arpegIx = 0;
                        }
                ///////////
                }else {
                   arpOctValue = arpOctaveKnob;
                    if (octaveShift[arpOctValue][arpOctIx + 1] > 2) {
                        arpegIx ++;
                        arpOctIx= 0;
                    for (int i = 0; i <= polyTopIndex; i++){
                        if ((noteButtons[arpegIx].mode == POLY_MODE) && ((noteButtons[arpegIx].gate) || (noteButtons[arpegIx].button))) {
                            break;
                        }
                        arpegIx ++;
                        if (arpegIx > polyTopIndex) arpegIx = 0;
                        }
                    } else {
                        arpOctIx ++;
                    }
                }
            lights[ARPOCT_LIGHT + 2 + octaveShift[arpOctValue][arpOctIx]].value = 1.0f;
            arpDisplayIx = arpegIx;
        }else if (!monogate){
            ///keysUp...update arp ratio
            arpclockRatio = updArpClockRatio;
            arpDisplayIx = -1;
            for (int i = 0 ; i < 5; i++){
                lights[ARPOCT_LIGHT + i].value = 0.0f;
            }
        }
             outputs[MONOPITCH_OUTPUT].value = octaveShift[arpOctValue][arpOctIx] + (noteButtons[arpegIx].key - 60) / 12.0f;
        }else{
            /// Normal Mono /////////////
            outputs[MONOPITCH_OUTPUT].value = noteButtons[liveMono].drift + (noteButtons[liveMono].key - 60) / 12.0f;
        }
    } /// end if output active
    
    outputs[MONOVEL_OUTPUT].value = noteButtons[liveMono].vel / 127.0f * 10.0f;
    bool monoRtgGate = monogate;
    if ((params[MONORETRIG_PARAM].value > 0.5f) && (noteButtons[liveMono].key != lastMono)){///if retrig
        monoRtgGate = !monoPulse.process(engineGetSampleTime());
        if (monoRtgGate) lastMono = noteButtons[liveMono].key;
    }
    
    outputs[MONOGATE_OUTPUT].value = !muteMono && monoRtgGate ? 10.0f : 0.0f;
 //////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
    
    //  Output locked Mono note
    outputs[LOCKEDVEL_OUTPUT].value = noteButtons[lockedMono].vel / 127.0f * 10.0f;
    outputs[LOCKEDPITCH_OUTPUT].value = noteButtons[lockedMono].drift + (noteButtons[lockedMono].key - 60) / 12.0f;
    
    bool lockedRtgGate = lockedgate;
    if ((params[LOCKEDRETRIG_PARAM].value > 0.5f) && (noteButtons[lockedMono].key != lastLocked)) {
        lockedRtgGate = !lockedPulse.process(engineGetSampleTime());
        if (lockedRtgGate) lastLocked = noteButtons[lockedMono].key; ///set new lastLive
    }
    outputs[LOCKEDGATE_OUTPUT].value = !muteLocked && lockedRtgGate ? 10.0f : 0.0f;
    
  //  int steps = int(engineGetSampleRate() / 200);///SMOOTHING TO 50 mS
    ///PITCH WHEEL
//    if (pitchBend.changed) {
//        //    min = 0     >>-5v
//        // center = 8192  >> 0v  ---- (default)
//        //    max = 16383 >> 5v
//        pitchBend.tSmooth.set(outputs[PBEND_OUTPUT].value , (pitchBend.val / 16384.0f * 10.0f) - 5.0f , steps);
//        pitchBend.changed = false;
//    }
//    outputs[PBEND_OUTPUT].value = pitchBend.tSmooth.next();
    pitchFilter.lambda = 100.f * engineGetSampleTime();
    outputs[PBEND_OUTPUT].value = pitchFilter.process(rescale(pitch, 0, 16384, -5.f, 5.f));
//    ///MODULATION
//    if (mod.changed) {
//        mod.tSmooth.set(outputs[MOD_OUTPUT].value, (mod.val / 127.0f * 10.0f), steps);
//        mod.changed = false;
//    }
//    outputs[MOD_OUTPUT].value = mod.tSmooth.next();
    modFilter.lambda = 100.f * engineGetSampleTime();
    outputs[MOD_OUTPUT].value = modFilter.process(rescale(mod, 0, 127, 0.f, 10.f));
//    ///SUSTAIN (no smoothing)
//    outputs[SUSTAIN_OUTPUT].value = sustain.val / 127.0f * 10.0f;
    sustainFilter.lambda = 100.f * engineGetSampleTime();
    outputs[SUSTAIN_OUTPUT].value = sustainFilter.process(rescale(sustain, 0, 127, 0.f, 10.f));
//    ///PRESSURE
//    if (afterTouch.changed) {
//        afterTouch.tSmooth.set(outputs[PRESSURE_OUTPUT].value, (afterTouch.val / 127.0f * 10.0f), steps);
//        afterTouch.changed = false;
//    }
//    outputs[PRESSURE_OUTPUT].value = afterTouch.tSmooth.next();
    pressureFilter.lambda = 100.f * engineGetSampleTime();
    outputs[PRESSURE_OUTPUT].value = pressureFilter.process(rescale(pressure, 0, 127, 0.f, 10.f));
    
    
//////////////////// S E Q U E N C E R ////////////////////////////
    doSequencer(); /////// SEQ //////    /////// SEQ //////    /////// SEQ //////    /////// SEQ //////    /////// SEQ //////
//////////////////// S E Q U E N C E R ////////////////////////////
    
    
    ///// RESET MIDI LIGHT
    if (resetMidiTrigger.process(params[RESETMIDI_PARAM].value)) {
        lights[RESETMIDI_LIGHT].value= 1.0f;
        MidiPanic();
        return;
    }
    if (lights[RESETMIDI_LIGHT].value > 0.0001f){
        lights[RESETMIDI_LIGHT].value -= 0.0001f ; // fade out light
    }

    bool pulseClk = clockPulse.process(1.0f /  engineGetSampleRate());
    outputs[SEQCLOCK_OUTPUT].value = pulseClk ? 10.0f : 0.0f;
    
    bool pulseStart = startPulse.process(1.0f /  engineGetSampleRate());
    outputs[SEQSTART_OUTPUT].value = pulseStart ? 10.0f : 0.0f;
    
    bool pulseStop = stopPulse.process(1.0f /  engineGetSampleRate());
    outputs[SEQSTOP_OUTPUT].value = pulseStop ? 10.0f : 0.0f;
    
    outputs[SEQSTARTSTOP_OUTPUT].value = (pulseStop || pulseStart) ? 10.0f : 0.0f;
    
    ////// TRIGGER BUTTONS....
    if (setSeqTrigger.process(params[SEQPAD_PARAM].value)){
        padSetMode = (padSetMode != SEQ_MODE)? SEQ_MODE:POLY_MODE;
        padSetLearn = false;
        for (int i = 0; i < numPads; i++) noteButtons[i].learn = false;
    }
    if (setLockTrigger.process(params[LOCKPAD_PARAM].value)){
        padSetMode = (padSetMode != LOCKED_MODE) ? LOCKED_MODE:POLY_MODE;
        padSetLearn = false;
        for (int i = 0; i < numPads; i++) noteButtons[i].learn = false;
    }
    if (setPadTrigger.process(params[PADXLOCK_PARAM].value)){
        padSetMode = (padSetMode != XLOCK_MODE)? XLOCK_MODE:POLY_MODE;
        padSetLearn = false;
        for (int i = 0; i < numPads; i++) noteButtons[i].learn = false;
    }
    if (setLearnTrigger.process(params[LEARNPAD_PARAM].value)){
        padSetLearn = !padSetLearn;
        padSetMode = POLY_MODE;
        
        if (!padSetLearn){///turn off learn on buttons
            for (int i = 0; i < numPads; i++) noteButtons[i].learn = false;
        }
    }

    lights[PSEQ_LIGHT].value = (padSetMode==SEQ_MODE) ? 1.0f : 0.0f;
    lights[PLOCK_LIGHT].value = (padSetMode==LOCKED_MODE) ? 1.0f : 0.0f;
    lights[PXLOCK_LIGHT].value = (padSetMode==XLOCK_MODE) ? 1.0f : 0.0f;
    lights[PLEARN_LIGHT].value = padSetLearn ? 1.0f : 0.0f;

    
    if (inputs[ARPMODE_INPUT].active)
        arpegMode = static_cast<int>(minmaxFit(inputs[ARPMODE_INPUT].value * 0.3, 0.0f, 2.0f));
    
    
    if (setArpegTrigger.process(params[ARPEGON_PARAM].value)){
        arpegMode = (arpegMode != 1)? 1:0;
    }
    if (setArcadeTrigger.process(params[ARCADEON_PARAM].value)){
        arpegMode = (arpegMode != 2)? 2:0;
    }

    
    lights[ARPEGON_LIGHT].value = (arpegMode==1) ? 1.0f : 0.0f;
    lights[ARCADEON_LIGHT].value = (arpegMode==2) ? 1.0f : 0.0f;
    
    
    if (muteSeqTrigger.process(params[MUTESEQ_PARAM].value)){
        muteSeq = !muteSeq;
        lights[MUTESEQ_LIGHT].value = (muteSeq) ? 1.0f : 0.0f;
    }
    if (muteMonoTrigger.process(params[MUTEMONO_PARAM].value)){
        muteMono = !muteMono;
        lights[MUTEMONO_LIGHT].value = (muteMono) ? 1.0f : 0.0f;
    }
    if (muteLockedTrigger.process(params[MUTELOCKED_PARAM].value)){
        muteLocked = !muteLocked;
        lights[MUTELOCKED_LIGHT].value = (muteLocked) ? 1.0f : 0.0f;
    }
    if (mutePolyATrigger.process(params[MUTEPOLYA_PARAM].value)){
        mutePoly = !mutePoly;
        lights[MUTEPOLY_LIGHT].value = (mutePoly) ? 1.0f : 0.0f;
    }
    if (mutePolyBTrigger.process(params[MUTEPOLYB_PARAM].value)){
        mutePoly = !mutePoly;
        lights[MUTEPOLY_LIGHT].value = (mutePoly) ? 1.0f : 0.0f;
    }

    
    if (polyTransUpTrigger.process(params[POLYTRANUP_PARAM].value)){
        if (padSetLearn){
            for (int i = 0; i < numPads; i++)
            {
                if ((noteButtons[i].learn) && (noteButtons[i].key<127)) noteButtons[i].key ++;
            }
        }else if (polyTransParam < 48) polyTransParam ++;
    }
    if (polyTransDwnTrigger.process(params[POLYTRANDWN_PARAM].value)){
        if (padSetLearn){
            for (int i = 0; i < numPads; i++)
            {
                if ((noteButtons[i].learn) && (noteButtons[i].key > 0)) noteButtons[i].key --;
            }
        }else if (polyTransParam > -48) polyTransParam --;
    }
        if ((seqTransUpTrigger.process(params[SEQTRANUP_PARAM].value)) && (seqTransParam < 48)) seqTransParam ++;
        if ((seqTransDwnTrigger.process(params[SEQTRANDWN_PARAM].value)) && (seqTransParam > -48)) seqTransParam --;
        
    dispNotenumber = (params[DISPLAYNOTENUM_PARAM].value > 0.5f);
  
    setMainDisplay(clockSource);
}
/////////////////////// * * * ///////////////////////////////////////////////// * * *
//                      * * *         E  N  D      O  F     S  T  E  P          * * *
/////////////////////// * * * ///////////////////////////////////////////////// * * *

void MIDIPolyInterface::doSequencer(){

    /////// SEQ //////    /////// SEQ //////    /////// SEQ //////    /////// SEQ //////    /////// SEQ //////
    /////// SEQ //////    /////// SEQ //////    /////// SEQ //////    /////// SEQ //////    /////// SEQ //////
    
    
   //////// GET RATIO STEP OFFSET with CVs ....
    int updSeqClockRatio = 0;
    if (inputs[SEQRATIO_INPUT].active)
        updSeqClockRatio = static_cast<int>(minmaxFit(inputs[SEQRATIO_INPUT].value + params[SEQCLOCKRATIO_PARAM].value, 0.0f, 11.0f));
    else
        updSeqClockRatio = static_cast<int>(params[SEQCLOCKRATIO_PARAM].value);
    
    if (updSeqClockRatio != seqclockRatio){
        seqclockRatio = updSeqClockRatio;
        syncArpPhase = true;
    }
    int updSeqFirst = 0;
    if (inputs[SEQFIRST_INPUT].active)
        updSeqFirst = static_cast<int>(minmaxFit(inputs[SEQFIRST_INPUT].value * 1.55f + params[SEQFIRST_PARAM].value, 0.0f, 15.0f));
    else
        updSeqFirst = static_cast<int>(params[SEQFIRST_PARAM].value);
    if (updSeqFirst != seqOffset)
    {
        seqOffset = updSeqFirst;
        seqStep = seqOffset;
    }
    if (inputs[SEQSTEPS_INPUT].active) {
    seqSteps = static_cast <int>  (minmaxFit((inputs[SEQSTEPS_INPUT].value * 1.55f + params[SEQSTEPS_PARAM].value), 1.0f,16.0f));
    }else{
    seqSteps = static_cast <int> (params[SEQSTEPS_PARAM].value);
    }
   ///////////////////////////////////////////////
    
    bool seqResetNow = false;
    
    if ((params[SEQGATERUN_PARAM].value > 0.5f) && (inputs[SEQRUN_INPUT].active)) {
        if ((!seqrunning) && (inputs[SEQRUN_INPUT].value > 3.0f)){
            seqrunning = true;
            if (params[SEQRUNRESET_PARAM].value > 0.5f) seqResetNow = true;
        } else  if ((seqrunning) && (inputs[SEQRUN_INPUT].value < 2.0f)){
            seqrunning = false;
        }
    } else { /// trigger selected for run or disconnected gate input...
        if (seqRunTrigger.process(params[SEQRUN_PARAM].value + inputs[SEQRUN_INPUT].value)) {
            if (params[SEQRUNRESET_PARAM].value > 0.5f){// if linked run > reset
                if (!seqrunning){
                    seqrunning = true;
                    seqResetNow = true;
                } else seqrunning = false;
            }else{ // reset not linked...regular toggle
                seqrunning = !seqrunning;
            }
        }
    }
    if (clockSource == 2){
       
        static int MIDIframe = 0;
            if (extBPM && (sampleFrames < (3 *  static_cast<int>(engineGetSampleRate()))))
                sampleFrames ++;
            else////// 3 seconds without a tick......(20 bmp min)
            {
                extBPM = false;
                firstBPM = true;
              //  MIDIframe = 0;
            }
        if (clkMIDItick){
            MIDIframe++;
            if (MIDIframe >= 24)
            {
                extBPM = true;
                MIDIframe = 0;
                getBPM();
            }
            clkMIDItick = false;
        }
        if(!inputs[SEQRUN_INPUT].active){
                if (MIDIstart){
                    seqrunning = true;
                    seqTickCount = 0;
                    arpTickCount = 0;
                    MIDIstart = false;
                    seqResetNow = true;
                }
                if (MIDIcont){
                    seqrunning = true;
                    seqTickCount = 0;
                    arpTickCount = 0;
                    MIDIcont = false;
                }
                if (MIDIstop){
                    seqrunning = false;
                    MIDIstop = false;
                }
        }
    }else if (clockSource == 0) {
       if (extBPM && (inputs[CLOCK_INPUT].active) && (sampleFrames < (3 * static_cast<int>(engineGetSampleRate()))))
            sampleFrames ++;
        else////// 3 seconds without a tick......(20 bmp min)
            {
                extBPM = false;
                firstBPM = true;
            }
            
        if (extClockTrigger.process(inputs[CLOCK_INPUT].value)) {
            ///// EXTERNAL CLOCK
            extBPM = true;
            ClockSeqSamples = static_cast<int>(static_cast<float>(sampleFrames) / ClockRatios[seqclockRatio]+0.5f);
            ClockArpSamples = static_cast<int>(static_cast<float>(sampleFrames) / ClockRatios[arpclockRatio]+0.5f);
            getBPM();
        }
    }
    // Reset manual
    if ((seqResetTrigger.process(params[SEQRESET_PARAM].value)) || ((inputs[SEQRESET_INPUT].active) && (seqResetTrigger.process(inputs[SEQRESET_INPUT].value)))) {
        seqResetLight = 1.0f;
        seqOctIx = 0;
        if (seqrunning) {
            seqResetNext = true;
        }else {
            seqResetNow = true;
        }
    }
    
    
    
    ///
    if (seqResetNow){
        seqPhase = 0.0f;
        seqi = 0;
        seqiWoct = 0;
        seqStep = seqOffset;
        seqResetLight = 1.0f;
        seqOctIx = 0;
        seqSwingDwn = true;
        arpSwingDwn = true;
        arpPhase = 0.0f;
        seqTickCount = 0;
        arpTickCount = 0;
    }
    ////// seq run // // // // // // // // //
    bool nextStep = false;

    if (seqrunning) {
        
        int seqOctaveKnob = static_cast<int> (params[SEQOCT_PARAM].value);
        if (stopped){
            stopped = false;
            startPulse.trigger(1e-3);
            clockPulse.trigger(1e-3);
            seqOctValue = seqOctaveKnob;
            seqSampleCount = 0;
            seqPhase = 0.0f;
            seqSwingDwn = true;
            arpSampleCount = 0;
            arpPhase = 0.0f;
            arpSwingDwn = true;
            seqTickCount = 0;
            arpTickCount = 0;
        }
        float swingknob = params[SEQARPSWING_PARAM].value / 40.0f;
        float seqswingPhase;
        int swingtick;
        int swingMidiPhase;
        
        bool notesFirst = (params[SEQOCTALT_PARAM].value > 0.5f);
        bool DontSwing = true;
        if ((params[SEQSWING_PARAM].value > 0.5f) && ((swingTriplet[seqclockRatio]) || (params[SWINGTRI_PARAM].value > 0.5f))){
        int lastStepByOct = seqSteps * (1 + std::abs(seqOctValue-3));
        ///if steps is Odd Don't swing last step..if Octaves first last Step is seqSteps * Octave cycles....if notes first last step is seqSteps...
        DontSwing = (((seqSteps % 2 == 1) && (notesFirst) && ((seqStep - seqOffset) == (seqSteps - 1))) ||
                            ((lastStepByOct % 2 == 1) && (!notesFirst) && (seqiWoct == (lastStepByOct - 1))));
        }
        if (clockSource < 2) {
            if (clockSource == 1) ClockSeqSamples = static_cast<int>(engineGetSampleRate() * 60.0f/BPMrate / ClockRatios[seqclockRatio] + 0.5f);
 
                seqPhase = static_cast<float>(seqSampleCount)/static_cast<float>(ClockSeqSamples);
                seqSampleCount++;
                
                if (DontSwing) {
                    seqswingPhase = 1.0f;
                }else{
                    if (seqSwingDwn){
                        seqswingPhase = 1.0f + swingknob;
                    }else{
                        seqswingPhase = 1.0f - swingknob;
                    }
                }
                if (seqPhase >= seqswingPhase) {
                    seqPhase = 0.0f;
                    seqSwingDwn = !seqSwingDwn;
                    nextStep = true;
                    seqSampleCount = 0;
                    if (DontSwing) seqSwingDwn = true;
                }
        }else{
            
            swingtick = static_cast<int>(swingknob * (24/ClockRatios[seqclockRatio]));
            if (seqMIDItick){
                seqMIDItick = false;
                seqTickCount ++;
                if (DontSwing) {
                    swingMidiPhase = 24/ClockRatios[seqclockRatio];
                }else{
                    if (seqSwingDwn){
                        swingMidiPhase = 24/ClockRatios[seqclockRatio] + swingtick;
                    }else{
                        swingMidiPhase = 24/ClockRatios[seqclockRatio] - swingtick;
                    }
                }
                if (seqTickCount >= swingMidiPhase){
                    seqTickCount = 0;
                    seqSwingDwn = !seqSwingDwn;
                    nextStep = true;
                    if (DontSwing) seqSwingDwn = true;
                }
            }
            
        }
        
        bool gateOut;
        bool pulseTrig = gatePulse.process(1.0f / engineGetSampleRate());
        gateOut = (noteButtons[seqStep].velseq > 0) && (!(pulseTrig && (params[SEQRETRIG_PARAM].value > 0.5f)));
        //// if note goes out to seq...(if not the outputs hold the last played value)
        if (params[SEQSEND_PARAM + seqStep].value > 0.5f){
            outputs[SEQPITCH_OUTPUT].value = noteButtons[seqStep].drift + octaveShift[seqOctValue][seqOctIx] + (inputs[SEQSHIFT_INPUT].value * params[TRIMSEQSHIFT_PARAM].value /48.0f ) + (seqTransParam + noteButtons[seqStep].key - 60) / 12.0f;
            outputs[SEQVEL_OUTPUT].value = noteButtons[seqStep].velseq / 127.0f * 10.0f;
            outputs[SEQGATE_OUTPUT].value = !muteSeq && gateOut ? 10.0f : 0.0f;
            ///if individual gate / vel
            if (params[SEQSEND_PARAM + seqStep].value > 1.5f){
                noteButtons[seqStep].gateseq = true;
             //   outputs[PITCH_OUTPUT + seqStep].value = noteButtons[seqStep].drift + octaveShift[seqOctValue][seqOctIx] + (seqTransParam + noteButtons[seqStep].key - 60) / 12.0f;
                outputs[VEL_OUTPUT + seqStep].value = noteButtons[seqStep].velseq / 127.0f * 10.0f;
                outputs[GATE_OUTPUT + seqStep].value = !muteSeq && gateOut ? 10.0f : 0.0f;
            }
        }else{
            outputs[SEQGATE_OUTPUT].value = 0.0f;
        }
        
          if (nextStep) {
        // restore gates and vel from individual outputs...
        if (params[SEQSEND_PARAM + seqStep].value > 1.5f){
            noteButtons[seqStep].gateseq = false;
           // outputs[PITCH_OUTPUT + seqStep].value = noteButtons[seqStep].drift + (noteButtons[seqStep].key - 60) / 12.0f;
            outputs[VEL_OUTPUT + seqStep].value = noteButtons[seqStep].vel / 127.0f * 10.0f;
            outputs[GATE_OUTPUT + seqStep].value = noteButtons[seqStep].gate ? 10.0f : 0.0f;
        }
            for (int i = 0 ; i < 5; i++){
                lights[SEQOCT_LIGHT + i].value = 0.0f;
            }
            if ((syncArpPhase) && (seqSwingDwn)) {
                arpSampleCount = 0;
                arpPhase = 0.0f;
                arpSwingDwn = true;
                syncArpPhase = false;
                arpTickCount = 0;
            }

            clockPulse.trigger(1e-3);
            gatePulse.trigger(1e-3);
            ////////////////////////
            
            if (seqResetNext){ ///if reset while running
                seqResetNext = false;
                seqPhase = 0.0f;
                seqi = 0;
                seqiWoct = 0;
                seqStep = seqOffset;
                seqSwingDwn = true;
                arpPhase = 0.0f;
                arpSwingDwn = true;
                seqTickCount = 0;
                arpTickCount = 0;
                seqSampleCount = 0;
                arpSampleCount = 0; /// SYNC THE ARPEGG
  
            }else{
            
            
            seqiWoct ++;
                    if (notesFirst) {
                    ////// ADVANCE STEP ///////
                    seqi ++;
                        if ( seqi > (seqSteps - 1)) { ///seq Cycle ... change octave ?
                                seqOctValue = seqOctaveKnob;
                                        if (octaveShift[seqOctValue][seqOctIx + 1] > 2) {
                                            seqOctIx= 0;
                                            seqiWoct = 0;
                                        }else{
                                            seqOctIx ++;
                                        }
                                seqi = 0;// next cycle
                            }
                   }else{
                       if (octaveShift[seqOctValue][seqOctIx + 1] > 2) {
                           seqOctValue = seqOctaveKnob;
                           seqOctIx = 0;
                           seqi ++;
                           if ( seqi > (seqSteps - 1)) {
                               seqi = 0;// next cycle
                               seqiWoct = 0;
                           }
                           
                       }else{
                           seqOctIx ++;
                       }
                  }
                 seqStep = ((seqi % seqSteps) + seqOffset) % numPads;
            }
         lights[SEQOCT_LIGHT + 2 + octaveShift[seqOctValue][seqOctIx]].value = 1.0f;
        }
    } else { ///stopped shut down gate....
        if (!stopped){
            noteButtons[seqStep].gateseq = false;
            stopped = true;
            stopPulse.trigger(1e-3);
            seqOctIx = 0;
            seqi = 0;
            seqiWoct = 0;
            for (int i = 0 ; i < 5; i++){
                lights[SEQOCT_LIGHT + i].value = 0.0f;
            }
            outputs[SEQGATE_OUTPUT].value = 0.0f;
        }
    }
   
    lights[SEQRUNNING_LIGHT].value = seqrunning ? 1.0f : 0.0f;
    
    if (seqResetLight > 0.0001f) seqResetLight -= 0.0001f;
    lights[SEQRESET_LIGHT].value = seqResetLight;
    
    /////// SEQ ////// ------- E N D  ---------  ------- E N D  ---------  ------- E N D  --------- /////// SEQ //////
    return;
}

///////////BUTTONS' DISPLAY

 struct NoteDisplay : TransparentWidget {
     MIDIPolyInterface::noteButton *nButton;
     NoteDisplay() {
            font = Font::load(FONT_FILE);
       }
     float dfontsize = 12.0f;
     int key;
     int vel;
     bool gate;
     bool gateseq;
     bool newkey;
     int frame = 0;
     int framevel = 0;
     bool showvel = true;
     int *polyIndex;
     int polyi;
     int id;
     int mode;
     bool learn;
     bool *dispNotenumber;
     bool notenumber;
     int *arpIx;
     int arpI;
     bool arp;
         std::shared_ptr<Font> font;
         std::string to_display;
     std::string displayNoteName(int key, bool notenumber);
    void draw(NVGcontext *vg) {
        frame ++;
        if (frame > 5){
            frame = 0;
            newkey = nButton->newkey;
            key =  nButton->key;
            vel = nButton->vel;
            gate = nButton->gate;
            gateseq = nButton->gateseq;
            mode = nButton->mode;
            learn = nButton->learn;
            arpI = *arpIx;///-1 if no arp
            polyi = *polyIndex;
            notenumber = *dispNotenumber;
        }
        int rrr,ggg,bbb,aaa;
        if (learn) {
            rrr=0xff; ggg=0xff; bbb=0x00;
            aaa=128;
        } else { switch (mode)  {
            case 0:{/// Poly
                rrr=0xff; ggg=0xff; bbb=0xff;
                aaa= vel;
            }break;
            case 1:{/// Seq
                rrr=0x00; ggg=0x99; bbb=0xff;
                aaa= 64 + vel * 1.5f;
            }break;
            case 2:{/// Lock
                rrr=0xff; ggg=0x00; bbb=0x00;
                aaa= 64 + vel * 1.5f;
            }break;
            default:{/// xLock
                rrr=0xff; ggg=0x80; bbb=0x00;
                aaa= 64 + vel * 1.5f;
            }break;
            }
        }
        NVGcolor backgroundColor = nvgRGBA(rrr*0.6f, ggg*0.6f, bbb*0.6f, aaa);
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 6.0f);
        nvgFillColor(vg, backgroundColor);
        nvgFill(vg);
        if (newkey) {
            showvel = true;
            framevel = 0;
          //  module->noteButtons[id].newkey = false;
            nButton->newkey = false;
        }else{
            to_display = displayNoteName(key, notenumber);
        }
        if (learn) {
            NVGcolor borderColor = nvgRGB(rrr, ggg, bbb);
            nvgStrokeWidth(vg, 2.0f);
            nvgStrokeColor(vg, borderColor);
            nvgStroke(vg);
        } else if (gate || gateseq){
            if (showvel) {
                       if (++framevel <= 20) {
                        to_display = "v" + std::to_string(vel);
                       } else {
                           showvel = false;
                           framevel = 0;
                       }
                }
            NVGcolor borderColor = nvgRGB(rrr, ggg, bbb);
            nvgStrokeWidth(vg, 2.0f);
            nvgStrokeColor(vg, borderColor);
            nvgStroke(vg);
        }
        if (arpI == id)
        { ///arp led
            NVGcolor ledColor = nvgRGB(0xff, 0xff, 0xff);
            nvgBeginPath(vg);
            nvgRect(vg, 4 , 8, 2, 2);
            nvgFillColor(vg, ledColor);
            nvgFill(vg);
           // nButton->arp = false;
        }
        if (polyi == id)
        { ///led
            NVGcolor ledColor = nvgRGB(0xff, 0xff, 0xff);
            nvgBeginPath(vg);
            nvgRect(vg, 4 , 4, 2, 2);
            nvgFillColor(vg, ledColor);
            nvgFill(vg);
        }
        nvgFontSize(vg, dfontsize);
        nvgFontFaceId(vg, font->handle);
        //nvgTextLetterSpacing(vg, 2.0f);
        NVGcolor textColor = nvgRGB(rrr, ggg, bbb);
        nvgFillColor(vg, textColor);
       // nvgTextLetterSpacing(vg, 3.0f);
        nvgTextAlign(vg, NVG_ALIGN_CENTER);
        //Vec textPos = Vec(47 - 14 * to_display.length(), 18.0f);
        //Vec textPos = Vec(5.0f, 20.0f);
        nvgTextBox(vg, 0.0f, 19.f, 48.0f ,to_display.c_str(), NULL);
    }
};

/////////// MIDI NOTES to Text
std::string NoteDisplay::displayNoteName(int value, bool notenumber)
{
    std::string notename[12] = {"C","C#","D","Eb","E","F","F#","G","Ab","A","Bb","B"};
    if (notenumber){
        return "n"+ std::to_string(value);
    }else{
       int octaveint = (value / 12) - 2;
       return notename[value % 12] + std::to_string(octaveint);
    }
}

///// MAIN DISPLAY //////

struct digiDisplay : TransparentWidget {
    digiDisplay() {
        font = Font::load(FONT_FILE);
    }
    float mdfontSize = 11.f;
    std::shared_ptr<Font> font;
    std::string *line1;
    std::string *line2;
    std::string *line3;
    std::string *voices;
    std::string displayedT[4];
    std::string seqDisplayedTr;
    std::string polyDisplayedTr;
    int *seqtransP;
    int *polytransP;
    int seqtrans = 0;
    int polytrans = 0;
    float thirdlineoff = 0.0f;
    bool thirdline = false;
    int frame = 0;
    void draw(NVGcontext* vg)
    {
        if (frame ++ > 5 )
        {
            polytrans = *polytransP;
            seqtrans = *seqtransP;
            seqDisplayedTr = std::to_string(seqtrans);
            polyDisplayedTr = std::to_string(polytrans);
            displayedT[0] = *line1;
            displayedT[1] = *line2;
            displayedT[2] = *line3;
            thirdline = (displayedT[2] != "");
            displayedT[3] = *voices;
               frame = 0;
       }

        nvgFillColor(vg, nvgRGBA(0xFF,0xFF,0xFF,0xFF));
        nvgFontSize(vg, mdfontSize);
        nvgFontFaceId(vg, font->handle);
        //nvgTextLetterSpacing(vg, 2.0f);
        nvgTextAlign(vg, NVG_ALIGN_CENTER);
        
        if (thirdline) {
            nvgTextBox(vg, 0.0f, 6.f+mdfontSize * 3.f, box.size.x, displayedT[2].c_str(), NULL);
            thirdlineoff = 0.0f;
        } else {thirdlineoff = 6.f;}
        nvgTextBox(vg, 0.0f, 2.f+mdfontSize + thirdlineoff, box.size.x, displayedT[0].c_str(), NULL);
        // nvgFontSize(vg, mdfontSize - 3);
        nvgTextLetterSpacing(vg, -1.0f);
        nvgTextBox(vg, 0.0f, 4.f+mdfontSize * 2.f + thirdlineoff * 1.5f, box.size.x, displayedT[1].c_str(), NULL);
        
        nvgTextLetterSpacing(vg, 0.0f);
        nvgTextBox(vg, 122.0f, -21.0f, 48, displayedT[3].c_str(), NULL);
        nvgTextBox(vg, 262.0f, -21.0f,30.0f, polyDisplayedTr.c_str(), NULL);
        nvgTextBox(vg, -7.0f, 284.0f,30.0f, seqDisplayedTr.c_str(), NULL);
    }

   void setFontSize(float const size)
    {
        mdfontSize = size;
    }
};
struct SelectorKnob : moDllzSelector32 {
    SelectorKnob() {
    minAngle = -0.88*M_PI;
    maxAngle = 1.0f*M_PI;
    }
//    void onAction(EventAction &e) override {
//    }
};
struct RatioKnob : moDllzSmSelector {
    RatioKnob() {
      //  box.size ={36,36};
        minAngle = -0.871*M_PI;
        maxAngle = 1.0*M_PI;
    }
};


struct SelectorOct : moDllzSelector32 {//Oct
    SelectorOct() {
       // box.size ={32,32};
        minAngle = -0.44f*M_PI;
        maxAngle = 0.44f*M_PI;
    }
};
struct Knob26 : moDllzKnob26 {///Unison
    Knob26() {
        //snap = true;
        minAngle = -0.75*M_PI;
        maxAngle = 0.75*M_PI;
    }
};
struct Knob26Snap : moDllzKnob26 {///swing
    Knob26Snap() {
        //box.size ={26,26};
        snap = true;
        minAngle = -0.75*M_PI;
        maxAngle = 0.75*M_PI;
    }
};
struct KnobSnap : moDllzKnobM {//BPM
    KnobSnap() {
        snap = true;
        minAngle = -0.836*M_PI;
        maxAngle = 1.0*M_PI;
    }
};

struct TangerineLight : GrayModuleLightWidget {
    TangerineLight() {
        addBaseColor(nvgRGB(0xff, 0x80, 0x00));
    }
};
struct WhiteYLight : GrayModuleLightWidget {
    WhiteYLight() {
        addBaseColor(nvgRGB(0xee, 0xee, 0x88));
    }
};



struct overMidiDisplayPoly : TransparentWidget {
    
    void draw(NVGcontext* vg)
    {
        NVGcolor backgroundColor = nvgRGBA(0x80, 0x80, 0x80, 0x24);
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, 165, 33, 5.0f);
        nvgFillColor(vg, backgroundColor);
        nvgFill(vg);
        nvgBeginPath(vg);
        NVGcolor linecolor = nvgRGB(0x50, 0x50, 0x50);
        nvgRect(vg, 52, 0, 1, 33);
        nvgFillColor(vg, linecolor);
        nvgFill(vg);
    }
    
};
/////////////////////////////////////////////// WIDGET ///////////////////////////////////////////////

struct MIDIPolyWidget : ModuleWidget
{
    MIDIPolyWidget(MIDIPolyInterface *module): ModuleWidget(module){
        setPanel(SVG::load(assetPlugin(plugin, "res/MIDIPoly.svg")));

    //Screws
	addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(135, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(435, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(585, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(0, 365)));
    addChild(Widget::create<ScrewBlack>(Vec(135, 365)));
	addChild(Widget::create<ScrewBlack>(Vec(435, 365)));
	addChild(Widget::create<ScrewBlack>(Vec(585, 365)));
    
  
    float xPos = 9;//61;
    float yPos = 19;
    
        MidiWidget *midiWidget = Widget::create<MidiWidget>(Vec(xPos,yPos));
        midiWidget->box.size = Vec(165,33);//115, 36);
        midiWidget->midiIO = &module->midiInput;
        
        midiWidget->driverChoice->box.size = Vec(54,16);
        midiWidget->deviceChoice->box.size = Vec(111,16);
        midiWidget->channelChoice->box.size = Vec(111,16);
        
        midiWidget->driverChoice->box.pos = Vec(0, 0);
        midiWidget->deviceChoice->box.pos = Vec(54, 0);
        midiWidget->channelChoice->box.pos = Vec(54, 17);
        
        midiWidget->driverSeparator->box.pos = Vec(0, 16);
        midiWidget->deviceSeparator->box.pos = Vec(0, 16);
        
        midiWidget->driverChoice->font = Font::load(mFONT_FILE);
        midiWidget->deviceChoice->font = Font::load(mFONT_FILE);
        midiWidget->channelChoice->font = Font::load(mFONT_FILE);
        
        midiWidget->driverChoice->textOffset = Vec(2,12);
        midiWidget->deviceChoice->textOffset = Vec(2,12);
        midiWidget->channelChoice->textOffset = Vec(2,12);
        
        midiWidget->driverChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
        midiWidget->deviceChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
        midiWidget->channelChoice->color = nvgRGB(0xdd, 0xdd, 0xdd);
        addChild(midiWidget);
        
        overMidiDisplayPoly *overmididisplaypoly = Widget::create<overMidiDisplayPoly>(Vec(9,19));
        addChild(overmididisplaypoly);
        
        xPos = 10;
        yPos = 37;
    addParam(ParamWidget::create<moDllzMidiPanic>(Vec(xPos, yPos), module, MIDIPolyInterface::RESETMIDI_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos+35.f, yPos+4.f), module, MIDIPolyInterface::RESETMIDI_LIGHT));

    yPos = 20;
    // PolyMode
   
    xPos = 234;
    addParam(ParamWidget::create<moDllzSwitchT>(Vec(xPos,yPos), module, MIDIPolyInterface::POLYMODE_PARAM, 0.0f, 2.0f, 1.0f));
    //Shift
    xPos = 278.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos,yPos-0.5f), Port::INPUT, module, MIDIPolyInterface::POLYSHIFT_INPUT));
    xPos = 303;
    addParam(ParamWidget::create<TTrimSnap>(Vec(xPos,yPos+4), module, MIDIPolyInterface::TRIMPOLYSHIFT_PARAM, 0.0f, 48.0f, 9.6f));
    
    
    // Poly Transp ...
    xPos = 355;
    addParam(ParamWidget::create<moDllzPulseUp>(Vec(xPos,yPos), module, MIDIPolyInterface::POLYTRANUP_PARAM, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<moDllzPulseDwn>(Vec(xPos,yPos+11), module, MIDIPolyInterface::POLYTRANDWN_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<TinyLight<YellowLight>>(Vec(xPos+8, yPos+25), module, MIDIPolyInterface::PLEARN_LIGHT));

    
    //Unison Drift
    xPos = 367.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos, yPos+7.5f), Port::INPUT, module, MIDIPolyInterface::POLYUNISON_INPUT));
    xPos = 395;
    addParam(ParamWidget::create<moDllzKnob32>(Vec(xPos ,yPos), module, MIDIPolyInterface::POLYUNISON_PARAM, 0.0f, 2.0f, 0.0f) );
    xPos = 444;
    addParam(ParamWidget::create<Knob26>(Vec(xPos-1,yPos-2), module, MIDIPolyInterface::DRIFT_PARAM, 0.0f, 0.1f, 0.0f));
    
    //Midi Numbers / notes
    xPos = 476;
    addParam(ParamWidget::create<moDllzSwitch>(Vec(xPos,yPos+10), module, MIDIPolyInterface::DISPLAYNOTENUM_PARAM, 0.0f, 1.0f, 0.0f));
    
    xPos = 509;
    // seq xLock
    yPos = 21;
    addParam(ParamWidget::create<moDllzClearButton>(Vec(xPos, yPos), module, MIDIPolyInterface::SEQPAD_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(xPos+28, yPos + 3), module, MIDIPolyInterface::PSEQ_LIGHT));
    yPos = 37;
    addParam(ParamWidget::create<moDllzClearButton>(Vec(xPos, yPos), module, MIDIPolyInterface::PADXLOCK_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<TangerineLight>>(Vec(xPos+28, yPos + 3), module, MIDIPolyInterface::PXLOCK_LIGHT));
    xPos = 550;
    // Lock Learn
    yPos = 21;
    addParam(ParamWidget::create<moDllzClearButton>(Vec(xPos, yPos), module, MIDIPolyInterface::LEARNPAD_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(xPos+28, yPos + 3), module, MIDIPolyInterface::PLEARN_LIGHT));
    yPos = 37;
    addParam(ParamWidget::create<moDllzClearButton>(Vec(xPos, yPos), module, MIDIPolyInterface::LOCKPAD_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos+28, yPos + 3), module, MIDIPolyInterface::PLOCK_LIGHT));
    
    //////// Note Buttons X 16 ///////////
    yPos = 66;
    xPos = 272;
    for (int i = 0; i < MIDIPolyInterface::numPads; i++)
    {
        addParam(ParamWidget::create<moDllzMoButton>(Vec(xPos, yPos), module, MIDIPolyInterface::KEYBUTTON_PARAM + i, 0.0f, 1.0f, 0.0f));
        {
            NoteDisplay *notedisplay = new NoteDisplay();
            notedisplay->box.pos = Vec(xPos,yPos);
            notedisplay->box.size = Vec(48, 27);
            notedisplay->id = i;
            notedisplay->polyIndex = &(module->polyIndex);
            notedisplay->arpIx = &(module->arpDisplayIx);
            notedisplay->dispNotenumber = &(module->dispNotenumber);
            notedisplay->nButton = &(module->noteButtons[i]);
            addChild(notedisplay);
        }
        addParam(ParamWidget::create<moDllzSwitchLedHT>(Vec(xPos + 54 ,yPos + 11), module, MIDIPolyInterface::SEQSEND_PARAM + i, 0.0f, 2.0f, 0.0f));
        addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(xPos + 64.2, yPos + 4), module, MIDIPolyInterface::SEQ_LIGHT + i));
        addOutput(Port::create<moDllzPortDark>(Vec(xPos + 81, yPos + 3.5f), Port::OUTPUT, module, MIDIPolyInterface::PITCH_OUTPUT + i));
        addOutput(Port::create<moDllzPortDark>(Vec(xPos + 105, yPos + 3.5f), Port::OUTPUT, module, MIDIPolyInterface::VEL_OUTPUT + i));
        addOutput(Port::create<moDllzPortDark>(Vec(xPos + 129, yPos + 3.5f), Port::OUTPUT, module, MIDIPolyInterface::GATE_OUTPUT + i));
       
        yPos += 31;
        if (yPos > 300){
            yPos = 66;
            xPos += 165;
        }
    }
    ////SEQ////
    {
        digiDisplay *mainDisplay = new digiDisplay();
        mainDisplay->box.pos = Vec(63, 57);
        mainDisplay->box.size = {198, 44};
        mainDisplay->line1 = &(module->mainDisplay[0]);//pdisplayString;
        mainDisplay->line2 = &(module->mainDisplay[1]);//pdisplayString;
        mainDisplay->line3 = &(module->mainDisplay[2]);//pdisplayString;
        mainDisplay->voices = &(module->mainDisplay[3]);//pdisplayString;
        mainDisplay->seqtransP = &(module->seqTransParam);
        mainDisplay->polytransP = &(module->polyTransParam);
        addChild(mainDisplay);
    }
   
    //    BPMKnob *clockKnob = dynamic_cast<BPMKnob*>(createParam<BPMKnob>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQSPEED_PARAM, -2.0f, 4.0f, 1.0f));
    //    CenteredLabel* const bpmLabel = new CenteredLabel(10);
    //    bpmLabel->font = Font::load(FONT_FILE);
    //    bpmLabel->box.pos = Vec(10, 56);
    //    bpmLabel->box.size = {60, 20};
    //    bpmLabel->text = "0";
    //    clockKnob->connectLabel(bpmLabel);
    //    addChild(bpmLabel);
    //    addParam(clockKnob);

    yPos = 125;
    //Seq SPEED and Ratio knobs
    xPos = 79;
    addParam(ParamWidget::create<KnobSnap>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQSPEED_PARAM,  20.0f, 240.0f, 120.0f));
    xPos = 146;
    addParam(ParamWidget::create<RatioKnob>(Vec(xPos,yPos+2), module, MIDIPolyInterface::SEQCLOCKRATIO_PARAM,  0.0f, 12.0f, 3.0f));
    
    yPos = 167.5f;
    //Seq SPEED and Ratio inputs
    xPos = 71.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, MIDIPolyInterface::SEQSPEED_INPUT));
    xPos = 134.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, MIDIPolyInterface::SEQRATIO_INPUT));
    
    xPos = 89;
    // STEP FIRST KNOBS
    yPos = 207;
    addParam(ParamWidget::create<SelectorKnob>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQSTEPS_PARAM, 1.0f, 16.0f, 16.0f));
    yPos = 266;
    addParam(ParamWidget::create<SelectorKnob>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQFIRST_PARAM, 0.0f, 15.0f, 0.0f));
    
    xPos = 67.5f;
    // STEP FIRST > > > > > > > >  INPUTS
    yPos = 226.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, MIDIPolyInterface::SEQSTEPS_INPUT));
    yPos = 285.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, MIDIPolyInterface::SEQFIRST_INPUT));

    yPos = 262;
    // Swing..... swing Triplets
    xPos = 134;
    addParam(ParamWidget::create<Knob26Snap>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQARPSWING_PARAM, -20.0, 20.0f, 0.0f));
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(xPos+31,yPos+9), module, MIDIPolyInterface::SEQSWING_PARAM, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(xPos+48,yPos+9), module, MIDIPolyInterface::ARPSWING_PARAM, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<moDllzSwitchLedH>(Vec(xPos+20,yPos+37), module, MIDIPolyInterface::SWINGTRI_PARAM, 0.0f, 1.0f, 0.0f));
    
    
    ///SEQ OCT
    yPos = 210;
    xPos = 148;
    addParam(ParamWidget::create<moDllzSmSelector>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQOCT_PARAM, 0.0f, 6.0f, 3.0f));
    ///ARPEG OCT
    xPos = 213;
    addParam(ParamWidget::create<moDllzSmSelector>(Vec(xPos,yPos), module, MIDIPolyInterface::ARPEGOCT_PARAM, 0.0f, 6.0f, 3.0f));
    
    ///// oct lights
    yPos=248;
    xPos=145;
    for (int i = 0 ; i < 5 ; i++){
        addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(xPos + (10 * i), yPos), module, MIDIPolyInterface::SEQOCT_LIGHT + i));
    }
    
    xPos=211;
    for (int i = 0 ; i < 5 ; i++){
       addChild(ModuleLightWidget::create<TinyLight<WhiteYLight>>(Vec(xPos + (10 * i), yPos), module, MIDIPolyInterface::ARPOCT_LIGHT + i));
    }
    yPos = 197;
    xPos = 156;
    addParam(ParamWidget::create<moDllzSwitchH>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQOCTALT_PARAM, 0.0f, 1.0f, 0.0f));
    
    xPos = 222;
    addParam(ParamWidget::create<moDllzSwitchH>(Vec(xPos,yPos), module, MIDIPolyInterface::ARPEGOCTALT_PARAM, 0.0f, 1.0f, 1.0f));
    
    //ARPEGG ............. RATIO . . . . . . .
    xPos = 215;
    yPos = 127;
    addParam(ParamWidget::create<RatioKnob>(Vec(xPos,yPos), module, MIDIPolyInterface::ARPEGRATIO_PARAM,  0.0f, 12.0f, 6.0f));
    xPos = 203.5f;
    yPos = 167.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::INPUT, module, MIDIPolyInterface::ARPEGRATIO_INPUT));
   
    
    //// ARP ARC INPUT
    xPos = 217.5f;
    yPos = 288.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, MIDIPolyInterface::ARPMODE_INPUT));
    xPos = 240;
    yPos = 259;
    addParam(ParamWidget::create<moDllzRoundButton>(Vec(xPos, yPos), module, MIDIPolyInterface::ARCADEON_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos+3.75f, yPos + 3.75f), module, MIDIPolyInterface::ARCADEON_LIGHT));
    yPos += 18;
    addParam(ParamWidget::create<moDllzRoundButton>(Vec(xPos, yPos), module, MIDIPolyInterface::ARPEGON_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<YellowLight>>(Vec(xPos+3.75f, yPos + 3.75f), module, MIDIPolyInterface::ARPEGON_LIGHT));
    
    ////// LEFT COL
    
    /// Clock IN
    xPos= 9.5f;
    yPos = 58.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, MIDIPolyInterface::CLOCK_INPUT));
    xPos = 18;
    yPos = 84;
    addParam(ParamWidget::create<moDllzSwitchTH>(Vec(xPos, yPos), module, MIDIPolyInterface:: SEQCLOCKSRC_PARAM,  0.0f, 2.0f, 1.0f));
    
    // SEQ START gate
    xPos = 21.5;
    yPos = 118;
    addParam(ParamWidget::create<moDllzSwitchH>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQGATERUN_PARAM, 0.0f, 1.0f, 0.0f));
    xPos = 13;
    yPos += 60;
    //Link Run Reset
    addParam(ParamWidget::create<moDllzSwitchLedH>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQRUNRESET_PARAM, 0.0f, 1.0f, 0.0f));
    
    ///seq Run Reset Inputs
    xPos = 9.5f;
    yPos = 134.5;
    addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, MIDIPolyInterface::SEQRUN_INPUT));
    yPos += 60;
    addInput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::INPUT, module, MIDIPolyInterface::SEQRESET_INPUT));
    ///seq Run Reset BUTTONS
    xPos += 30;
    yPos = 140;
    addParam(ParamWidget::create<moDllzRoundButton>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQRUN_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(xPos+3.75f,yPos+3.75f), module, MIDIPolyInterface::SEQRUNNING_LIGHT));
    yPos += 60;
    addParam(ParamWidget::create<moDllzRoundButton>(Vec(xPos,yPos), module, MIDIPolyInterface::SEQRESET_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(xPos+3.75f,yPos+3.75f), module, MIDIPolyInterface::SEQRESET_LIGHT));
    
    xPos = 9.5f;
    // START STOP CLOCK OUTS
    yPos = 241.5f;
    addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDIPolyInterface::SEQSTART_OUTPUT));
    addOutput(Port::create<moDllzPort>(Vec(xPos+23, yPos), Port::OUTPUT, module, MIDIPolyInterface::SEQSTOP_OUTPUT));
    yPos += 40;
    addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDIPolyInterface::SEQSTARTSTOP_OUTPUT));
    addOutput(Port::create<moDllzPort>(Vec(xPos+23, yPos), Port::OUTPUT, module, MIDIPolyInterface::SEQCLOCK_OUTPUT));
    
    // Transp ...
    yPos = 328;
    xPos = 8.5f;
    addInput(Port::create<moDllzPort>(Vec(xPos, yPos-4.5f), Port::INPUT, module, MIDIPolyInterface::SEQSHIFT_INPUT));
    xPos = 33;
    addParam(ParamWidget::create<TTrimSnap>(Vec(xPos,yPos), module, MIDIPolyInterface::TRIMSEQSHIFT_PARAM, 0.0f, 48.0f, 9.6f));
    xPos = 87;
    addParam(ParamWidget::create<moDllzPulseUp>(Vec(xPos,yPos-3), module, MIDIPolyInterface::SEQTRANUP_PARAM, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<moDllzPulseDwn>(Vec(xPos,yPos+8), module, MIDIPolyInterface::SEQTRANDWN_PARAM, 0.0f, 1.0f, 0.0f));
    
    
    yPos = 327.5;
    // SEQ OUTS
    xPos = 99.5f;
    addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDIPolyInterface::SEQPITCH_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDIPolyInterface::SEQVEL_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDIPolyInterface::SEQGATE_OUTPUT));
    xPos += 27.5f;
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(xPos,yPos+8.5f), module, MIDIPolyInterface::SEQRETRIG_PARAM, 0.0f, 1.0f, 1.0f));
    
    //// MIDI OUTS
    xPos = 202;
    addParam(ParamWidget::create<moDllzSwitchT>(Vec(xPos,yPos-2.5f), module, MIDIPolyInterface::MONOPITCH_PARAM, 0.0f, 2.0f, 1.0f));
    xPos = 234.5f;
    addOutput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::OUTPUT, module, MIDIPolyInterface::MONOPITCH_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::OUTPUT, module, MIDIPolyInterface::MONOVEL_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::OUTPUT, module, MIDIPolyInterface::MONOGATE_OUTPUT));
    xPos += 27.5f;
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(xPos,yPos+8.5f), module, MIDIPolyInterface::MONORETRIG_PARAM, 0.0f, 1.0f, 0.0f));
    xPos += 14.5;
    addOutput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::OUTPUT, module, MIDIPolyInterface::PBEND_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::OUTPUT, module, MIDIPolyInterface::MOD_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::OUTPUT, module, MIDIPolyInterface::PRESSURE_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos,yPos), Port::OUTPUT, module, MIDIPolyInterface::SUSTAIN_OUTPUT));
    xPos += 27.5f;
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(xPos,yPos+8.5f), module, MIDIPolyInterface::HOLD_PARAM, 0.0f, 1.0f, 1.0f));

 
    ///LOCKED OUTS
    xPos= 451;
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(xPos,yPos+8.5f), module, MIDIPolyInterface::PLAYXLOCKED_PARAM, 0.0f, 1.0f, 1.0f));
    xPos += 18;
    addParam(ParamWidget::create<moDllzSwitchT>(Vec(xPos,yPos-2.5f), module, MIDIPolyInterface::LOCKEDPITCH_PARAM, 0.0f, 2.0f, 0.0f));
    xPos = 500.5f;
    addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDIPolyInterface::LOCKEDPITCH_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDIPolyInterface::LOCKEDVEL_OUTPUT));
    xPos += 24;
    addOutput(Port::create<moDllzPort>(Vec(xPos, yPos), Port::OUTPUT, module, MIDIPolyInterface::LOCKEDGATE_OUTPUT));
    xPos += 27.5f;
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(xPos,yPos+10.5f), module, MIDIPolyInterface::LOCKEDRETRIG_PARAM, 0.0f, 1.0f, 0.0f));

///mute buttons

    yPos=318;
    xPos=102;
    addParam(ParamWidget::create<moDllzMuteG>(Vec(xPos,yPos), module, MIDIPolyInterface::MUTESEQ_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos+57.5f, yPos+3), module, MIDIPolyInterface::MUTESEQ_LIGHT));
    xPos=237;
    addParam(ParamWidget::create<moDllzMuteG>(Vec(xPos,yPos), module, MIDIPolyInterface::MUTEMONO_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos+57.5f, yPos+3), module, MIDIPolyInterface::MUTEMONO_LIGHT));
    xPos=503;
    addParam(ParamWidget::create<moDllzMuteG>(Vec(xPos,yPos), module, MIDIPolyInterface::MUTELOCKED_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(xPos+57.5f, yPos+3), module, MIDIPolyInterface::MUTELOCKED_LIGHT));
    yPos=56;
    xPos=401;
    addParam(ParamWidget::create<moDllzMuteGP>(Vec(xPos,yPos), module, MIDIPolyInterface::MUTEPOLYA_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(xPos+20, yPos+4), module, MIDIPolyInterface::MUTEPOLY_LIGHT));
    xPos=566;
    addParam(ParamWidget::create<moDllzMuteGP>(Vec(xPos,yPos), module, MIDIPolyInterface::MUTEPOLYB_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(xPos+20, yPos+4), module, MIDIPolyInterface::MUTEPOLY_LIGHT));

    }
};
//void MIDIPolyWidget::step() {
//    ModuleWidget::step();
//}

} // namespace rack_plugin_moDllz

using namespace rack_plugin_moDllz;

RACK_PLUGIN_MODEL_INIT(moDllz, MIDIPoly) {
   Model *modelMIDIPoly = Model::create<MIDIPolyInterface, MIDIPolyWidget>("moDllz", "MIDIPoly16", "MIDI Poly 16", MIDI_TAG, ARPEGGIATOR_TAG, SEQUENCER_TAG, CONTROLLER_TAG, MULTIPLE_TAG, EXTERNAL_TAG);
   return modelMIDIPoly;
}

