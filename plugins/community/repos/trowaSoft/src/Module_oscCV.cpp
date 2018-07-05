#include "Module_oscCV.hpp"
#include "rack.hpp"
using namespace rack;
#include "TSOSCCommunicator.hpp"
#include "Widget_oscCV.hpp"
#include <cmath>  


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCV()
// Create a module with numChannels.
// @numChannels: (IN) Number of input and output 'channels'. Each channel has two CV's.
// @cv2osc: (IN) True to do CV to OSC out.
// @osc2cv: (IN) True to do OSC to CV out.
// At least one of those flags should be true.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCV::oscCV(int numChannels, bool cv2osc, bool osc2cv) : Module(NUM_PARAMS + numChannels*2, NUM_INPUTS + numChannels*2, NUM_OUTPUTS + numChannels * 2, NUM_LIGHTS + numChannels * 2)
{
	oscInitialized = false;
	oscId = TSOSCConnector::GetId();
	this->doOSC2CVPort = osc2cv;
	this->doCVPort2OSC = cv2osc;

	this->numberChannels = numChannels;
	if (doCVPort2OSC)
	{
		inputTriggers = new SchmittTrigger[numberChannels];
		inputChannels = new TSOSCCVInputChannel[numberChannels];
	}
	if (doOSC2CVPort)
	{
		outputChannels = new TSOSCCVChannel[numberChannels];
		pulseGens = new PulseGenerator[numberChannels];
	}

	initialChannels();
	return;
} // end constructor
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up or ram.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCV::~oscCV()
{
	oscInitialized = false;

	cleanupOSC();
	if (oscBuffer != NULL)
	{
		free(oscBuffer);
		oscBuffer = NULL;
	}
	if (oscListener != NULL)
	{
		delete oscListener;
		oscListener = NULL;
	}
	if (inputChannels != NULL)
		delete[] inputChannels;
	if (outputChannels != NULL)
		delete[] outputChannels;
	if (pulseGens != NULL)
		delete[] pulseGens;
	if (inputTriggers != NULL)
		delete[] inputTriggers;
	return;
} // end destructor
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset(void)
// Initialize values.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::reset() {
	// Stop OSC while we reset the values
	cleanupOSC();

	setOscNamespace(TROWA_OSCCV_DEFAULT_NAMESPACE); // Default namespace.
	this->oscReconnectAtLoad = false;

	// Reset our values
	oscMutex.lock();
	initialChannels();
	this->currentOSCSettings.oscTxIpAddress = OSC_ADDRESS_DEF;
	this->currentOSCSettings.oscTxPort = OSC_OUTPORT_DEF;
	this->currentOSCSettings.oscRxPort = OSC_INPORT_DEF;
	oscMutex.unlock();

	this->oscShowConfigurationScreen = false;

	sendDt = 0.0f;
	sendFrequency_Hz = TROWA_OSCCV_DEFAULT_SEND_HZ;
	return;
} // end reset()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// initOSC()
// @ipAddres : (IN) The ip address of the OSC client / server.
// @outputPort : (IN) The Tx port.
// @inputPort : (IN) The Rx port.
// Initialize OSC on the given ip and ports.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::initOSC(const char* ipAddress, int outputPort, int inputPort)
{
	oscMutex.lock();
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	debug("oscCV::initOSC() - Initializing OSC");
#endif
	try
	{
		bool portsRegistered = false;
		// Try to register these ports:
		if (!doCVPort2OSC) {
			// No output
			portsRegistered = TSOSCConnector::RegisterPort(oscId, inputPort);
		}
		else if (!doOSC2CVPort) {
			// No OSC Input
			portsRegistered = TSOSCConnector::RegisterPort(oscId, outputPort);
		}
		else {
			portsRegistered = TSOSCConnector::RegisterPorts(oscId, outputPort, inputPort);
		}

		if (portsRegistered)
		{
			oscError = false;
			this->currentOSCSettings.oscTxIpAddress = ipAddress;
			if (oscBuffer == NULL)
			{
				oscBuffer = (char*)malloc(OSC_OUTPUT_BUFFER_SIZE * sizeof(char));
			}
			if (doCVPort2OSC) {
				// CV Port -> OSC Tx
				if (oscTxSocket == NULL)
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
					debug("oscCV::initOSC() - Create TRANS socket at %s, port %d.", ipAddress, outputPort);
#endif
					oscTxSocket = new UdpTransmitSocket(IpEndpointName(ipAddress, outputPort));
					this->currentOSCSettings.oscTxPort = outputPort;
				}
			}
			if (doOSC2CVPort) {
				// OSC Rx -> CV Port
				if (oscRxSocket == NULL)
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
					debug("oscCV::initOSC() - Create RECV socket at any address, port %d. Osc Namespace %s.", inputPort, oscNamespace.c_str());
#endif
					if (oscListener == NULL)
						oscListener = new TSOSCCVSimpleMsgListener(this->oscNamespace, this);
					else
						oscListener->setNamespace(this->oscNamespace);
					oscRxSocket = new UdpListeningReceiveSocket(IpEndpointName(IpEndpointName::ANY_ADDRESS, inputPort), oscListener);
					this->currentOSCSettings.oscRxPort = inputPort;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
					debug("oscCV::initOSC() - Starting listener thread...");
#endif
					oscListenerThread = std::thread(&UdpListeningReceiveSocket::Run, oscRxSocket);
				}
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("oscCV::initOSC() - OSC Initialized");
#endif
			oscInitialized = true;
		}
		else
		{
			oscError = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("oscCV::initOSC() - Ports in use already.");
#endif
		}
	}
	catch (const std::exception& ex)
	{
		oscError = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		warn("oscCV::initOSC() - Error initializing: %s.", ex.what());
#endif
	}
	oscMutex.unlock();
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up OSC.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::cleanupOSC() 
{
	oscMutex.lock();
	try
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("oscCV::cleanupOSC() - Cleaning up OSC");
#endif
		oscInitialized = false;
		oscError = false;


		TSOSCConnector::ClearPorts(oscId, currentOSCSettings.oscTxPort, currentOSCSettings.oscRxPort);
		if (oscRxSocket != NULL)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("oscCV::cleanupOSC() - Cleaning up RECV socket.");
#endif
			oscRxSocket->AsynchronousBreak();
			oscListenerThread.join(); // Wait for him to finish
			delete oscRxSocket;
			oscRxSocket = NULL;
			//delete oscListener;
			//oscListener = NULL;
		}
		if (oscTxSocket != NULL)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("oscCV::cleanupOSC() - Cleanup TRANS socket.");
