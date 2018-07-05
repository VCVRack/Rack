#ifndef MODULE_OSCCV_HPP
#define MODULE_OSCCV_HPP

#include "rack.hpp"
using namespace rack;
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSOSCCommunicator.hpp"
#include <thread> // std::thread
#include <mutex>
#include <string>
#include <queue>

#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"

// Model for trowa OSC2CV
extern Model* modelOscCV;

#define TROWA_OSSCV_SHOW_ADV_CH_CONFIG			1 // Flag to showing advanced config or hiding it (while it is not finished)

#define TROWA_OSCCV_DEFAULT_NUM_CHANNELS		8 // Default number of channels
#define TROWA_OSCCV_NUM_PORTS_PER_INPUT			2 // Each input port should have a trigger input and actual value input.
#define TROWA_OSCCV_DEFAULT_NAMESPACE		"trowacv" // Default namespace for this module (should not be the same as the sequencers)
#define TROWA_OSCCV_MAX_VOLTAGE				 10.0 // Max output voltage
#define TROWA_OSCCV_MIN_VOLTAGE				-10.0 // Min output voltage
#define TROWA_OSCCV_VAL_BUFFER_SIZE			  512 // Buffer size for value history
#define TROWA_OSCCV_TRIGGER_ON_V			 10.0 // Trigger on/high output voltage
#define TROWA_OSCCV_TRIGGER_OFF_V			  0.0 // Trigger off/low output voltage
#define TROWA_OSCCV_MIDI_VALUE_MIN_V		     -5 // -5v : Midi Value 0 (C-1)
#define TROWA_OSCCV_MIDI_VALUE_MAX_V		5.58333 // +5.5833v : Midi Value 127
#define TROWA_OSCCV_MIDI_VALUE_MIN		         0 // Midi value 0
#define TROWA_OSCCV_MIDI_VALUE_MAX			   127 // Midi value 127
#define TROWA_OSCCV_DEFAULT_SEND_HZ			   100 // If no trigger input, bang out OSC when val changes this many times per second.
#define TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL		2


// A channel for OSC.
struct TSOSCCVChannel {
	// Base param ids for the channel
	enum BaseParamIds {
		CH_SHOW_CONFIG,
		CH_NUM_PARAMS
	};
	// Path for this channel. Must start with '/'.
	std::string path;
	// The value
	float val = 0.0;
	// The translated value
	float translatedVal = 0.0;
	//// The last value we SENT over OSC (for tracking changes).
	//float lastVal = -20.0;
	//// The last translated value we SENT over OSC (for tracking changes).
	//float lastTranslatedVal = -20.0;
	uint32_t uintVal = 0;
	//uint32_t lastUintVal = 0;
	// Channel number (1-based)
	int channelNum;
	// What our parameter type should be. We can't really translate strings to voltage, so that is not available.
	enum ArgDataType : int {
		OscFloat = 1,
		OscInt = 2,
		OscBool = 3,
		// An OSC Midi message -- Not sure what actually supports this natively.
		OscMidi = 20
	};
	ArgDataType dataType = ArgDataType::OscFloat;
	// Message received.
	//float msgReceived = 0.0;

	// Value buffer
	float valBuffer[TROWA_OSCCV_VAL_BUFFER_SIZE] = { 0.0 };
	// Value buffer current index to insert into.
	int valBuffIx = 0;
	// The frame index.
	int frameIx = 0;

	// Show channel configuration for this channel.
	SchmittTrigger showChannelConfigTrigger;


	/// TODO: Configuration for conversion & use the conversion stuff.
	/// TODO: Eventually allow strings? Basically user would have to enumerate and we should have an index into the array of strings.

	// Min Rack input or output voltage
	float minVoltage = TROWA_OSCCV_MIN_VOLTAGE;
	// Max Rack input or output voltage
	float maxVoltage = TROWA_OSCCV_MAX_VOLTAGE;
	// Min OSC input or output value.
	float minOscVal = 0;
	// Max OSC input or output value.
	float maxOscVal = 127;
	// If we should translate between the min and max values.
	bool convertVals = false;

	std::mutex mutPath;

	TSOSCCVChannel()
	{
		return;
	}

	TSOSCCVChannel(int chNum, std::string path) : TSOSCCVChannel()
	{
		this->channelNum = chNum;
		this->path = path;
		initialize();
		return;
	}

	virtual void initialize() {
		this->convertVals = false;
		this->val = 0.0;
		this->translatedVal = getValOSC2CV();
		this->dataType = ArgDataType::OscFloat;
		// Min Rack input or output voltage
		minVoltage = TROWA_OSCCV_MIDI_VALUE_MIN_V;
		// Max Rack input or output voltage
		maxVoltage = TROWA_OSCCV_MIDI_VALUE_MAX_V;
		// Min OSC input or output value.
		minOscVal = 0;
		// Max OSC input or output value.
		maxOscVal = 127;
		for (int i = 0; i < TROWA_OSCCV_VAL_BUFFER_SIZE; i++)
		{
			valBuffer[i] = 0.0f;
		}
		valBuffIx = 0;
		convertVals = false;
		return;
	} // end initialize()


