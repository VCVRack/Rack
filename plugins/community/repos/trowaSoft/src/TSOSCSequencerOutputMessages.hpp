#ifndef  TSOSCSEQEUNCEROUTPUTMESSAGES_HPP
#define TSOSCSEQEUNCEROUTPUTMESSAGES_HPP


// Output messages over OSC for trowaSoft Sequencers.
enum SeqOSCOutputMsg {
	// Send Play State (Playing/Paused)
	// /play/state
	// Parameters: int playing
	PlayRunningState,
	// Send Play Toggle [touchOSC] (Playing/Paused)
	// /play/state/tog
	// Parameters: int playing
	PlayToggleRun,
	// Send Internal step clock tick or External clock pulse
	// /clock
	// Parameters: int step
	PlayClock,
	// Sequencer RESET button click
	// /reset
	// Parameters: “bang”
	PlayReset,
	// Send Current Playing Pattern
	// /play/pat
	// Parameters: int pattern
	PlayPattern,
	// Send the Current Stored Play Pattern
	// /play/pat/sav
	// Parameters: int pattern
	PlayPatternSav,
	// Send Current BPM
	// /bpm
	// Parameters: float bpm, int divisorId
	PlayBPM,
	// Send Stored BPM
	// /play/bpm/sav
	// Parameters: int pattern
	PlayBPMSav,
	// Send BPM Divisor
	// /bpmnote
	// Parameters: int divisorId
	PlayBPMNote,
	// Send Step Value
	// /step
	// Parameters: int step, float value
	EditStep,
	// Send Step String (voltSeq: needed for touchOSC). The step number will be part of the address (for touchOSC).
	// /step/lbl/
	// Parameters: string valStr
	EditStepString,
	// Send Step Value (Grid/touchOSC)
	// /edit/stepgrid/<row>/<col>
	// Parameters: float value
	EditTOSC_GridStep,
	// Send Current Length
	// /play/len
	// Parameters: int stepLength
	PlayLength,
	// Send the Current Stored Play Length
	// /play/len/sav
	// Parameters: int stepLength
	PlayLengthSav,
	// Send Current Output Mode (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT)
	// /play/omode
	// Parameters: int modeId
	PlayOutputMode,
	// Send Current Edit Pattern
	// /edit/pat
	// Parameters: int pattern
	EditPattern,
	// Send Current Edit Channel
	// /edit/ch
	// Parameters: int channel
	EditChannel,
	// Send Mode (Edit, Performance)
	// /mode
	// Parameters: int mode
	PlayMode,
	// Send Current Clipboard (copied Pattern/Channel)
	// /edit/clipboard/
	// Parameters: int pattern, int channel
	EditClipboard,
	// Send Current Pattern Copied to clipboard [touchOSC]
	// /edit/pat/cpycurr
	// Parameters: int pattern
	EditPatternCpyCurr,
	// Send Current Channel Copied to clipboard [touchOSC]
	// /edit/ch/cpycurr
	// Parameters: int channel
	EditChannelCpyCurr,
	// Send Current Step LED [touchOSC]
	// /step/led/<step>
	// Parameters: int value
	PlayStepLed,
	// Current Step LED Color [touchOSC]
	// /step/led/{step}/color
	// Parameters: string color
	PlayStepLedColor,
	// Step Color [touchOSC]
	// /edit/step/{step}/color
	// Parameters: string color
	EditStepColor,
	// Step Grid Color [touchOSC]
	// /edit/stepgrid/color
	// Parameters: string color
	EditStepGridColor,
	NUM_OSC_OUTPUT_MSGS
};

// In touch osc, to change color, use same address generally, just append "/color".
#define OSC_TOUCH_OSC_CHANGE_COLOR_FS	"%s/color"