#endif
			delete oscTxSocket;
			oscTxSocket = NULL;
		}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("oscCV::cleanupOSC() - OSC cleaned");
#endif
	}
	catch (const std::exception& ex)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("oscCV::cleanupOSC() - Exception caught:\n%s", ex.what());
#endif
	}
	oscMutex.unlock();
} // end cleanupOSC()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// toJson(void)
// Serialize to json.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
json_t *oscCV::toJson() {
	json_t* rootJ = json_object();

	// version
	json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));

	// OSC Parameters
	json_t* oscJ = json_object();
	json_object_set_new(oscJ, "IpAddress", json_string(this->currentOSCSettings.oscTxIpAddress.c_str()));
	json_object_set_new(oscJ, "TxPort", json_integer(this->currentOSCSettings.oscTxPort));
	json_object_set_new(oscJ, "RxPort", json_integer(this->currentOSCSettings.oscRxPort));
	json_object_set_new(oscJ, "Namespace", json_string(this->oscNamespace.c_str()));
	json_object_set_new(oscJ, "AutoReconnectAtLoad", json_boolean(oscReconnectAtLoad)); // [v11, v0.6.3]
	json_object_set_new(oscJ, "Initialized", json_boolean(oscInitialized)); // [v11, v0.6.3] We know the settings are good at least at the time of save
	json_object_set_new(rootJ, "osc", oscJ);

	// Channels
	json_object_set_new(rootJ, "numCh", json_integer(numberChannels));
	json_t* inputChannelsJ = json_array();
	json_t* outputChannelsJ = json_array();
	for (int c = 0; c < numberChannels; c++)
	{
		// Input
		json_array_append_new(inputChannelsJ, inputChannels[c].serialize());
		json_array_append_new(outputChannelsJ, outputChannels[c].serialize());
	}
	json_object_set_new(rootJ, "inputChannels", inputChannelsJ);
	json_object_set_new(rootJ, "outputChannels", outputChannelsJ);

	return rootJ;
} // end toJson()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// fromJson(void)
// Deserialize.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void oscCV::fromJson(json_t *rootJ) {
	json_t* currJ = NULL;
	bool autoReconnect = false;
	// OSC Parameters
	json_t* oscJ = json_object_get(rootJ, "osc");
	if (oscJ)
	{
		currJ = json_object_get(oscJ, "IpAddress");
		if (currJ)
			this->currentOSCSettings.oscTxIpAddress = json_string_value(currJ);
		currJ = json_object_get(oscJ, "TxPort");
		if (currJ)
			this->currentOSCSettings.oscTxPort = (uint16_t)(json_integer_value(currJ));
		currJ = json_object_get(oscJ, "RxPort");
		if (currJ)
			this->currentOSCSettings.oscRxPort = (uint16_t)(json_integer_value(currJ));
		currJ = json_object_get(oscJ, "Namespace");
		if (currJ)
			setOscNamespace( json_string_value(currJ) );
		currJ = json_object_get(oscJ, "AutoReconnectAtLoad");
		if (currJ)
			oscReconnectAtLoad = json_boolean_value(currJ);
		if (oscReconnectAtLoad)
		{
			currJ = json_object_get(oscJ, "Initialized");
			autoReconnect = currJ && json_boolean_value(currJ);
		}
	} // end if OSC node

	// Channels
	int nChannels = numberChannels;
	currJ = json_object_get(rootJ, "numCh");
	if (currJ)
	{
		nChannels = json_integer_value(currJ);
		if (nChannels > numberChannels)
			nChannels = numberChannels;
	}
	json_t* inputChannelsJ = json_object_get(rootJ, "inputChannels");
	json_t* outputChannelsJ = json_object_get(rootJ, "outputChannels");
	for (int c = 0; c < nChannels; c++)
	{
		// Input
		if (inputChannelsJ)
		{
			json_t* channelJ = json_array_get(inputChannelsJ, c);
			if (channelJ) {
				inputChannels[c].deserialize(channelJ);
			} // end if channel object
		} // end if there is an inputChannels array
		// Output
		if (outputChannelsJ)
		{
			json_t* channelJ = json_array_get(outputChannelsJ, c);
			if (channelJ) {
				outputChannels[c].deserialize(channelJ);
			} // end if channel object
		} // end if there is an outputChannels array
	} // end loop through channels

	if (autoReconnect)
	{
		// Try to reconnect
		cleanupOSC();
		this->initOSC(this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);

		if (oscError || !oscInitialized)
		{
			warn("oscCV::fromJson(): Error on auto-reconnect OSC %s :%d :%d.", this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);
		}
		else
		{
			info("oscCV::fromJson(): Successful auto-reconnection of OSC %s :%d :%d.", this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);
		}
	}
	return;
} // end fromJson() 

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step(void)
// Process.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::step()
{
	//bool oscStarted = false; // If OSC just started to a new address this step.
	switch (this->oscCurrentAction)
	{
	case OSCAction::Disable:
		this->cleanupOSC(); // Try to clean up OSC
		break;
	case OSCAction::Enable:
		this->cleanupOSC(); // Try to clean up OSC if we already have something
		this->initOSC(this->oscNewSettings.oscTxIpAddress.c_str(), this->oscNewSettings.oscTxPort, this->oscNewSettings.oscRxPort);
		//oscStarted = this->oscInitialized;
		break;
	case OSCAction::None:
	default:
		break;
	}
	this->oscCurrentAction = OSCAction::None;

	// OSC is Enabled and Active light
	lights[LightIds::OSC_ENABLED_LIGHT].value = (oscInitialized) ? 1.0 : 0.0;

	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	// Rack Input Ports ==> OSC
	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	if (doCVPort2OSC) {
		// Timer for sending values
		bool sendTime = false;
		sendDt += sendFrequency_Hz / engineGetSampleRate();
		if (sendDt > 1.0f) {
			sendDt -= 1.0f;
			sendTime = true;
		}
		//------------------------------------------------------------
		// Look for inputs from Rack --> Send Out OSC
		//------------------------------------------------------------
		bool packetOpened = false;
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		char addressBuffer[512];
		for (int c = 0; c < this->numberChannels; c++)
		{
			inputChannels[c].setValue(inputs[InputIds::CH_INPUT_START + c * 2 + 1].value); // 2nd one is value
			bool sendVal = false;
			if (oscInitialized && inputs[InputIds::CH_INPUT_START + c * 2 + 1].active)
			{
				if (inputs[InputIds::CH_INPUT_START + c * 2].active) // Input Trigger Port
				{
					sendVal = inputTriggers[c].process(inputs[InputIds::CH_INPUT_START + c * 2].value);
				} // end if trigger is active
				else
				{
					if (!inputChannels[c].doSend) {
						// Only send if changed enough. Maybe about 0.01 V? Defined on channel.
						// 1. Check for change:
						if (inputChannels[c].convertVals)
							sendVal = std::abs(inputChannels[c].translatedVal - inputChannels[c].lastTranslatedVal) > inputChannels[c].channelSensitivity;
						else
							sendVal = std::abs(inputChannels[c].val - inputChannels[c].lastVal) > inputChannels[c].channelSensitivity;
						// 2. Mark channel as needing to output
						if (sendVal) {
							inputChannels[c].doSend = true;
						}
					}
					// 3. Check if it is time to send out
					sendVal = sendTime && inputChannels[c].doSend;
				}
				float outVal = inputChannels[c].translatedVal;
				if (sendVal)
				{
					lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value = 1.0f;
					oscMutex.lock();
					try
					{
						if (!packetOpened)
						{
							oscStream << osc::BeginBundleImmediate;
							packetOpened = true;
						}
						sprintf(addressBuffer, "/%s%s", oscNamespace.c_str(), inputChannels[c].getPath().c_str());
						oscStream << osc::BeginMessage(addressBuffer);
						if (inputChannels[c].convertVals)
						{
							// Enforce Data Type:
							switch (inputChannels[c].dataType)
							{
							case TSOSCCVChannel::ArgDataType::OscInt:
								oscStream << static_cast<int>(inputChannels[c].translatedVal);
								outVal = static_cast<float>(static_cast<int>(inputChannels[c].translatedVal));
								break;
							case TSOSCCVChannel::ArgDataType::OscBool:
								oscStream << static_cast<bool>(inputChannels[c].translatedVal);
								outVal = static_cast<float>(static_cast<bool>(inputChannels[c].translatedVal));
								break;
							case TSOSCCVChannel::ArgDataType::OscFloat:
							default:
								oscStream << inputChannels[c].translatedVal;
								break;
							}
						}
						else
						{
							// Raw value out
							oscStream << inputChannels[c].val;
						}
						oscStream << osc::EndMessage;
						//oscStream << osc::BeginMessage(addressBuffer)
						//	<< ((inputChannels[c].convertVals) ? inputChannels[c].translatedVal : inputChannels[c].val)
						//	<< osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
						debug("SEND OSC[%d]: %s %7.3f", c, addressBuffer, inputChannels[c].getValCV2OSC());
#endif					
					}
					catch (const std::exception& e)
					{
						warn("Error %s.", e.what());
					}
					oscMutex.unlock();
					// Save our last sent values
					//inputChannels[c].lastTranslatedVal = inputChannels[c].translatedVal;
					inputChannels[c].lastTranslatedVal = outVal;
					inputChannels[c].lastVal = inputChannels[c].val;
					inputChannels[c].doSend = false; // Reset
				} // end if send value 
			} // end if oscInitialied

			if (lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value > 0 && !sendVal) {
				lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL].value -= lightLambda;
			}
		} // end for loop
		if (packetOpened)
		{
			oscMutex.lock();
			try
			{
				oscStream << osc::EndBundle;
				oscTxSocket->Send(oscStream.Data(), oscStream.Size());
			}
			catch (const std::exception& e)
			{
				warn("Error %s.", e.what());
			}
			oscMutex.unlock();
		} // end if packet opened (close it)
	} // end Rack Input Ports ==> OSC Output

	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	// OSC ==> Rack Output Ports
	//--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--%--
	if (doOSC2CVPort)
	{
		//------------------------------------------------------------
		// Look for OSC Rx messages --> Output to Rack
		//------------------------------------------------------------
		while (rxMsgQueue.size() > 0)
		{
			TSOSCCVSimpleMessage rxOscMsg = rxMsgQueue.front();
			rxMsgQueue.pop();

			int chIx = rxOscMsg.channelNum - 1;
			if (chIx > -1 && chIx < numberChannels)
			{
				// Process the message
				pulseGens[chIx].trigger(TROWA_PULSE_WIDTH); // Trigger (msg received)
				//outputChannels[chIx].setValue(rxOscMsg.rxVal);
				outputChannels[chIx].setOSCInValue(rxOscMsg.rxVal);
				lights[LightIds::CH_LIGHT_START + chIx * 2 + 1].value = 1.0f;
			} // end if valid channel
		} // end while (loop through message queue)
		  // ::: OUTPUTS :::
		float dt = 1.0 / engineGetSampleRate();
		for (int c = 0; c < numberChannels; c++)
		{
			// Output the value first
			// We should limit this value (-10V to +10V). Rack says nothing should be higher than +/- 12V.
			// Do any massaging?
			float outVal = outputChannels[c].getValOSC2CV();
			outputs[OutputIds::CH_OUTPUT_START + c * 2 + 1].value = clamp(outVal, TROWA_OSCCV_MIN_VOLTAGE, TROWA_OSCCV_MAX_VOLTAGE);
			outputChannels[c].addValToBuffer(outVal);
			// Then trigger if needed.
			bool trigger = pulseGens[c].process(dt);
			outputs[OutputIds::CH_OUTPUT_START + c * 2].value = (trigger) ? TROWA_OSCCV_TRIGGER_ON_V : TROWA_OSCCV_TRIGGER_OFF_V;
			lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1].value = clamp(lights[LightIds::CH_LIGHT_START + c * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1].value - lightLambda, 0.0f, 1.0f);
		}
	}
	return;
} // end step()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// setOscNamespace()
// @oscNamespace : (IN) The namespace (without /).
// Set the OSC namespace (thread safe-ish).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCV::setOscNamespace(std::string oscNamespace)
{
	std::lock_guard<std::mutex> lock(oscMutex);
	if (oscNamespace[0] == '/')
		this->oscNamespace = oscNamespace.substr(1);
	else
		this->oscNamespace = oscNamespace;
	if (this->oscListener != NULL)
	{
		try
		{
			//debug("Setting listener's namespace: %s", oscNamespace.c_str());
			this->oscListener->setNamespace(oscNamespace);
		}
		catch (const std::exception& e)
		{
			warn("Error %s.", e.what());
		}
	}
	return;
} // end setOscNamespace()