	// Get the value translated from OSC to CV voltage.
	float getValOSC2CV() {
		float tVal = val;
		if (convertVals) {
			tVal = rescale(val, minOscVal, maxOscVal, minVoltage, maxVoltage);
		}
		return tVal;
	}
	// Get the value translated from CV voltage to OSC value.
	float getValCV2OSC() {
		float tVal = val;
		if (convertVals) {
			tVal = rescale(val, minVoltage, maxVoltage, minOscVal, maxOscVal);
			switch (this->dataType)
			{
			case TSOSCCVChannel::ArgDataType::OscInt:
				tVal = static_cast<float>(static_cast<int>(tVal));
				break;
			case TSOSCCVChannel::ArgDataType::OscBool:
				tVal = static_cast<float>(static_cast<bool>(tVal));
				break;
			case TSOSCCVChannel::ArgDataType::OscFloat:
			default:
				break;
			}
		}
		return tVal;
	}
	void setOSCInValue(float oscVal) {
		val = oscVal;
		translatedVal = getValOSC2CV();
		return;
	}
	void addValToBuffer(float buffVal);

	void setValue(float newVal) {
		val = newVal;
		if (convertVals)
			translatedVal = getValCV2OSC();
		addValToBuffer(newVal);
		return;
	}
	void setPath(std::string path)
	{
		std::lock_guard<std::mutex> lock(mutPath);			
		if (path[0] != '/')
			this->path = "/" + path;
		else
			this->path = path;
		return;
	}
	std::string getPath() {
		std::lock_guard<std::mutex> lock(mutPath);
		return path;
	}

	//--------------------------------------------------------
	// serialize()
	// @returns : The channel json node.
	//--------------------------------------------------------
	virtual json_t* serialize();
	//--------------------------------------------------------
	// deserialize()
	// @rootJ : (IN) The channel json node.
	//--------------------------------------------------------
	virtual void deserialize(json_t* rootJ);

};
// Channel specifically for CV Input -> OSC.
// Extra stuff for knowing when to send output.
struct TSOSCCVInputChannel : TSOSCCVChannel {
	// The last value we SENT over OSC (for tracking changes).
	float lastVal = -20.0;
	// The last translated value we SENT over OSC (for tracking changes).
	float lastTranslatedVal = -20.0;
	// If trigger is not set up (input type channel), how much input change is needed to send a message out.
	float channelSensitivity = 0.05f;
	// If we should send. Working value for module.
	bool doSend = false;

	TSOSCCVInputChannel() : TSOSCCVChannel()
	{
		return;
	}

	TSOSCCVInputChannel(int chNum, std::string path)
	{
		this->channelNum = chNum;
		this->path = path;
		this->initialize();
		return;
	}
	void initialize() override {
		this->lastVal = -20.0;
		this->lastTranslatedVal = -20.0;
		channelSensitivity = 0.05f;
		TSOSCCVChannel::initialize();
		doSend = false;
		return;
	} // end initialize()

	//--------------------------------------------------------
	// serialize()
	// @returns : The channel json node.
	//--------------------------------------------------------
	json_t* serialize() override;
	//--------------------------------------------------------
	// deserialize()
	// @rootJ : (IN) The channel json node.
	//--------------------------------------------------------
	void deserialize(json_t* rootJ) override;
};

struct TSOSCCVSimpleMessage {
	// Channel Number (1-N)
	int channelNum;
	float rxVal;
	uint32_t uintRxVal;

	TSOSCCVSimpleMessage(int chNum, float recvVal)
	{
		channelNum = chNum;
		rxVal = recvVal;
	}
	TSOSCCVSimpleMessage(int chNum, float recvVal, uint32_t uintVal)
	{
		channelNum = chNum;
		rxVal = recvVal;
		uintRxVal = uintVal;
	}

};


class TSOSCCVSimpleMsgListener;

