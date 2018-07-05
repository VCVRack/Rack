#ifndef TSEXTERNALCONTROLMESSAGE_HPP
#define TSEXTERNALCONTROLMESSAGE_HPP

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// External (from Rack) Control message.
// Currently this will be for OSC but may be from MIDI or anything else in the future.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSExternalControlMessage {
	// External source of a control message (i.e. OSC).
	// Currently only OSC
	enum MessageSource {
		// OSC: Open Sound Control
		OSC = 1
	};
	// Message Type / Action to do. Currently enumerated for our sequencers.
	enum MessageType {
		// Set Play State
		// /play/state
		// Parameters: int value
		SetPlayRunningState,
		// Toggle Play/Pause
		// /play/state/tog
		// Parameters: -NONE-
		TogglePlayRunningState,
		// Reset
		// /play/reset
		// Parameters: -NONE-
		SetPlayReset,
		// Change Playing Pattern
		// /play/pat
		// Parameters: int pattern
		SetPlayPattern,
		// Store Play Pattern
		// /play/pat/sav
		// Parameters: int pattern
		StorePlayPattern,
		// Set BPM
		// /play/bpm
		// Parameters: int bpm
		SetPlayBPM,
		// Add to BPM
		// /play/bpm/add
		// Parameters: int addBPM
		AddPlayBPM,
		// Store BPM
		// /play/bpm/sav
		// Parameters: int bpm
		StorePlayBPM,
		// Set Tempo (0-1)
		// /play/tempo
		// Parameters: float tempo
		SetPlayTempo,
		// Add to Tempo (0-1)
		// /play/tempo/add
		// Parameters: float addTempo
		AddPlayTempo,
		// Set BPM Note
		// /play/bpmnote
		// Parameters: int divisorId
		SetPlayBPMNote,
		// Add to the BPM Note Index (selection)
		// /play/bpmnote/add
		// Parameters: int addIx
		AddPlayBPMNote,
		// Change Step Length
		// /play/len
		// Parameters: int step
		SetPlayLength,
		// Store Step Length
		// /play/len/sav
		// Parameters: int pattern
		StorePlayLength,
		// Set Ouput Mode (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT)
		// /play/omode
		// Parameters: int modeId
		SetPlayOutputMode,
		// Change Edit Pattern
		// /edit/pat
		// Parameters: int pattern
		SetEditPattern,
		// Change Edit Channel
		// /edit/ch
		// Parameters: int channel
		SetEditChannel,
		// Set the Step Value
		// /edit/step
		// Parameters: int step, float value, (opt) int pattern, (opt) int channel
		SetEditStep,
		// Set the Step Value (/<step>)
		// /edit/step
		// Parameters: float value
		SetEditStepValue,
		// Toggle Edit Step
		// /edit/step/tog
		// Parameters: int step, (opt) float value, (opt) int pattern, (opt) int channel
		ToggleEditStepValue,
		// Jump to Step Number (playing)
		// /play/step
		// Parameters: int step
		SetPlayCurrentStep,
		// Set Mode (Edit, Performance)
		// /play/mode
		// Parameters: int mode
		SetPlayMode,
		// Toggle Mode (Edit, Performance)
		// /play/mode/tog
		// Parameters: -NONE-
		TogglePlayMode,
		// Copy Channel
		// /edit/ch/cpy
		// Parameters: (opt) int channel, (opt) int pattern
		CopyEditChannel,
		// Copy Pattern
		// /edit/pat/cpy
		// Parameters: (opt) int pattern
		CopyEditPattern,
		// Paste
		// /edit/clipboard/pst
		// Parameters: -NONE-
		PasteEditClipboard,
		// Randomize Channel Steps
		// /edit/step/rnd
		// Parameters: -NONE-
		RandomizeEditStepValue,
		// Initialize the module
		// /edit/module/init
		// Parameters: -NONE-
		InitializeEditModule,
		// Total # message types
		NUM_MESSAGE_TYPES
	};
	// The message type / action.
	MessageType messageType;
	// The message source (i.e. OSC).
	MessageSource messageSource;
	// Pattern number 0-63
	int pattern;
	// Channel 0-15
	int channel;
	// Step number 0-(MaxSteps-1)
	int step;
	// The mode (or BPM)
	int mode;
	// The value / mode.
	float val;
};


#endif // !TSEXTERNALCONTROLMESSAGE_HPP