//--------------------------------------------------------------------------------------------------------------------------------------------
// ProcessMessage()
// @rxMsg : (IN) The received message from the OSC library.
// @remoteEndPoint: (IN) The remove end point (sender).
// Handler for receiving messages from the OSC library. Taken from their example listener.
// Should create a generic TSOSCCVSimpleMessage and dump it in the module instance's queue.
//--------------------------------------------------------------------------------------------------------------------------------------------
void TSOSCCVSimpleMsgListener::ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint)
{
	(void)remoteEndpoint; // suppress unused parameter warning
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	debug("[RECV] OSC Message: %s", rxMsg.AddressPattern());
#endif
	try 
	{
		mutExNamespace.lock();
		const char* ns = this->oscNamespace.c_str();
		mutExNamespace.unlock();
		std::string addr = rxMsg.AddressPattern();
		int len = strlen(ns);
		if (std::strcmp(addr.substr(0, len).c_str(), ns) != 0) // Message is not for us
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("Message is not for our namespace (%s).", ns);
#endif
			return;
		}
		std::string subAddr = addr.substr(len);
		const char* path = subAddr.c_str();

		/// TODO: Support MIDI message type

//		std::vector<std::string> parts = str_split(subAddr.substr(1), '/');
//		int numParts = parts.size();
//
//#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
//		debug("[RECV] %s - %d parts.", path, numParts);
//#endif

		// Get the argument (not strongly typed.... We'll read in as float and cast as what we need in case > 1 channel is receiving this message)
		// (touchOSC will only ever send floats)
		osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
		osc::int32 intArg = 0;
		float floatArg = 0.0;
		bool boolArg = false;
		osc::MidiMessage midiArg;
		uint32_t uintArg = 0;
		try
		{
			int numArgs = rxMsg.ArgumentCount();
			if (numArgs > 0)
			{
				switch (*(rxMsg.TypeTags())) {
				case osc::TypeTagValues::INT32_TYPE_TAG:
					args >> intArg >> osc::EndMessage;
					boolArg = intArg > 0;
					floatArg = static_cast<float>(floatArg);
					break;
				case osc::TypeTagValues::TRUE_TYPE_TAG:
				case osc::TypeTagValues::FALSE_TYPE_TAG:
					args >> boolArg >> osc::EndMessage;
					floatArg = static_cast<float>(boolArg);
					intArg = static_cast<int>(boolArg);
					break;
				case osc::TypeTagValues::MIDI_MESSAGE_TYPE_TAG:
					args >> midiArg >> osc::EndMessage;
					uintArg = midiArg.value;
					break;
				case osc::TypeTagValues::FLOAT_TYPE_TAG:
				default:
					args >> floatArg >> osc::EndMessage;
					intArg = static_cast<int>(floatArg);
					boolArg = floatArg > 0;
					break;
				}
			}
		}
		catch (osc::WrongArgumentTypeException touchOSCEx)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Wrong argument type: Error %s message: ", path, touchOSCEx.what());
			debug("We received (float) %05.2f.", floatArg);
#endif
		}

		// Make a message for each channel that may be listening to this address.
		for (int c = 0; c < oscModule->numberChannels; c++)
		{
			if (strlen(path) == strlen(oscModule->outputChannels[c].path.c_str()) && std::strcmp(path, oscModule->outputChannels[c].path.c_str()) == 0)
			{
				switch (oscModule->outputChannels[c].dataType)
				{
					case TSOSCCVChannel::ArgDataType::OscBool:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
						debug("OSC Recv Ch %d: Bool %d at %s.", c+1, boolArg, oscModule->outputChannels[c].path.c_str());
#endif
						oscModule->rxMsgQueue.push(TSOSCCVSimpleMessage(c + 1, boolArg));
						break;
					case TSOSCCVChannel::ArgDataType::OscInt:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
						debug("OSC Recv Ch %d: Int %d at %s.", c + 1, intArg, oscModule->outputChannels[c].path.c_str());
#endif
						oscModule->rxMsgQueue.push(TSOSCCVSimpleMessage(c + 1, intArg));
						break;
					case TSOSCCVChannel::ArgDataType::OscMidi:
						// Actually, I don't think anything natively supports this, so this would be unused.
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
						debug("OSC Recv Ch %d: MIDI %08x at %s.", c + 1, uintArg, oscModule->outputChannels[c].path.c_str());
#endif
						oscModule->rxMsgQueue.push(TSOSCCVSimpleMessage(c + 1, floatArg, uintArg));
						break;
					case TSOSCCVChannel::ArgDataType::OscFloat:
					default:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
						debug("OSC Recv Ch %d: Float %7.4f at %s.", c + 1, floatArg, oscModule->outputChannels[c].path.c_str());
#endif
						oscModule->rxMsgQueue.push(TSOSCCVSimpleMessage(c + 1, floatArg));
						break;
				} // end switch
			} // end if path matches
		} // end loop through channels
	}
	catch (osc::Exception& e) {
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("Error parsing OSC message %s: %s", rxMsg.AddressPattern(), e.what());
#endif
	} // end catch
	return;
} // end ProcessMessage()