// Send Play State (Playing/Paused) (format string).
// Parameters: int playing
#define OSC_SEND_PLAY_RUNNINGSTATE_FS	"%s/play/state"
// Send Play Toggle [touchOSC] (Playing/Paused) (format string).
// Parameters: int playing
#define OSC_SEND_PLAY_TOGGLERUN_FS	"%s/play/state/tog"
// Send Internal step clock tick or External clock pulse (format string).
// Parameters: int step
#define OSC_SEND_PLAY_CLOCK_FS	"%s/clock"
// Sequencer RESET button click (format string).
// Parameters: “bang”
#define OSC_SEND_PLAY_RESET_FS	"%s/reset"
// Send Current Playing Pattern (format string).
// Parameters: int pattern
#define OSC_SEND_PLAY_PATTERN_FS	"%s/play/pat"
// Send the Current Stored Play Pattern (format string).
// Parameters: int pattern
#define OSC_SEND_PLAY_PATTERN_SAV_FS	"%s/play/pat/sav"
// Send Current BPM (format string).
// Parameters: float bpm, int divisorId
#define OSC_SEND_PLAY_BPM_FS	"%s/play/bpm"
// Send Stored BPM (format string).
// Parameters: int pattern
#define OSC_SEND_PLAY_BPM_SAV_FS	"%s/play/bpm/sav"
// Send BPM Divisor (format string).
// Parameters: int divisorId
#define OSC_SEND_PLAY_BPMNOTE_FS	"%s/bpmnote"
// Send Step Value (format string). The step number will be part of the address (for touchOSC).
// Parameters: int step, float value
#define OSC_SEND_EDIT_STEP_FS	"%s/edit/step/"
// Send Step String (format string). The step number will be part of the address (for touchOSC).
// Parameters: int step, string valStr
#define OSC_SEND_EDIT_STEPSTRING_FS	"%s/edit/step/lbl/"
// Send Step Value (Grid/touchOSC) (format string).
// %s/edit/stepgrid/<row>/<col> : row/col should be appended.
// Parameters: float value
#define OSC_SEND_EDIT_TOSC_GRIDSTEP_FS	"%s/edit/stepgrid/"
// Send Current Length (format string).
// Parameters: int stepLength
#define OSC_SEND_PLAY_LENGTH_FS	"%s/play/len"
// Send the Current Stored Play Length (format string).
// Parameters: int stepLength
#define OSC_SEND_PLAY_LENGTH_SAV_FS	"%s/play/len/sav"
// Send Current Output Mode (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT) (format string).
// Parameters: int modeId
#define OSC_SEND_PLAY_OUTPUTMODE_FS	"%s/play/omode"
// Send Current Edit Pattern (format string).
// Parameters: int pattern
#define OSC_SEND_EDIT_PATTERN_FS	"%s/edit/pat"
// Send Current Edit Channel (format string).
// Parameters: int channel
#define OSC_SEND_EDIT_CHANNEL_FS	"%s/edit/ch"
// Send Mode (Edit, Performance) (format string).
// Parameters: int mode
#define OSC_SEND_PLAY_MODE_FS	"%s/mode"
// Send Current Clipboard (copied Pattern/Channel) (format string).
// Parameters: int pattern, int channel
#define OSC_SEND_EDIT_CLIPBOARD_FS	"%s/edit/clipboard/"
// Send Current Pattern Copied to clipboard [touchOSC] (format string).
// Parameters: int pattern
#define OSC_SEND_EDIT_PATTERN_CPYCURR_FS	"%s/edit/pat/cpycurr"
// Send Current Channel Copied to clipboard [touchOSC] (format string).
// Parameters: int channel
#define OSC_SEND_EDIT_CHANNEL_CPYCURR_FS	"%s/edit/ch/cpycurr"
// Send Current Step LED (format string).
// %s/step/led/<step> : step should be appended.
// Parameters: int value
#define OSC_SEND_PLAY_STEP_LED_FS	"%s/step/led/"
// Current Step LED Color [touchOSC] (format string).
// Parameters: string color
#define OSC_SEND_PLAY_STEP_LEDCOLOR_FS	"%s/step/led/{step}/color"
// Step Color [touchOSC] (format string).
// %s/edit/step/{step}/color
// Parameters: string color
#define OSC_SEND_EDIT_STEP_COLOR_FS	"%s/edit/step/{step}/color"
// Step Grid Color [touchOSC] (format string).
// Parameters: string color
#define OSC_SEND_EDIT_STEPGRID_COLOR_FS	"%s/edit/stepgrid/color"


// Format strings for our output OSC messages for our sequencers.
const char* const TSSeqOSCOutputFormats[] = {  
	OSC_SEND_PLAY_RUNNINGSTATE_FS,
	OSC_SEND_PLAY_TOGGLERUN_FS,
	OSC_SEND_PLAY_CLOCK_FS,
	OSC_SEND_PLAY_RESET_FS,
	OSC_SEND_PLAY_PATTERN_FS,
	OSC_SEND_PLAY_PATTERN_SAV_FS,
	OSC_SEND_PLAY_BPM_FS,
	OSC_SEND_PLAY_BPM_SAV_FS,
	OSC_SEND_PLAY_BPMNOTE_FS,
	OSC_SEND_EDIT_STEP_FS,
	OSC_SEND_EDIT_STEPSTRING_FS,
	OSC_SEND_EDIT_TOSC_GRIDSTEP_FS,
	OSC_SEND_PLAY_LENGTH_FS,
	OSC_SEND_PLAY_LENGTH_SAV_FS,
	OSC_SEND_PLAY_OUTPUTMODE_FS,
	OSC_SEND_EDIT_PATTERN_FS,
	OSC_SEND_EDIT_CHANNEL_FS,
	OSC_SEND_PLAY_MODE_FS,
	OSC_SEND_EDIT_CLIPBOARD_FS,
	OSC_SEND_EDIT_PATTERN_CPYCURR_FS,
	OSC_SEND_EDIT_CHANNEL_CPYCURR_FS,
	OSC_SEND_PLAY_STEP_LED_FS,
	OSC_SEND_PLAY_STEP_LEDCOLOR_FS,
	OSC_SEND_EDIT_STEP_COLOR_FS,
	OSC_SEND_EDIT_STEPGRID_COLOR_FS
};


#endif //  TSOSCSEQEUNCEROUTPUTMESSAGES_HPP