//===============================================================================
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCV
// OSC <=> CV (Open Sound Control <=> Control Voltage)
// Generic input port -> osc message (on change or on trigger)
// Generic osc message -> output port (on receive)
// Received messages can only have 1 argument (or only 1 will be parsed and used anyway).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
//===============================================================================
struct oscCV : Module {
	// User control parameters
	enum ParamIds {
		OSC_SAVE_CONF_PARAM, // ENABLE and Save the configuration for OSC
		OSC_DISABLE_PARAM,   // Disable OSC (ignore config values)
		OSC_SHOW_CONF_PARAM, // Configure OSC toggle
		OSC_SHOW_ADV_CONF_PARAM, // [TBI] Advanced configuration
		OSC_ADV_CONF_NEXT_PARAM, // [TBI] Advanced configuration Next >
		OSC_ADV_CONF_PREV_PARAM, // [TBI] Advanced configuration < Prev
		OSC_ADV_CONF_BACK_PARAM, // [TBI] Advanced configuration Back
		OSC_AUTO_RECONNECT_PARAM, // Automatically reconnect (if connection is active as of save) on re-load.
		OSC_CH_SAVE_PARAM, // Channel: Save changes and go back to main config. 
		OSC_CH_CANCEL_PARAM, // Channel: Save changes and go back to main config. 
		OSC_CH_OSC_DATATYPE_PARAM, // Channel: OSC data type.
		OSC_CH_TRANSLATE_VALS_PARAM, // Channel: Flag to translate/massage values
		OSC_CH_MIN_CV_VOLT_PARAM, // Channel: Minimum CV Voltage
		OSC_CH_MAX_CV_VOLT_PARAM, // Channel: Maximum CV Voltage
		OSC_CH_MIN_OSC_VAL_PARAM, // Channel: Minimum OSC Value
		OSC_CH_MAX_OSC_VAL_PARAM, // Channel: Maximum OSC Value
		OSC_CH_SEND_FREQ_PARAM, // [TBI] Channel [INPUT->OSC only]: Send frequency (if trigger not active)
		OSC_CH_SEND_THRESHOLD_PARAM, // [TBI] Channell [INPUT->OSC only]: CV value change needed to trigger send (if trigger not active)
		CH_PARAM_START,
		NUM_PARAMS = CH_PARAM_START // Add #channels * 2 to this
	};
	enum InputIds {
		CH_INPUT_START,
		NUM_INPUTS = CH_INPUT_START // Add # channels *2 to this
	};
	enum OutputIds {
		CH_OUTPUT_START,
		NUM_OUTPUTS = CH_OUTPUT_START // Add # channels*2 to this Determined by # of channels
	};
	enum LightIds {
		OSC_CONFIGURE_LIGHT, // The light for configuring OSC.
		OSC_ENABLED_LIGHT, // Light for OSC enabled and currently running/active.
		OSC_CH_TRANSLATE_LIGHT, // Light for Channel Translate enabled.
		CH_LIGHT_START,
		NUM_LIGHTS = CH_LIGHT_START // Add # channels *2 to this
	};

	// Flag for doing Input Rack CV -> OSC. 
	bool doCVPort2OSC = true;
	// Flag for Input OSC -> Rack CV
	bool doOSC2CVPort = true;
	// Number of channels we have
	int numberChannels = TROWA_OSCCV_DEFAULT_NUM_CHANNELS;
	// Input CV (from Rack) ==> Needs to be output to OSC
	TSOSCCVInputChannel* inputChannels = NULL;
	// Input OSC (from External) ==> Needs to be translated to Rack output port CV
	TSOSCCVChannel* outputChannels = NULL;
	PulseGenerator* pulseGens = NULL;
	// The received messages.
	std::queue<TSOSCCVSimpleMessage> rxMsgQueue;
	SchmittTrigger* inputTriggers;

	int oscId;
	/// TODO: OSC members should be dumped into an OSC base class....
	// Mutex for osc messaging.
	std::mutex oscMutex;
	// Current OSC IP address and port settings.
	TSOSCConnectionInfo currentOSCSettings = { OSC_ADDRESS_DEF,  OSC_OUTPORT_DEF , OSC_INPORT_DEF };
	// OSC Configure trigger
	SchmittTrigger oscConfigTrigger;
	SchmittTrigger oscConnectTrigger;
	// Show the OSC configuration screen or not.
	bool oscShowConfigurationScreen = false;

	float sendDt = 0.0f;
	int sendFrequency_Hz = TROWA_OSCCV_DEFAULT_SEND_HZ;

