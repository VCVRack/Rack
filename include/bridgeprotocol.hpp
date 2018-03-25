#pragma once
#include <stdint.h>


namespace rack {


// TODO Change driver and port number to something less common
/** Driver ID in AudioIO and MidiIO */
#define BRIDGE_DRIVER -12512
#define BRIDGE_HOST "127.0.0.1"
#define BRIDGE_PORT 12512
#define BRIDGE_NUM_PORTS 16
/** Number of VST/AU automation parameters */
#define BRIDGE_NUM_PARAMS 16
/** An arbitrary number which prevents connection from other protocols (like WebSockets) and old Bridge versions */
#define BRIDGE_HELLO 0xff00fefd
#define BRIDGE_INPUTS 8
#define BRIDGE_OUTPUTS 8


/** All commands are called from the client and served by the server
send
- uint8_t cmd
*/
enum BridgeCommand {
	NO_COMMAND = 0,
	/** Requests the server to shut down the client */
	QUIT_COMMAND,
	/** Sets the port
	send
	- uint8_t port
	*/
	PORT_SET_COMMAND,
	/** Sends a 3-byte MIDI command
	send
	- uint8_t msg[3]
	*/
	MIDI_MESSAGE_COMMAND,
	/** Sets the audio sample rate
	send
	- uint32_t sampleRate
	*/
	AUDIO_SAMPLE_RATE_SET_COMMAND,
	/** Sends and receives an audio buffer
	send
	- uint32_t frames
	- float input[BRIDGE_INPUTS * frames]
	recv
	- float output[BRIDGE_OUTPUTS * frames]
	*/
	AUDIO_PROCESS_COMMAND,
	NUM_COMMANDS
};


} // namespace rack
