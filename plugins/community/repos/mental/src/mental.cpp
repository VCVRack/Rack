#include "mental.hpp"
#include <math.h>

RACK_PLUGIN_MODEL_DECLARE(mental, MentalSubMixer);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalMults);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalMixer);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalFold);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalClip);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalGates);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalABSwitches);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalQuantiser);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalChord);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalMuxes);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalLogic);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalButtons);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalSums);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalPitchShift);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalClockDivider);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalCartesian);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalPatchMatrix);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalBinaryDecoder);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalSwitch8);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalMux8);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalCounters);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalKnobs);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalGateMaker);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalMasterClock);
//RACK_PLUGIN_MODEL_DECLARE(mental, MentalPatchNotes);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalQuadLFO);
RACK_PLUGIN_MODEL_DECLARE(mental, MentalRadioButtons);

RACK_PLUGIN_INIT(mental) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("https://github.com/Strum/Strums_Mental_VCV_Modules");
   RACK_PLUGIN_INIT_MANUAL("https://github.com/Strum/Strums_Mental_VCV_Modules/wiki/Mental-Modules-WIKI");

   RACK_PLUGIN_MODEL_ADD(mental, MentalSubMixer);
   RACK_PLUGIN_MODEL_ADD(mental, MentalMults);
   RACK_PLUGIN_MODEL_ADD(mental, MentalMixer);
   RACK_PLUGIN_MODEL_ADD(mental, MentalFold);
   RACK_PLUGIN_MODEL_ADD(mental, MentalClip);
   RACK_PLUGIN_MODEL_ADD(mental, MentalGates);
   RACK_PLUGIN_MODEL_ADD(mental, MentalABSwitches);
   RACK_PLUGIN_MODEL_ADD(mental, MentalQuantiser);
   RACK_PLUGIN_MODEL_ADD(mental, MentalChord);
   RACK_PLUGIN_MODEL_ADD(mental, MentalMuxes);
   RACK_PLUGIN_MODEL_ADD(mental, MentalLogic);
   RACK_PLUGIN_MODEL_ADD(mental, MentalButtons);
   RACK_PLUGIN_MODEL_ADD(mental, MentalSums);
   RACK_PLUGIN_MODEL_ADD(mental, MentalPitchShift);
   RACK_PLUGIN_MODEL_ADD(mental, MentalClockDivider);
   RACK_PLUGIN_MODEL_ADD(mental, MentalCartesian);
   RACK_PLUGIN_MODEL_ADD(mental, MentalPatchMatrix);
   RACK_PLUGIN_MODEL_ADD(mental, MentalBinaryDecoder);
   RACK_PLUGIN_MODEL_ADD(mental, MentalSwitch8);
   RACK_PLUGIN_MODEL_ADD(mental, MentalMux8);
   RACK_PLUGIN_MODEL_ADD(mental, MentalCounters);
   RACK_PLUGIN_MODEL_ADD(mental, MentalKnobs);
   RACK_PLUGIN_MODEL_ADD(mental, MentalGateMaker);
   RACK_PLUGIN_MODEL_ADD(mental, MentalMasterClock);
   //RACK_PLUGIN_MODEL_ADD(mental, MentalPatchNotes);
   RACK_PLUGIN_MODEL_ADD(mental, MentalQuadLFO);
   RACK_PLUGIN_MODEL_ADD(mental, MentalRadioButtons);

   /*
     p->addModel(createModel<MentalSubMixerWidget>("mental", "MentalSubMixer", "Mental Sub Mixer", MIXER_TAG, PANNING_TAG));
     p->addModel(createModel<MentalMultsWidget>("mental", "MentalMults", "Mental Mults", DUAL_TAG, MULTIPLE_TAG));
     p->addModel(createModel<MentalMixerWidget>("mental", "MentalMixer", "Mental Mixer", MIXER_TAG, PANNING_TAG));
     p->addModel(createModel<MentalFoldWidget>("mental", "MentalFold", "Mental Wave Folder", WAVESHAPER_TAG));
     p->addModel(createModel<MentalClipWidget>("mental", "MentalClip", "Mental Wave Clipper", DISTORTION_TAG));
     p->addModel(createModel<MentalGatesWidget>("mental", "MentalGates", "Mental Gates", UTILITY_TAG));
     p->addModel(createModel<MentalABSwitchesWidget>("mental", "MentalABSwitches", "Mental A/B Switches", SWITCH_TAG, UTILITY_TAG));
     //p->addModel(createModel<MentalNoiseGateWidget>("mental", "mental", "MentalNoiseGate", "Mental Noise Gate"));
     p->addModel(createModel<MentalQuantiserWidget>("mental", "MentalQuantiser", "Mental Quantiser", QUANTIZER_TAG));
     p->addModel(createModel<MentalChordWidget>("mental", "MentalChord", "Mental Chord", CONTROLLER_TAG));
     p->addModel(createModel<MentalMuxesWidget>("mental", "MentalMuxes", "Mental Muxes", UTILITY_TAG));
     p->addModel(createModel<MentalLogicWidget>("mental", "MentalLogic", "Mental Logic", LOGIC_TAG));
     p->addModel(createModel<MentalButtonsWidget>("mental", "MentalButtons", "Mental Buttons", UTILITY_TAG));
     p->addModel(createModel<MentalSumsWidget>("mental", "MentalSums", "Mental Sums", UTILITY_TAG));
     p->addModel(createModel<MentalPitchShiftWidget>("mental", "MentalPitchShift", "Mental Pitch Shifter", UTILITY_TAG));
     p->addModel(createModel<MentalClockDividerWidget>("mental", "MentalClockDivider", "Mental Clock Divider", UTILITY_TAG));
     p->addModel(createModel<MentalCartesianWidget>("mental", "MentalCartesian", "Mental Cartesian Sequencer", SEQUENCER_TAG));
     p->addModel(createModel<MentalPatchMatrixWidget>("mental", "MentalPatchMatrix", "Mental Patch Matrix", UTILITY_TAG)); 
     p->addModel(createModel<MentalBinaryDecoderWidget>("mental", "MentalBinaryDecoder", "Mental Binary Decoder", UTILITY_TAG));
     p->addModel(createModel<MentalSwitch8Widget>("mental", "MentalSwitch8", "Mental 8 Way Switch", UTILITY_TAG));
     p->addModel(createModel<MentalMux8Widget>("mental", "MentalMux8", "Mental 8 to 1 Mux", UTILITY_TAG));
     p->addModel(createModel<MentalCountersWidget>("mental", "MentalCounters", "Mental Counters", UTILITY_TAG));
     p->addModel(createModel<MentalKnobsWidget>("mental", "MentalKnobs", "Mental Knobs", UTILITY_TAG));
     p->addModel(createModel<MentalGateMakerWidget>("mental", "MentalGateMaker", "Mental Gate Maker", UTILITY_TAG));    
     p->addModel(createModel<MentalMasterClockWidget>("mental", "MentalMasterClock", "MentalMasterClock", CLOCK_TAG));
     p->addModel(createModel<MentalPatchNotesWidget>("mental", "MentalPatchNotes", "MentalPatchNotes", UTILITY_TAG));
     p->addModel(createModel<MentalQuadLFOWidget>("mental", "MentalQuadLFO", "Quad LFO", LFO_TAG, QUAD_TAG, CLOCK_TAG));
     p->addModel(createModel<MentalRadioButtonsWidget>("mental", "MentalRadioButtons", "Radio Buttons", UTILITY_TAG, SWITCH_TAG, CONTROLLER_TAG));*/      
}