	// Flag to reconnect at load. IFF true and oscInitialized is also true.
	bool oscReconnectAtLoad = false;
	// Flag if OSC objects have been initialized
	bool oscInitialized = false;
	// If there is an osc error.
	bool oscError = false;
	// OSC output buffer.
	char* oscBuffer = NULL;
	// OSC namespace to use. Without the '/'.
	std::string oscNamespace = TROWA_OSCCV_DEFAULT_NAMESPACE;
	// Sending OSC socket
	UdpTransmitSocket* oscTxSocket = NULL;
	// OSC message listener
	TSOSCCVSimpleMsgListener* oscListener = NULL;
	// Receiving OSC socket
	UdpListeningReceiveSocket* oscRxSocket = NULL;
	// The OSC listener thread
	std::thread oscListenerThread;
	// Prev step that was last turned off (when going to a new step).
	int oscLastPrevStepUpdated = TROWA_INDEX_UNDEFINED;
	// Settings for new OSC.
	TSOSCInfo oscNewSettings = { OSC_ADDRESS_DEF,  OSC_OUTPORT_DEF , OSC_INPORT_DEF };
	// OSC Mode action (i.e. Enable, Disable)
	enum OSCAction {
		None,
		Disable,
		Enable
	};
	// Flag for our module to either enable or disable osc.
	OSCAction oscCurrentAction = OSCAction::None;
	// If this has it controls configured.
	bool isInitialized = false;
	const float lightLambda = 0.005f;

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// oscCV()
	// Create a module with numChannels.
	// @numChannels: (IN) Number of input and output 'channels'. Each channel has two CV's.
	// @cv2osc: (IN) True to do CV to OSC out.
	// @osc2cv: (IN) True to do OSC to CV out.
	// At least one of those flags should be true.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	oscCV(int numChannels, bool cv2osc, bool osc2cv);

	oscCV() : oscCV(TROWA_OSCCV_DEFAULT_NUM_CHANNELS, true, true) {
		return;
	}
	~oscCV();

	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// initializeChannels(void)
	// Set channels to default values.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void initialChannels() {
		for (int i = 0; i < numberChannels; i++)
		{
			if (doCVPort2OSC) {
				inputChannels[i].channelNum = i + 1;
				inputChannels[i].path = "/ch/" + std::to_string(i + 1);
				inputChannels[i].initialize();
			}
			if (doOSC2CVPort) {
				outputChannels[i].channelNum = i + 1;
				outputChannels[i].path = "/ch/" + std::to_string(i + 1);
				outputChannels[i].initialize();
			}
		}
		return;
	}
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// initOSC()
	// @ipAddres : (IN) The ip address of the OSC client / server.
	// @outputPort : (IN) The Tx port.
	// @inputPort : (IN) The Rx port.
	// Initialize OSC on the given ip and ports.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void initOSC(const char* ipAddress, int outputPort, int inputPort);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// Clean up OSC.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void cleanupOSC();
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// setOscNamespace()
	// @oscNamespace : (IN) The namespace (without /).
	// Set the OSC namespace (thread safe-ish).
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void setOscNamespace(std::string oscNamespace);
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// step(void)
	// Process.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void step() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// reset(void)
	// Initialize values.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	void reset() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// toJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	json_t *toJson() override;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// fromJson(void)
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
	void fromJson(json_t *rootJ) override;

};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Listener for OSC incoming messages.
// Currently each module must have its own listener object & slave thread since I'm not 100% sure about the threading in Rack (if we could keep
// one thread alive throughout the deaths of other modules). This way, its easy to clean up (when module dies, it kills its slave listener thread)
// instead of tracking how many modules are still alive and using OSC.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
class TSOSCCVSimpleMsgListener : public osc::OscPacketListener {
public:
	// Pointer to OSC module with a message queue to dump into.
	oscCV * oscModule;
	// OSC namespace to use. Currently, if message doesn't have this namespace, we will ignore it. In future, maybe one listener can feed multiple modules with different namespaces?
	std::string oscNamespace;
	// Instantiate a listener.
	TSOSCCVSimpleMsgListener();
	// Instantiate a listener.
	TSOSCCVSimpleMsgListener(std::string oscNs, oscCV* oscModule)
	{
		this->oscModule = oscModule;
		if (oscNs.length() > 0 && oscNs.at(0) != '/')
			this->oscNamespace = "/" + oscNs;
		else
			this->oscNamespace = oscNs;
	}
	void setNamespace(std::string oscNs)
	{
		//debug("Listener.setNamespace(): %s, first char is %c.", oscNs.c_str(), oscNs.at(0));
		std::lock_guard<std::mutex> lock(mutExNamespace);
		if (oscNs.length() > 0 && oscNs.at(0) != '/')
			this->oscNamespace = "/" + oscNs;
		else
			this->oscNamespace = oscNs;
		return;
	}
protected:
	// Mutex for setting the namespace.
	std::mutex mutExNamespace;

	//--------------------------------------------------------------------------------------------------------------------------------------------
	// ProcessMessage()
	// @rxMsg : (IN) The received message from the OSC library.
	// @remoteEndPoint: (IN) The remove end point (sender).
	// Handler for receiving messages from the OSC library. Taken from their example listener.
	// Should create a generic TSExternalControlMessage for our trowaSoft sequencers and dump it in the module instance's queue.
	//--------------------------------------------------------------------------------------------------------------------------------------------
	virtual void ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint) override;
};


#endif // !MODULE_OSCCV_HPP
