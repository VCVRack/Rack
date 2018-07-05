#ifndef TS_OSC_SEQEUNCER_LISTENER_HPP
#define TS_OSC_SEQEUNCER_LISTENER_HPP

#include <string.h>
#include <stdio.h>
#include "util/common.hpp"
#include "util/math.hpp"

#include "rack.hpp"
using namespace rack;
#include "TSExternalControlMessage.hpp"
#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"

struct TSSequencerModuleBase;


#define CURRENT_EDIT_PATTERN_IX		-1 // Flags that we should use the currentEditingPatternIx
#define CURRENT_EDIT_CHANNEL_IX		-1 // Flags that we should use the currentEditingChannelIx

// Set Play State
// Parameters: int value
#define OSC_SET_PLAY_RUNNINGSTATE	"/play/state"
// Toggle Play/Pause
// Parameters: -NONE-
#define OSC_TOGGLE_PLAY_RUNNINGSTATE	"/play/state/tog"
// Reset
// Parameters: -NONE-
#define OSC_SET_PLAY_RESET	"/play/reset"
// Change Playing Pattern
// Parameters: int pattern
#define OSC_SET_PLAY_PATTERN	"/play/pat"
// Store Play Pattern
// Parameters: int pattern
#define OSC_STORE_PLAY_PATTERN	"/play/pat/sav"
// Set BPM
// Parameters: int bpm
#define OSC_SET_PLAY_BPM	"/play/bpm"
// Add to BPM
// Parameters: int addBPM
#define OSC_ADD_PLAY_BPM	"/play/bpm/add"
// Store BPM
// Parameters: int bpm
#define OSC_STORE_PLAY_BPM	"/play/bpm/sav"
// Set Tempo (0-1)
// Parameters: float tempo
#define OSC_SET_PLAY_TEMPO	"/play/tempo"
// Add to Tempo (0-1)
// Parameters: float addTempo
#define OSC_ADD_PLAY_TEMPO	"/play/tempo/add"
// Set BPM Note
// Parameters: int divisorId
#define OSC_SET_PLAY_BPMNOTE	"/play/bpmnote"
// Add to the BPM Note Index (selection)
// Parameters: int addIx
#define OSC_ADD_PLAY_BPMNOTE	"/play/bpmnote/add"
// Change Step Length
// Parameters: int step
#define OSC_SET_PLAY_LENGTH	"/play/len"
// Store Step Length
// Parameters: int pattern
#define OSC_STORE_PLAY_LENGTH	"/play/len/sav"
// Set Ouput Mode (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT)
// Parameters: int modeId
#define OSC_SET_PLAY_OUTPUTMODE	"/play/omode"
// Change Edit Pattern
// Parameters: int pattern
#define OSC_SET_EDIT_PATTERN	"/edit/pat"
// Change Edit Channel
// Parameters: int channel
#define OSC_SET_EDIT_CHANNEL	"/edit/ch"
// Set the Step Value
// Parameters: int step, float value, (opt) int pattern, (opt) int channel
#define OSC_SET_EDIT_STEP	"/edit/step"
// Set the Step Value (/<step>)
// Parameters: float value
#define OSC_SET_EDIT_STEPVALUE	"/edit/step"
// Toggle Edit Step
// Parameters: int step, (opt) float value, (opt) int pattern, (opt) int channel
#define OSC_TOGGLE_EDIT_STEPVALUE	"/edit/step/tog"
// Set the Step Value (Grid) [touchOSC]
// Parameters: float value
#define OSC_SET_EDIT_GRIDSTEP	"/edit/stepgrid"
// Jump to Step Number (playing)
// Parameters: int step
#define OSC_SET_PLAY_CURRENTSTEP	"/play/step"
// Set Mode (Edit, Performance)
// Parameters: int mode
#define OSC_SET_PLAY_MODE	"/play/mode"
// Toggle Mode (Edit, Performance)
// Parameters: -NONE-
#define OSC_TOGGLE_PLAY_MODE	"/play/mode/tog"
// Copy Channel
// Parameters: (opt) int channel, (opt) int pattern
#define OSC_COPY_EDIT_CHANNEL	"/edit/ch/cpy"
// Copy Pattern
// Parameters: (opt) int pattern
#define OSC_COPY_EDIT_PATTERN	"/edit/pat/cpy"
// Paste
// Parameters: -NONE-
#define OSC_PASTE_EDIT_CLIPBOARD	"/edit/clipboard/pst"
// Randomize Channel Steps (same as Context Menu -> Randomize)
// Parameters: -NONE-
#define OSC_RANDOMIZE_EDIT_STEPVALUE	"/edit/step/rnd"
// Initialize the module (same as Context Menu -> Initialize)
// Parameters: -NONE-
#define OSC_INITIALIZE_EDIT_MODULE	"/edit/module/init"
// Copy Current Channel [touchOSC]
// Parameters: -NONE-
#define OSC_COPYCURRENT_EDIT_CHANNEL	"/edit/ch/cpycurr"
// Copy Current Pattern [touchOSC]
// Parameters: -NONE-
#define OSC_COPYCURRENT_EDIT_PATTERN	"/edit/pat/cpycurr"




//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Listener for OSC incoming messages.
// Currently each module must have its own listener object & slave thread since I'm not 100% sure about the threading in Rack (if we could keep
// one thread alive throughout the deaths of other modules). This way, its easy to clean up (when module dies, it kills its slave listener thread)
// instead of tracking how many modules are still alive and using OSC.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
class TSOSCSequencerListener : public osc::OscPacketListener {
public:
	// Pointer to our sequencer module (so we can dump messages in its queue).
	TSSequencerModuleBase* sequencerModule;
	// OSC namespace to use. Currently, if message doesn't have this namespace, we will ignore it. In future, maybe one listener can feed multiple modules with different namespaces?
	std::string oscNamespace;
	// Instantiate a listener.
	TSOSCSequencerListener();
protected:
	//--------------------------------------------------------------------------------------------------------------------------------------------
	// ProcessMessage()
	// @rxMsg : (IN) The received message from the OSC library.
	// @remoteEndPoint: (IN) The remove end point (sender).
	// Handler for receiving messages from the OSC library. Taken from their example listener.
	// Should create a generic TSExternalControlMessage for our trowaSoft sequencers and dump it in the module instance's queue.
	//--------------------------------------------------------------------------------------------------------------------------------------------
	virtual void ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint) override;
};
#endif