//--------------------------------------------------------
// addValToBuffer()
// Add a value to the buffer.
// @buffVal : (IN) The value to possibly add.
//--------------------------------------------------------
void TSOSCCVChannel::addValToBuffer(float buffVal)
{
	float deltaTime = powf(2.0, -12.0);
	int frameCount = (int)ceilf(deltaTime * engineGetSampleRate());
	// Add frame to buffer
	if (valBuffIx < TROWA_OSCCV_VAL_BUFFER_SIZE) {
		if (++frameIx > frameCount) {
			frameIx = 0;
			valBuffer[valBuffIx++] = buffVal;
		}
	}
	else {
		frameIx++;
		const float holdTime = 0.1;
		if (frameIx >= engineGetSampleRate() * holdTime) {
			valBuffIx = 0;
			frameIx = 0;
		}
	}
	return;
} // end addValToBuffer()

//--------------------------------------------------------
// serialize()
// @returns : The channel json node.
//--------------------------------------------------------
json_t* TSOSCCVChannel::serialize()
{
	json_t* channelJ = json_object();
	json_object_set_new(channelJ, "path", json_string(getPath().c_str()));
	json_object_set_new(channelJ, "dataType", json_integer(dataType));
	json_object_set_new(channelJ, "convertVals", json_integer(convertVals));
	json_object_set_new(channelJ, "minV", json_real(minVoltage));
	json_object_set_new(channelJ, "maxV", json_real(maxVoltage));
	json_object_set_new(channelJ, "minOSC", json_real(minOscVal));
	json_object_set_new(channelJ, "maxOSC", json_real(maxOscVal));
	return channelJ;
} // end serialize()
//--------------------------------------------------------
// deserialize()
// @rootJ : (IN) The channel json node.
//--------------------------------------------------------
void TSOSCCVChannel::deserialize(json_t* rootJ) {
	json_t* currJ = NULL;
	if (rootJ) {
		currJ = json_object_get(rootJ, "path");
		if (currJ)
			setPath(json_string_value(currJ));
		currJ = json_object_get(rootJ, "dataType");
		if (currJ)
			dataType = static_cast<TSOSCCVChannel::ArgDataType>(json_integer_value(currJ));
		currJ = json_object_get(rootJ, "convertVals");
		if (currJ)
			convertVals = static_cast<bool>(json_integer_value(currJ));
		currJ = json_object_get(rootJ, "minV");
		if (currJ)
			minVoltage = json_real_value(currJ);
		currJ = json_object_get(rootJ, "maxV");
		if (currJ)
			maxVoltage = json_real_value(currJ);
		currJ = json_object_get(rootJ, "minOSC");
		if (currJ)
			minOscVal = json_real_value(currJ);
		currJ = json_object_get(rootJ, "maxOSC");
		if (currJ)
			maxOscVal = json_real_value(currJ);
	}
	return;
} // end deserialize()

//--------------------------------------------------------
// serialize()
// @returns : The channel json node.
//--------------------------------------------------------
json_t* TSOSCCVInputChannel::serialize()
{
	json_t* channelJ = TSOSCCVChannel::serialize();
	json_object_set_new(channelJ, "channelSensitivity", json_real(channelSensitivity));
	return channelJ;
} // end serialize()
//--------------------------------------------------------
// deserialize()
// @rootJ : (IN) The channel json node.
//--------------------------------------------------------
void TSOSCCVInputChannel::deserialize(json_t* rootJ) {
	TSOSCCVChannel::deserialize(rootJ);
	if (rootJ) {
		json_t* currJ = NULL;
		currJ = json_object_get(rootJ, "channelSensitivity");
		if (currJ)
			channelSensitivity = json_real_value(currJ);
	}
	return;
} // end deserialize()

// Model for trowa OSC2CV
RACK_PLUGIN_MODEL_INIT(trowaSoft, OscCV) {
   Model* modelOscCV = Model::create<oscCV, oscCVWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "cvOSCcv", /*name*/ "cvOSCcv", /*Tags*/ EXTERNAL_TAG);
   return modelOscCV;
}
