#include <chrono>
#include <string.h>
#include <exception>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "TSOSCSequencerListener.hpp"
#include "TSOSCSequencerOutputMessages.hpp"
#include "TSOSCCommunicator.hpp"
#include "TSSequencerWidgetBase.hpp"

// Static Variables:
RandStructure TSSequencerModuleBase::RandomPatterns[TROWA_SEQ_NUM_RANDOM_PATTERNS] = {
	{ 1,{ 0 } },
	{ 2,{ 0, 1 } },
	{ 2,{ 0,0,0,1 } },
	{ 2,{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } },
	{ 2,{ 0,0,1,1 } },
	{ 2,{ 0,1,1,1 } },
	{ 2,{ 0,1,1,0 } },
	{ 3,{ 0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,2 } },
	{ 3,{ 0,0,1,2 } },
	{ 3,{ 0,0,0,0,0,0,1,2 } },
	{ 3,{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2 } },
	{ 3,{ 0,1,0,2 } },
	{ 3,{ 0,1,2,0 } },
	{ 3,{ 0,1,1,2 } },
	{ 3,{ 0,2,1,2 } },
	{ 4,{ 0,1,2,3 } },
	{ 4,{ 0,0,1,1,2,2,3,3 } },
	{ 4,{ 0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3 } },
	{ 5,{ 0,1,0,1,0,1,0,2,0,3,0,3,0,3,0,4 } },
	{ 6,{ 0,1,0,2,0,1,0,3,0,1,0,4,0,1,0,5 } },
	{ 8,{ 0,0,0,1,2,3,4,5,0,0,0,1,2,3,6,7 } },
	{ 9,{ 0,1,0,2,0,1,0,3,0,1,0,4,5,6,7,8 } },
	{ 10,{ 0,1,1,2,3,4,5,6,0,1,1,2,3,7,8,9 } },
	{ 10,{ 0,1,1,2,3,4,5,6,1,1,1,2,3,7,8,9 } },
	{ 10,{ 0,1,2,3,4,5,6,7,0,1,2,3,4,5,8,9 } },
	{ 10,{ 0,1,2,3,4,5,6,7,0,1,8,3,4,5,6,9 } },
	{ 11,{ 0,1,2,3,4,5,6,7,8,1,2,3,4,5,9,10 } }
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerModuleBase()
// Instantiate the abstract base class.
// @numSteps: (IN) Maximum number of steps
// @numRows: (IN) The number of rows (for layout).
// @numCols: (IN) The number of columns (for layout).
// @numRows * @numCols = @numSteps
// @defStateVal : (IN) The default state value (i.e. 0/false for a boolean step sequencer or whatever float value you want).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSSequencerModuleBase::TSSequencerModuleBase(/*in*/ int numSteps, /*in*/ int numRows, /*in*/ int numCols, /*in*/ float defStateVal) : Module(NUM_PARAMS + numSteps, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS + numSteps)
{
	useOSC = false;
	oscInitialized = false;
	oscBuffer = NULL;
	oscTxSocket = NULL;
	oscListener = NULL;
	oscRxSocket = NULL;
	oscNamespace = OSC_DEFAULT_NS;
	oscId = TSOSCConnector::GetId();

	for (int i = 0; i < SeqOSCOutputMsg::NUM_OSC_OUTPUT_MSGS; i++)
	{
		for (int j = 0; j < OSC_ADDRESS_BUFFER_SIZE; j++)
			oscAddrBuffer[i][j] = '\0';
	}

	prevIndex = TROWA_INDEX_UNDEFINED;

	gateTriggers = NULL;

	lastStepWasExternalClock = false;
	defaultStateValue = defStateVal;
	currentChannelEditingIx = 0;
	currentPatternEditingIx = 0;
	currentPatternPlayingIx = 0;
	// Number of steps in not static at compile time anymore...
	maxSteps = numSteps; // Num Steps may vary now up to 64
	currentNumberSteps = maxSteps;
	storedNumberSteps = maxSteps;
	this->numRows = numRows;
	this->numCols = numCols;

	stepLights = new float*[numRows];
	gateLights = new float*[numRows];
	padLightPtrs = new ColorValueLight**[numRows];

	for (int r = 0; r < numRows; r++)
	{
		stepLights[r] = new float[numCols];
		gateLights[r] = new float[numCols];
		padLightPtrs[r] = new ColorValueLight*[numCols];
		for (int c = 0; c < numCols; c++)
		{
			stepLights[r][c] = 0;
			gateLights[r][c] = 0;
		}
	}
	for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
	{
		copyBuffer[g] = new float[maxSteps];
	}

	for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
	{
		for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
		{
			triggerState[p][g] = new float[maxSteps];
			for (int s = 0; s < maxSteps; s++)
			{
				triggerState[p][g][s] = defaultStateValue;
			}
		}
	}
	modeStrings[0] = "TRIG";
	modeStrings[1] = "RTRG";
	modeStrings[2] = "GATE"; // CONT/GATE

	copySourcePatternIx = -1;
	copySourceChannelIx = TROWA_SEQ_COPY_CHANNELIX_ALL; // Which trigger we are copying, -1 for all		

	initialized = false;
	firstLoad = true;
	return;
} // end TSSequencerModuleBase()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Delete our goodies.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
TSSequencerModuleBase::~TSSequencerModuleBase()
{
	initialized = false; // Stop doing stuff
	cleanupOSC();
	for (int r = 0; r < numRows; r++)
	{
		if (stepLights[r])
			delete[] stepLights[r];
		if (gateLights[r])
			delete[] gateLights[r];
		if (padLightPtrs[r])
			delete[] padLightPtrs[r];
	}
	if (stepLights != NULL)
	{
		delete[] stepLights; stepLights = NULL;
	}
	if (gateLights != NULL)
	{
		delete[] gateLights; gateLights = NULL;
	}
	if (padLightPtrs != NULL)
	{
		delete[] padLightPtrs;	padLightPtrs = NULL;
	}
	for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
	{
		delete[] copyBuffer[g];
		copyBuffer[g] = NULL; // We should be totally dead & unreferenced anyway, so I'm not sure we have NULL our ptrs???
	}
	for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
	{
		for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
		{
			delete[] triggerState[p][g];
			triggerState[p][g] = NULL;
		}
	}
	this->copyGateLight = NULL;
	this->copyPatternLight = NULL;
	this->pasteLight = NULL;
	// Free our buffer if we had initialized it
	oscMutex.lock();
	if (oscBuffer != NULL)
	{
		free(oscBuffer);
		oscBuffer = NULL;
	}
	oscMutex.unlock();
	return;
} // end ~TSSequencerModuleBase()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// reset(void)
// Reset ALL step values to default.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::reset()
{
	valuesChanging = true;
	for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
	{
		for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				triggerState[p][c][s] = defaultStateValue;
			}
		}
	}
	/// TODO: Also clear our clipboard and turn off OSC?
	reloadEditMatrix = true;
	valuesChanging = false;
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// randomize()
// @patternIx : (IN) The index into our pattern matrix (0-63). Or TROWA_INDEX_UNDEFINED for all patterns.
// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
// @useStructured: (IN) Create a random sequence/pattern of random values.
// Random all from : https://github.com/j4s0n-c/trowaSoft-VCV/issues/8
// Structured from : https://github.com/j4s0n-c/trowaSoft-VCV/issues/10
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::randomize(int patternIx, int channelIx, bool useStructured)
{
	if (patternIx == TROWA_INDEX_UNDEFINED)
	{
		// All patterns:
		for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
		{
			randomize(p, TROWA_INDEX_UNDEFINED, useStructured); // All channels
		}
	}
	else if (channelIx == TROWA_INDEX_UNDEFINED)
	{
		// This pattern:
		for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
		{
			randomize(patternIx, c, useStructured);
		}
	}
	else
	{
		valuesChanging = true;
		// -- Randomize Channel Specified --
		float val;
		if (useStructured)
		{
			// Use a pattern
			// A, AB, ABBA, ABAC
			int rIx = rand() % numStructuredRandomPatterns;// TROWA_SEQ_NUM_RANDOM_PATTERNS;
			int n = RandomPatterns[rIx].numDiffVals;
			float* randVals = new float[n];
			int patternLen = RandomPatterns[rIx].pattern.size();
			// Every Channel should get its own random pattern
			for (int i = 0; i < n; i++)
				randVals[i] = getRandomValue();

			for (int s = 0; s < maxSteps; s++)
			{
				val = randVals[RandomPatterns[rIx].pattern[s % patternLen]];
				triggerState[patternIx][channelIx][s] = val;
				if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
					onShownStepChange(s, val);
			}
			delete[] randVals;
		} // end if random pattern/structure
		else
		{
			// Every value is random
			for (int s = 0; s < maxSteps; s++)
			{
				val = getRandomValue();
				triggerState[patternIx][channelIx][s] = val;
				if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
					onShownStepChange(s, val);
			}
		} // end else (normal Rand -- all values random)
		reloadEditMatrix = (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx);
		valuesChanging = false;
	} // end else (channel and pattern specified)
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Set the OSC namespace.
// @oscNs: (IN) The namespace for OSC.
// Sets the command address strings too.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::setOSCNamespace(const char* oscNs)
{
	this->oscNamespace = oscNs;
	for (int i = 0; i < SeqOSCOutputMsg::NUM_OSC_OUTPUT_MSGS; i++)
	{
		// Create our array of output addresses based on the base format and the osc name space.
		sprintf(this->oscAddrBuffer[i], TSSeqOSCOutputFormats[i], oscNamespace.c_str());
	}
	// Add %d (all this was changed for touchOSC's limitations)
	std::strcat(oscAddrBuffer[SeqOSCOutputMsg::EditStepString], "%d");
	std::strcat(oscAddrBuffer[SeqOSCOutputMsg::EditStep], "%d");
	std::strcat(oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], "%d");
	// [touchOSC] Add some <row>/<col>
	std::strcat(oscAddrBuffer[SeqOSCOutputMsg::EditTOSC_GridStep], "%d/%d");

	return;
} // end setOSCNameSpace()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Initialize OSC on the given ip and ports.
// @ipAddress: (IN) The ip address.
// @outputPort: (IN) The output port.
// @inputPort: (IN) The input port.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::initOSC(const char* ipAddress, int outputPort, int inputPort)
{
	oscMutex.lock();
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	debug("TSSequencerModuleBase::initOSC() - Initializing OSC");
#endif
	try
	{
		// Try to register these ports:
		if (TSOSCConnector::RegisterPorts(oscId, outputPort, inputPort))
		{
			oscError = false;
			this->currentOSCSettings.oscTxIpAddress = ipAddress;
			this->setOSCNamespace(this->oscNamespace.c_str());
			if (oscBuffer == NULL)
			{
				oscBuffer = (char*)malloc(OSC_OUTPUT_BUFFER_SIZE * sizeof(char));
			}
			if (oscTxSocket == NULL)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				debug("TSSequencerModuleBase::initOSC() - Create TRANS socket at %s, port %d.", ipAddress, outputPort);
#endif
				oscTxSocket = new UdpTransmitSocket(IpEndpointName(ipAddress, outputPort));
				this->currentOSCSettings.oscTxPort = outputPort;
			}
			if (oscRxSocket == NULL)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				debug("TSSequencerModuleBase::initOSC() - Create RECV socket at any address, port %d.", inputPort);
#endif
				oscListener = new TSOSCSequencerListener();
				oscListener->sequencerModule = this;
				oscListener->oscNamespace = this->oscNamespace;
				oscRxSocket = new UdpListeningReceiveSocket(IpEndpointName(IpEndpointName::ANY_ADDRESS, inputPort), oscListener);
				this->currentOSCSettings.oscRxPort = inputPort;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
				debug("TSSequencerModuleBase::initOSC() - Starting listener thread...");
#endif
				oscListenerThread = std::thread(&UdpListeningReceiveSocket::Run, oscRxSocket);
			}
//#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
//			debug("TSSequencerModuleBase::initOSC() - OSC Initialized");
//#endif
			info("TSSequencerModuleBase::initOSC() - OSC Initialized : %s :%d :%d", ipAddress, outputPort, inputPort);

			oscInitialized = true;
		}
		else
		{
			oscError = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("TSSequencerModuleBase::initOSC() - Ports in use already.");
#endif
		}
	}
	catch (const std::exception& ex)
	{
		oscError = true;
		warn("TSSequencerModuleBase::initOSC() - Error initializing: %s.", ex.what());
	}
	oscMutex.unlock();
	return;
} // end initOSC()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Clean up OSC.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::cleanupOSC()
{
	oscMutex.lock();
	try
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("TSSequencerModuleBase::cleanupOSC() - Cleaning up OSC");
#endif
		oscInitialized = false;
		oscError = false;
		TSOSCConnector::ClearPorts(oscId, currentOSCSettings.oscTxPort, currentOSCSettings.oscRxPort);
		if (oscRxSocket != NULL)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("TSSequencerModuleBase::cleanupOSC() - Cleaning up RECV socket.");
#endif
			oscRxSocket->AsynchronousBreak();
			oscListenerThread.join(); // Wait for him to finish
			delete oscRxSocket;
			oscRxSocket = NULL;
		}
		if (oscTxSocket != NULL)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("TSSequencerModuleBase::cleanupOSC() - Cleanup TRANS socket.");
#endif
			delete oscTxSocket;
			oscTxSocket = NULL;
		}
		//if (oscBuffer != NULL)
		//{
		//	free(oscBuffer);
		//	oscBuffer = NULL;
		//}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("TSSequencerModuleBase::cleanupOSC() - OSC cleaned");
#endif
	}
	catch (const std::exception& ex)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("TSSequencerModuleBase::cleanupOSC() - Exception caught:\n%s", ex.what());
#endif
	}
	oscMutex.unlock();
	return;
} // end cleanupOSC()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// copy()
// @patternIx : (IN) The index into our pattern matrix (0-15).
// @channelIx : (IN) The index of the channel (gate/trigger/voice) to copy if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL for all).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::copy(/*in*/ int patternIx, /*in*/ int channelIx)
{
	copySourceChannelIx = channelIx;
	copySourcePatternIx = patternIx;
	if (copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
	{
		// Copy entire pattern (all gates/triggers/voices)
		for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				copyBuffer[g][s] = triggerState[copySourcePatternIx][g][s];
			}
		}
	}
	else
	{
		// Copy just the gate:
		for (int s = 0; s < maxSteps; s++)
		{
			copyBuffer[copySourceChannelIx][s] = triggerState[copySourcePatternIx][copySourceChannelIx][s];
		}
	}
	return;
} // end copy()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// paste(void)
// Paste our current clipboard Pattern/Gate to the currently selected Pattern/Gate.
// @returns: True if the values were copied, false if not.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool TSSequencerModuleBase::paste()
{
	if (copySourcePatternIx < 0) // Nothing to copy
		return false;
	valuesChanging = true;
	if (copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
	{
		// Copy entire pattern (all gates/triggers/voices)
		for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				triggerState[currentPatternEditingIx][g][s] = copyBuffer[g][s];
			}
		}
	}
	else
	{
		// Copy just the channel:
		for (int s = 0; s < maxSteps; s++)
		{
			triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = copyBuffer[copySourceChannelIx][s];
		}
	}
	valuesChanging = false;
	return true;
} // end paste()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Set a single the step value
// (i.e. this command probably comes from an external source)
// @step : (IN) The step number to edit (0 to maxSteps).
// @val : (IN) The step value.
// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::setStepValue(int step, float val, int channel, int pattern)
{
	int r, c;
	if (channel == CURRENT_EDIT_CHANNEL_IX)
	{
		channel = currentChannelEditingIx;
	}
	if (pattern == CURRENT_EDIT_PATTERN_IX)
	{
		pattern = currentPatternEditingIx;
	}
	triggerState[pattern][channel][step] = val;
	r = step / this->numCols;
	c = step % this->numCols;
	if (pattern == currentPatternEditingIx && channel == currentChannelEditingIx)
	{
		if (triggerState[pattern][channel][step])
		{
			gateLights[r][c] = 1.0f - stepLights[r][c];
			if (gateTriggers != NULL)
				gateTriggers[step].state = SchmittTrigger::HIGH;
		}
		else
		{
			gateLights[r][c] = 0.0f; // Turn light off	
			if (gateTriggers != NULL)
				gateTriggers[step].state = SchmittTrigger::LOW;
		}
	}
	oscMutex.lock();
	if (useOSC && oscInitialized)
	{
		try
		{
			char addrBuff[50] = { 0 };
			// Send the result back
			if (this->oscCurrentClient == OSCClient::touchOSCClient)
			{
				int gridRow, gridCol;
				touchOSC::stepIndex_to_mcRowCol(step, numRows, numCols, &gridRow, &gridCol);
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditTOSC_GridStep], gridRow, gridCol); // Grid's /<row>/<col> to accomodate touchOSC's lack of multi-parameter support.
			}
			else
			{
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], step + 1); // Changed to /<step> to accomodate touchOSC's lack of multi-parameter support.
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("setStepValue() - Received a msg (s=%d, v=%0.2f, c=%d, p=%d), sending back (%s).",
				step, val, channel, pattern,
				addrBuff);
#endif
			osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
			oscStream << osc::BeginBundleImmediate
				<< osc::BeginMessage(addrBuff)
				<< triggerState[pattern][channel][step]
				<< osc::EndMessage
				<< osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());

		}
		catch (const std::exception &e)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("seStepValue - Error sending back msg: %s.", e.what());
#endif
		}
	}
	oscMutex.unlock();
	return;
} // end setStepValue()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// getStepInputs()
// Get the inputs shared between our Sequencers.
// Now also processes our external message queue.
// @pulse : (OUT) If gate pulse
// @reloadMatrix: (OUT) If the edit matrix should be refreshed.
// @valueModeChanged: (OUT) If the output value mode has changed.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSSequencerModuleBase::getStepInputs(/*out*/ bool* pulse, /*out*/ bool* reloadMatrix, /*out*/ bool* valueModeChanged)
{
	// Track if we have changed these
	bool editPatternChanged = false;
	bool editChannelChanged = false;
	float lastBPM = currentBPM;
	bool playBPMChanged = false;
	bool lastRunning = running;
	int lastBPMNoteIx = this->selectedBPMNoteIx;
	int lastStepIndex = index;

	// Run
	if (runningTrigger.process(params[RUN_PARAM].value)) {
		running = !running;
	}
	lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;

	bool oscStarted = false; // If OSC just started to a new address this step.
	switch (this->oscCurrentAction)
	{
	case OSCAction::Disable:
		this->cleanupOSC(); // Try to clean up OSC
		break;
	case OSCAction::Enable:
		this->cleanupOSC(); // Try to clean up OSC if we already have something
		this->initOSC(this->oscNewSettings.oscTxIpAddress.c_str(), this->oscNewSettings.oscTxPort, this->oscNewSettings.oscRxPort);
		this->useOSC = true;
		oscStarted = this->useOSC && this->oscInitialized;
		break;
	case OSCAction::None:
	default:
		break;
	}
	this->oscCurrentAction = OSCAction::None;

	// OSC is Enabled and Active light
	lights[LightIds::OSC_ENABLED_LIGHT].value = (useOSC && oscInitialized) ? 1.0 : 0.0;

	if (!firstLoad)
		lastPatternPlayingIx = currentPatternPlayingIx;


	bool nextStep = false;
	// Now calculate BPM even if we are paused:
	// BPM calculation selection
	if (selectedBPMNoteTrigger.process(params[SELECTED_BPM_MULT_IX_PARAM].value)) {
		if (selectedBPMNoteIx < TROWA_TEMP_BPM_NUM_OPTIONS - 1)
			selectedBPMNoteIx++;
		else
			selectedBPMNoteIx = 0; // Wrap around
		lights[SELECTED_BPM_MULT_IX_LIGHT].value = 1.0;
	}
	float clockTime = 1.0;
	float input = 1.0;
	if (inputs[BPM_INPUT].active)
	{
		// Use whatever voltage we are getting (-10 TO 10 input)
		input = rescale(inputs[BPM_INPUT].value, TROWA_SEQ_PATTERN_MIN_V, TROWA_SEQ_PATTERN_MAX_V,
			TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX);
	}
	else
	{
		// Otherwise read our knob
		input = params[BPM_PARAM].value; // -2 to 6
	}
	clockTime = powf(2.0, input); // -2 to 6
	// Calculate his all the time now instead of just on next step:
	currentBPM = roundf(clockTime * BPMOptions[selectedBPMNoteIx]->multiplier);
	playBPMChanged = lastBPM != currentBPM;

	if (running)
	{
		if (inputs[EXT_CLOCK_INPUT].active)
		{
			// External clock input
			if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value))
			{
				realPhase = 0.0;
				nextStep = true;
				lastStepWasExternalClock = true;
			}
		}
		else
		{
			// Internal clock
			lastStepWasExternalClock = false;
			float dt = clockTime / engineGetSampleRate(); // Real dt
			realPhase += dt; // Real Time no matter what
			if (realPhase >= 1.0)
			{
				realPhase -= 1.0;
				nextStep = true;
			}
			//if (nextStep)
			//{	
			//currentBPM = roundf(clockTime * BPMOptions[selectedBPMNoteIx]->multiplier);
			//playBPMChanged = lastBPM != currentBPM;
			//}
		}
	} // end if running

	// Current Playing Pattern
	// If we get an input, then use that:
	if (inputs[SELECTED_PATTERN_PLAY_INPUT].active)
	{
		currentPatternPlayingIx = VoltsToPattern(inputs[SELECTED_PATTERN_PLAY_INPUT].value) - 1;
	}
	else
	{
		// Otherwise read our knob parameter and use that
		currentPatternPlayingIx = (int)clamp(static_cast<int>(roundf(params[SELECTED_PATTERN_PLAY_PARAM].value)), 0, TROWA_SEQ_NUM_PATTERNS - 1);
	}
	if (currentPatternPlayingIx < 0)
		currentPatternPlayingIx = 0;
	else if (currentPatternPlayingIx > TROWA_SEQ_NUM_PATTERNS - 1)
		currentPatternPlayingIx = TROWA_SEQ_NUM_PATTERNS - 1;


	// Current Edit Pattern
	int lastEditPatternIx = currentPatternEditingIx;
	// From User Knob:
	currentPatternEditingIx = (int)clamp(static_cast<int>(roundf(params[SELECTED_PATTERN_EDIT_PARAM].value)), 0, TROWA_SEQ_NUM_PATTERNS - 1);
	if (currentPatternEditingIx < 0)
		currentPatternEditingIx = 0;
	else if (currentPatternEditingIx > TROWA_SEQ_NUM_PATTERNS - 1)
		currentPatternEditingIx = TROWA_SEQ_NUM_PATTERNS - 1;


	// Gate inputs (which gate we are displaying & editing)
	int lastChannelIx = currentChannelEditingIx;
	currentChannelEditingIx = (int)clamp(static_cast<int>(roundf(params[SELECTED_CHANNEL_PARAM].value)), 0, TROWA_SEQ_NUM_CHNLS - 1);
	if (currentChannelEditingIx < 0)
		currentChannelEditingIx = 0;
	else if (currentChannelEditingIx > TROWA_SEQ_NUM_CHNLS - 1)
		currentChannelEditingIx = TROWA_SEQ_NUM_CHNLS - 1;
	editChannelChanged = lastChannelIx != currentChannelEditingIx;


	int r = 0;
	int c = 0;

	// Current output value mode	
	selectedOutputValueMode = static_cast<ValueMode>((int)clamp(static_cast<int>(roundf(params[SELECTED_OUTPUT_VALUE_MODE_PARAM].value)), 0, TROWA_SEQ_NUM_MODES - 1));

	int lastNumberSteps = currentNumberSteps;
	if (inputs[STEPS_INPUT].active)
	{
		// Use the input if something is connected.
		// Some seqeuencers go to 64 steps, we want the same voltage to mean the same step number no matter how many max steps this one takes.
		// so voltage input is normalized to indicate step 1 to step 64, but we'll limit it to maxSteps.
		currentNumberSteps = (int)clamp(static_cast<int>(roundf(rescale(inputs[STEPS_INPUT].value, TROWA_SEQ_STEPS_MIN_V, TROWA_SEQ_STEPS_MAX_V, 1.0, (float)TROWA_SEQ_MAX_NUM_STEPS))), 1, maxSteps);
	}
	else
	{
		// Otherwise read our knob
		currentNumberSteps = (int)clamp(static_cast<int>(roundf(params[STEPS_PARAM].value)), 1, maxSteps);
	}

	//------------------------------------------------------------
	// Check if we have any eternal messages.
	// (i.e. from OSC)
	//------------------------------------------------------------
	/// TODO: Check performance hit from sending OSC in general
	/// TODO: Some thread safety and make sure that this queue never gets unruly? such that we start getting out of time.
	bool resetMsg = false;
	bool doPaste = false;
	int prevCopyPatternIx = copySourcePatternIx;
	int prevCopyChannelIx = copySourceChannelIx;
	bool storedPatternChanged = false;
	bool storedLengthChanged = false;
	bool storedBPMChanged = false;
	while (ctlMsgQueue.size() > 0)
	{
		TSExternalControlMessage recvMsg = (TSExternalControlMessage)(ctlMsgQueue.front());
		ctlMsgQueue.pop();
		float tmp;
		/// TODO: redorder switch for most common cases first.
		switch (recvMsg.messageType)
		{
		case TSExternalControlMessage::MessageType::ToggleEditStepValue:
		case TSExternalControlMessage::MessageType::SetEditStepValue:
			if (currentCtlMode == ExternalControllerMode::EditMode)
			{
				int p = (recvMsg.pattern == CURRENT_EDIT_PATTERN_IX) ? currentPatternEditingIx : recvMsg.pattern;
				int c = (recvMsg.channel == CURRENT_EDIT_CHANNEL_IX) ? currentChannelEditingIx : recvMsg.channel;
				float oldVal = this->triggerState[p][c][recvMsg.step];
				float val = (recvMsg.messageType == TSExternalControlMessage::MessageType::ToggleEditStepValue) ? getToggleStepValue(recvMsg.step, recvMsg.val, /*channel*/ c, /*pattern*/ p) : recvMsg.val;
				if (oldVal != val)
				{
					// Value has changed:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("[%d] Set Step Value (value changed): %d (P %d, C %d) = %.4f.", recvMsg.messageType, recvMsg.step, p, c, val);
#endif
					this->setStepValue(recvMsg.step, val, /*channel*/ c, /*pattern*/ p);
				}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				else
				{
					debug("[%d] Step value did not change -- Ignore : %d (P %d, C %d) = %.4f.", recvMsg.messageType, recvMsg.step, p, c, val);
				}
#endif
			}
			else
			{
				// In performance mode, this will be interupted as jump to (playing):
				if (recvMsg.pattern != CURRENT_EDIT_PATTERN_IX)
				{
					currentPatternPlayingIx = recvMsg.pattern; // Jump to this pattern if sent
															 // Update our knob
					controlKnobs[KnobIx::PlayPatternKnob]->value = currentPatternPlayingIx;
					controlKnobs[KnobIx::PlayPatternKnob]->dirty = true;
					params[ParamIds::SELECTED_PATTERN_PLAY_PARAM].value = currentPatternPlayingIx;
				}
				// Jump to this step:
				if (nextStep)
				{
					// We are already at beginning of a step, so we can sneak this in now.
					index = recvMsg.step - 1; // We will set nextStep to true to go to this step fresh
					nextIndex = TROWA_INDEX_UNDEFINED;
				}
				else
				{
					// Next time we are ready to go to the next step, this should be it.
					nextIndex = recvMsg.step;
				}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Performance Mode: Jump to Step (index): %d (Pattern %d).", index, currentPatternPlayingIx);
#endif
			}
			break;
		case TSExternalControlMessage::MessageType::SetPlayCurrentStep:
			// We want to wait until the 'next step' (finish the one we are currently doing so things are still in time).
			if (nextStep)
			{
				// We are already at beginning of a step, so we can sneak this in now.
				index = recvMsg.step - 1; // We will set nextStep to true to go to this step fresh
				nextIndex = TROWA_INDEX_UNDEFINED;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Set Play Step (index): %d [Immediate].", recvMsg.step);
#endif
			}
			else
			{
				// Next time we are ready to go to the next step, this should be it.
				nextIndex = recvMsg.step;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Set Play Step (index): %d [Next, curr is %d].", nextIndex, index);
#endif
			}
			break;
		case TSExternalControlMessage::MessageType::SetPlayReset:
			resetMsg = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set Reset.");
#endif
			break;
		case TSExternalControlMessage::MessageType::SetPlayPattern:
			if (recvMsg.pattern == TROWA_INDEX_UNDEFINED)
				recvMsg.pattern = storedPatternPlayingIx; // Check our stored pattern
			if (recvMsg.pattern > -1 && recvMsg.pattern < TROWA_SEQ_NUM_PATTERNS)
			{
				currentPatternPlayingIx = recvMsg.pattern;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Set Play Pattern: %d.", currentPatternPlayingIx);
#endif
				// Update our knob
				controlKnobs[KnobIx::PlayPatternKnob]->value = currentPatternPlayingIx;
				controlKnobs[KnobIx::PlayPatternKnob]->dirty = true;
				params[ParamIds::SELECTED_PATTERN_PLAY_PARAM].value = currentPatternPlayingIx;
			}
			break;
		case TSExternalControlMessage::MessageType::StorePlayPattern:
			if (storedPatternPlayingIx != recvMsg.pattern)
			{
				storedPatternPlayingIx = recvMsg.pattern;
				storedPatternChanged = true;
			}
			break;
		case TSExternalControlMessage::MessageType::SetPlayOutputMode:
			// -- Set Ouput Mode: (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT) --
			selectedOutputValueMode = (ValueMode)(recvMsg.mode);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set Output Mode: %d (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT).", selectedOutputValueMode);
#endif
			controlKnobs[KnobIx::OutputModeKnob]->value = selectedOutputValueMode;
			controlKnobs[KnobIx::OutputModeKnob]->dirty = true;
			params[ParamIds::SELECTED_OUTPUT_VALUE_MODE_PARAM].value = selectedOutputValueMode;
			break;
		case TSExternalControlMessage::MessageType::SetEditPattern:
			currentPatternEditingIx = recvMsg.pattern;
			*reloadMatrix = true; // Refresh our display matrix
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set Edit Pattern: %d.", currentPatternEditingIx);
#endif
			// Update our knob
			controlKnobs[KnobIx::EditPatternKnob]->value = currentPatternEditingIx;
			controlKnobs[KnobIx::EditPatternKnob]->dirty = true;
			params[ParamIds::SELECTED_PATTERN_EDIT_PARAM].value = currentPatternEditingIx;
			break;
		case TSExternalControlMessage::MessageType::SetEditChannel:
			currentChannelEditingIx = recvMsg.channel;
			*reloadMatrix = true; // Refresh our display matrix
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set Edit Channel: %d.", currentChannelEditingIx);
#endif
			// Update our knob
			controlKnobs[KnobIx::EditChannelKnob]->value = currentChannelEditingIx;
			controlKnobs[KnobIx::EditChannelKnob]->dirty = true;
			params[ParamIds::SELECTED_CHANNEL_PARAM].value = currentChannelEditingIx;
			break;
		case TSExternalControlMessage::MessageType::TogglePlayMode:
			// -- Control Mode: Edit / Performance Mode --
			currentCtlMode = (currentCtlMode == ExternalControllerMode::EditMode) ? ExternalControllerMode::PerformanceMode : ExternalControllerMode::EditMode;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Toggle Control Mode: %d.", currentCtlMode);
#endif
			break;
		case TSExternalControlMessage::MessageType::SetPlayMode:
			// -- Control Mode: Edit / Performance Mode --
			currentCtlMode = static_cast<ExternalControllerMode>(recvMsg.mode);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set Control Mode: %d.", currentCtlMode);
#endif
			break;
		case TSExternalControlMessage::MessageType::StorePlayBPM:
			if (storedBPM != recvMsg.mode)
			{
				storedBPMChanged = true;
				storedBPM = recvMsg.mode;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Store BPM (%d): %.2f.", recvMsg.mode, storedBPM);
#endif
			}
			break;
			// Now BPM and Tempo are separate in OSC.
			/// TODO: Maybe move the calcs to the listener thread? But if we have other external messages from non-OSC, then it would have to repeated somewhere...
		case TSExternalControlMessage::MessageType::SetPlayBPM: // "BPM" is relative to the note
																//float bpm = recvMsg.mode;
																// currentBPM = 2^knob * mult
																// currentBPM / mult = 2 ^ knob
																// log2(currentBPM / mult) = knob
			if (recvMsg.mode == TROWA_INDEX_UNDEFINED)
				recvMsg.mode = storedBPM;
			tmp = clamp(std::log2f(recvMsg.mode / BPMOptions[selectedBPMNoteIx]->multiplier), static_cast<float>(TROWA_SEQ_BPM_KNOB_MIN), static_cast<float>(TROWA_SEQ_BPM_KNOB_MAX));
			controlKnobs[KnobIx::BPMKnob]->value = tmp;
			controlKnobs[KnobIx::BPMKnob]->dirty = true;
			params[ParamIds::BPM_PARAM].value = tmp;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set BPM (%d): %.2f.", recvMsg.mode, tmp);
#endif
			break;
		case TSExternalControlMessage::MessageType::AddPlayBPM: // "BPM" is relative to the note
			tmp = pow(2, controlKnobs[KnobIx::BPMKnob]->value) // Current BPM
				+ recvMsg.mode / BPMOptions[selectedBPMNoteIx]->multiplier;
			tmp = std::log2f(tmp);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Add BPM (%d): Knob %.2f, End is %.2f", recvMsg.mode, controlKnobs[KnobIx::BPMKnob]->value, tmp);
#endif
			controlKnobs[KnobIx::BPMKnob]->value = clamp(tmp, static_cast<float>(TROWA_SEQ_BPM_KNOB_MIN), static_cast<float>(TROWA_SEQ_BPM_KNOB_MAX));
			controlKnobs[KnobIx::BPMKnob]->dirty = true;
			params[ParamIds::BPM_PARAM].value = tmp;
			break;
		case TSExternalControlMessage::MessageType::SetPlayTempo: // Tempo goes from 0 to 1
			tmp = rescale(recvMsg.val, 0.0, 1.0, TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX);
			controlKnobs[KnobIx::BPMKnob]->value = tmp;
			controlKnobs[KnobIx::BPMKnob]->dirty = true;
			params[ParamIds::BPM_PARAM].value = tmp;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set Tempo (%.2f): Knob %.2f.", recvMsg.val, tmp);
#endif
			break;
		case TSExternalControlMessage::MessageType::AddPlayTempo: // Tempo goes from 0 to 1
			tmp = rescale(recvMsg.val, 0.0, 1.0, TROWA_SEQ_BPM_KNOB_MIN, TROWA_SEQ_BPM_KNOB_MAX);
			controlKnobs[KnobIx::BPMKnob]->value = clamp(tmp + controlKnobs[KnobIx::BPMKnob]->value, static_cast<float>(TROWA_SEQ_BPM_KNOB_MIN), static_cast<float>(TROWA_SEQ_BPM_KNOB_MAX));
			controlKnobs[KnobIx::BPMKnob]->dirty = true;
			params[ParamIds::BPM_PARAM].value = tmp;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Add Tempo (%.2f): Knob %.2f.", recvMsg.val, controlKnobs[KnobIx::BPMKnob]->value);
#endif
			break;
		case TSExternalControlMessage::MessageType::AddPlayBPMNote:
			selectedBPMNoteIx = (selectedBPMNoteIx + recvMsg.step) % TROWA_TEMP_BPM_NUM_OPTIONS;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Add %d to BPM Note Ix: %d.", recvMsg.step, selectedBPMNoteIx);
#endif
			break;
		case TSExternalControlMessage::MessageType::SetPlayBPMNote:
			selectedBPMNoteIx = recvMsg.step % TROWA_TEMP_BPM_NUM_OPTIONS;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set BPM Note Ix: %d.", selectedBPMNoteIx);
#endif
			break;
		case TSExternalControlMessage::MessageType::StorePlayLength:
			if (recvMsg.step != storedNumberSteps)
			{
				storedNumberSteps = recvMsg.step;
				storedLengthChanged = true;
			}
			break;
		case TSExternalControlMessage::MessageType::SetPlayLength:
			// Use our stored value if sent is -1
			recvMsg.step = (recvMsg.step == TROWA_INDEX_UNDEFINED) ? storedNumberSteps : recvMsg.step;
			if (recvMsg.step > 0 && recvMsg.step <= maxSteps)
			{
				currentNumberSteps = recvMsg.step;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Set Play Step Length: %d.", currentNumberSteps);
#endif
				// Update our knob
				controlKnobs[KnobIx::StepLengthKnob]->value = currentNumberSteps;
				controlKnobs[KnobIx::StepLengthKnob]->dirty = true;
				params[ParamIds::STEPS_PARAM].value = currentNumberSteps;
			}
			break;
		case TSExternalControlMessage::MessageType::PasteEditClipboard:
			doPaste = true;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Paste message.");
#endif
			break;
		case TSExternalControlMessage::MessageType::CopyEditPattern:
		{
			int pat = (recvMsg.pattern == CURRENT_EDIT_PATTERN_IX) ? currentPatternEditingIx : recvMsg.pattern;
			if (copySourcePatternIx > -1 && copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
			{
				// Clear clipboard 
				clearClipboard();
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("(clear clipboard) Copy Edit Pattern: %d.", pat);
#endif
			}
			else
			{
				//int c = (recvMsg.channel == CURRENT_EDIT_CHANNEL_IX) ? currentChannelEditingIx : recvMsg.channel;
				copy(pat, TROWA_SEQ_COPY_CHANNELIX_ALL);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				pasteLight->setColor(COLOR_WHITE);
				lights[COPY_PATTERN_LIGHT].value = 1; // Light up Pattern Copy as Active clipboard
				lights[COPY_CHANNEL_LIGHT].value = 0;	// Inactivate Gate Copy light
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Copy Edit Pattern: %d.", pat);
#endif
			}
		}
		break;
		case TSExternalControlMessage::MessageType::CopyEditChannel:
		{
			int pat = (recvMsg.pattern == CURRENT_EDIT_PATTERN_IX) ? currentPatternEditingIx : recvMsg.pattern;
			int ch = (recvMsg.channel == CURRENT_EDIT_CHANNEL_IX) ? currentChannelEditingIx : recvMsg.channel;
			if (copySourcePatternIx > -1 && copySourceChannelIx > -1)
			{
				// Clear clipboard 
				clearClipboard();
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("(clear clipboard) Copy Edit Channel: (P:%d, C:%d).", pat, ch);
#endif
			}
			else
			{
				copy(pat, ch);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				pasteLight->setColor(voiceColors[currentChannelEditingIx]);
				lights[COPY_CHANNEL_LIGHT].value = 1;		// Light up Channel Copy Light as Active clipboard
				copyGateLight->setColor(voiceColors[currentChannelEditingIx]); // Match the color with our Channel color
				lights[COPY_PATTERN_LIGHT].value = 0; // Inactivate Pattern Copy Light
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Copy Edit Channel: (P:%d, C:%d).", pat, ch);
#endif
			}

		}
		break;
		case TSExternalControlMessage::MessageType::SetPlayRunningState:
		case TSExternalControlMessage::MessageType::TogglePlayRunningState:
			if (recvMsg.messageType == TSExternalControlMessage::MessageType::TogglePlayRunningState)
			{
				running = !running;
			}
			else
			{
				running = recvMsg.val > 0;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Set Running to: %d.", running);
#endif
			runningTrigger.state = (running) ? SchmittTrigger::HIGH : SchmittTrigger::LOW;
			params[RUN_PARAM].value = running;
			lights[RUNNING_LIGHT].value = running ? 1.0 : 0.0;
			break;
		case TSExternalControlMessage::MessageType::RandomizeEditStepValue:
			randomize();
			break;
		case TSExternalControlMessage::MessageType::InitializeEditModule:
			reset();
			for (int i = 0; i < KnobIx::NumKnobs; i++)
			{
				controlKnobs[i]->value = controlKnobs[i]->defaultValue;
				controlKnobs[i]->dirty = true;
			}
			// We also need to make sure our controls reset....
			//Module::reset(); // Base method reset should do the knobs
			//_ParentWidget->reset();
			/// TODO: We should also send our new values to OSC if OSC is enabled. We would have to re-read the vals though,
			/// I think the values should trigger that they changed next step()... TODO: double check that this happens
			break;
		default:
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("Ooops - didn't handle this control message type yet %d.", recvMsg.messageType);
#endif
			break;
		} // end switch
	} // end loop through message queue	


	//-- COPY / PASTE --
	bool pasteCompleted = false;
	if (pasteTrigger.process(params[PASTE_PARAM].value) || doPaste)
	{
		pasteCompleted = paste(); // Paste whatever we have if we have anything		
	}
	else
	{
		// Check Copy
		if (copyPatternTrigger.process(params[COPY_PATTERN_PARAM].value))
		{
			if (copySourcePatternIx > -1 && copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
			{
				// Clear clipboard 
				clearClipboard();
			}
			else
			{
				copy(currentPatternEditingIx, TROWA_SEQ_COPY_CHANNELIX_ALL);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				pasteLight->setColor(COLOR_WHITE);
				lights[COPY_PATTERN_LIGHT].value = 1; // Light up Pattern Copy as Active clipboard
				lights[COPY_CHANNEL_LIGHT].value = 0;	// Inactivate Gate Copy light				
			}
		}
		if (copyGateTrigger.process(params[COPY_CHANNEL_PARAM].value))
		{
			if (copySourcePatternIx > -1 && copySourceChannelIx > -1)
			{
				// Clear clipboard 
				clearClipboard();
			}
			else
			{
				copy(currentPatternEditingIx, currentChannelEditingIx);
				lights[PASTE_LIGHT].value = 1;	// Activate paste light to show there is something on the clipboard
				pasteLight->setColor(voiceColors[currentChannelEditingIx]);
				lights[COPY_CHANNEL_LIGHT].value = 1;		// Light up Channel Copy Light as Active clipboard
				copyGateLight->setColor(voiceColors[currentChannelEditingIx]); // Match the color with our Channel color
				lights[COPY_PATTERN_LIGHT].value = 0; // Inactivate Pattern Copy Light				
			}
		} // end if copyGateTrigger()
	}




	// Check value mode change after we have processed incoming messages.
	*valueModeChanged = (lastOutputValueMode != selectedOutputValueMode);
	lastOutputValueMode = selectedOutputValueMode;

	// Reset
	// [03/30/2018] So, now j4s0n wants RESET to wait until the next step is played... 
	// So it's delayed reset. https://github.com/j4s0n-c/trowaSoft-VCV/issues/11
	if (resetTrigger.process(params[RESET_PARAM].value + inputs[RESET_INPUT].value) || resetMsg)
	{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
		debug("Reset");
#endif
		resetPaused = !running;
		// [03/30/2018] Delay reset until the next step. (https://github.com/j4s0n-c/trowaSoft-VCV/issues/11)
		// So it's more like JUMP TO step 0 (waits until the next step).
		resetQueued = true; // Flag that the reset has been queued.
	} // end check for reset

	if (resetQueued && nextStep) {
		resetQueued = false; 
		realPhase = 0.0;
		swingAdjustedPhase = 0; // Reset swing		
		index = 999;
		nextStep = true;
		lights[RESET_LIGHT].value = 1.0;
		nextIndex = TROWA_INDEX_UNDEFINED; // Reset our jump to index
		oscMutex.lock();
		if (useOSC && oscInitialized)
		{
			osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
			oscStream << osc::BeginBundleImmediate
				<< osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayReset])
				<< "bang" << osc::EndMessage
				<< osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
	} // end if resetQueued and it's time to reset

	// Next Step
	if (nextStep)
	{
		if (nextIndex == TROWA_INDEX_UNDEFINED)
			index++; // Advance step
		else
		{
			index = nextIndex; // Set to our 'jump to' value
			nextIndex = TROWA_INDEX_UNDEFINED; // Reset our jump to index
		}
		if (index >= currentNumberSteps || index < 0) {
			index = 0; // Reset (artifical limit)
		}
		// Show which step we are on:
		r = index / this->numCols;// TROWA_SEQ_STEP_NUM_COLS;
		c = index % this->numCols; //TROWA_SEQ_STEP_NUM_COLS;
		stepLights[r][c] = 1.0f;
		gatePulse.trigger(TROWA_PULSE_WIDTH);

		oscMutex.lock();
		if (useOSC && oscInitialized)
		{
			// [01/06/2018] Changed to one-based for OSC (send index+1 instead of index)
			osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
			oscStream << osc::BeginBundleImmediate
				<< osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayClock])
				<< index + 1 << osc::EndMessage
				<< osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
	} // end if next step

	// If we were just unpaused and we were reset during the pause, make sure we fire the first step.
	if (running && !lastRunning)
	{
		if (resetPaused)
		{
			gatePulse.trigger(TROWA_PULSE_WIDTH);
		}
		resetPaused = false;
	} // end if

	// Reset light
	lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / lightLambda / engineGetSampleRate();
	// BPM Note Calc light:
	lights[SELECTED_BPM_MULT_IX_LIGHT].value -= lights[SELECTED_BPM_MULT_IX_LIGHT].value / lightLambda / engineGetSampleRate();
	*pulse = gatePulse.process(engineGetSampleTime());// 1.0 / engineGetSampleRate());

	editChannelChanged = currentChannelEditingIx != lastChannelIx;
	editPatternChanged = currentPatternEditingIx != lastEditPatternIx;
	// See if we should reload our matrix
	//*reloadMatrix = currentChannelEditingIx != lastChannelIx || currentPatternEditingIx != lastEditPatternIx || pasteCompleted || this->reloadEditMatrix || firstLoad;
	*reloadMatrix = editChannelChanged || editPatternChanged || pasteCompleted || this->reloadEditMatrix || firstLoad || oscStarted;


	// Send messages if needed
	/// TODO: Make a message sender to do this crap
	oscMutex.lock();
	if (useOSC && oscInitialized)
	{
		bool bundleOpened = false;
		// If something has changed or we just started up osc, then send the status of our sequencer.
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		if (lastRunning != running || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayRunningState])
				<< (int)(running)
				<< osc::EndMessage;
			// Send another toggle message for touchOSC
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayToggleRun])
				<< (int)(running)
				<< osc::EndMessage;
		}
		if (lastPatternPlayingIx != currentPatternPlayingIx || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayPattern])
				<< (currentPatternPlayingIx + 1)
				<< osc::EndMessage;
		}
		if (playBPMChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayBPM])
				<< currentBPM << this->selectedBPMNoteIx
				<< osc::EndMessage;
		}
		if (lastNumberSteps != currentNumberSteps || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayLength])
				<< currentNumberSteps
				<< osc::EndMessage;
		} // end playLengthChanged
		if (*valueModeChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayOutputMode])
				<< (int)(this->selectedOutputValueMode)
				<< osc::EndMessage;
		} // end playOutputModeChanged
		if (editPatternChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditPattern])
				<< (currentPatternEditingIx + 1)
				<< osc::EndMessage;
		} // end editPatternChanged
		if (editChannelChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditChannel])
				<< (currentChannelEditingIx + 1)
				<< osc::EndMessage;
		} // end editChannelChanged
		if (lastBPMNoteIx != this->selectedBPMNoteIx || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayBPMNote])
				<< selectedBPMNoteIx
				<< osc::EndMessage;
		} // end bpmNoteChanged

		if (storedPatternChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayPatternSav])
				<< ((storedPatternPlayingIx > -1) ? storedPatternPlayingIx + 1 : currentPatternPlayingIx + 1)
				<< osc::EndMessage;
		} // end storedPatternChanged

		if (storedLengthChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayLengthSav])
				<< ((storedNumberSteps > 0) ? storedNumberSteps : currentNumberSteps)
				<< osc::EndMessage;
		} // end storedPatternChanged

		if (storedBPMChanged || oscStarted)
		{
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::PlayBPMSav])
				<< ((storedBPM > 0) ? storedBPM : (int)(currentBPM))
				<< osc::EndMessage;
		}


		if (copySourcePatternIx != prevCopyPatternIx || copySourceChannelIx != prevCopyChannelIx || oscStarted)
		{
			// Clipboard has changed
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}

			if (copySourcePatternIx == TROWA_INDEX_UNDEFINED)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Sending Clear Clipboard: %s %d.", oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr], 0);
#endif
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditClipboard])
					<< 0
					<< 0
					<< osc::EndMessage;

				// Clipboard was cleared
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr])
					<< 0
					<< osc::EndMessage;
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditPatternCpyCurr])
					<< 0
					<< osc::EndMessage;
			}
			else
			{
				// Send clipboard message (pattern, channel)
				oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditClipboard])
					<< (copySourcePatternIx + 1)
					<< (copySourceChannelIx + 1)
					<< osc::EndMessage;
				if (copySourceChannelIx == TROWA_SEQ_COPY_CHANNELIX_ALL)
				{
					// Pattern copied (pattern)
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Sending Copied Pattern: %s %d.", oscAddrBuffer[SeqOSCOutputMsg::EditPatternCpyCurr], (copySourcePatternIx + 1));
#endif
					oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditPatternCpyCurr])
						<< (copySourcePatternIx + 1)
						<< osc::EndMessage;
					oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr])
						<< 0
						<< osc::EndMessage;
				}
				else
				{
					// Channel copied (channel)
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Sending Copied Channel: %s %d.", oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr], (copySourceChannelIx + 1));
#endif
					oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditPatternCpyCurr])
						<< 0
						<< osc::EndMessage;
					oscStream << osc::BeginMessage(oscAddrBuffer[SeqOSCOutputMsg::EditChannelCpyCurr])
						<< (copySourceChannelIx + 1)
						<< osc::EndMessage;
				}
			}// end else
		} // end if clipboard change

#if OSC_UPDATE_CURRENT_STEP_LED
		if (lastStepIndex != index)
		{
			// Turn off last led, turn on this led
			if (!bundleOpened)
			{
				oscStream << osc::BeginBundleImmediate;
				bundleOpened = true;
			}
			char addrBuff[50] = { 0 };
			// Prev step should turn off:
			sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], lastStepIndex + 1);
			oscStream << osc::BeginMessage(addrBuff)
				<< 0
				<< osc::EndMessage;
			// Current step should turn on:
			sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], index + 1);
			oscStream << osc::BeginMessage(addrBuff)
				<< 1
				<< osc::EndMessage;
		}
#endif

		//--- FINISH BUNDLE ---
		if (bundleOpened)
		{
			// Finish and send
			oscStream << osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
	} // end send osc
	oscMutex.unlock();

	firstLoad = false;
	return;
} // end getStepInputs()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// toJson(void)
// Save our junk to json.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
json_t *TSSequencerModuleBase::toJson() {
	json_t *rootJ = json_object();

	// version
	json_object_set_new(rootJ, "version", json_integer(TROWA_INTERNAL_VERSION_INT));

	// running
	json_object_set_new(rootJ, "running", json_boolean(running));

	// Current Items:
	json_object_set_new(rootJ, "currentPatternEditIx", json_integer((int)currentPatternEditingIx));
	json_object_set_new(rootJ, "currentTriggerEditIx", json_integer((int)currentChannelEditingIx));
	// The current output / knob mode.
	json_object_set_new(rootJ, "selectedOutputValueMode", json_integer((int)selectedOutputValueMode));
	// Current BPM calculation note (i.e. 1/4, 1/8, 1/8T, 1/16)
	json_object_set_new(rootJ, "selectedBPMNoteIx", json_integer((int)selectedBPMNoteIx));

	// triggers
	json_t *triggersJ = json_array();
	for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
	{
		for (int t = 0; t < TROWA_SEQ_NUM_CHNLS; t++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				json_t *gateJ = json_real((float)triggerState[p][t][s]);
				json_array_append_new(triggersJ, gateJ);
			} // end for (steps)
		} // end for (triggers)
	} // end for (patterns)
	json_object_set_new(rootJ, "triggers", triggersJ);

	// gateMode
	json_t *gateModeJ = json_integer((int)gateMode);
	json_object_set_new(rootJ, "gateMode", gateModeJ);

	// OSC Parameters
	json_t* oscJ = json_object();
	json_object_set_new(oscJ, "IpAddress", json_string(this->currentOSCSettings.oscTxIpAddress.c_str()));
	json_object_set_new(oscJ, "TxPort", json_integer(this->currentOSCSettings.oscTxPort));
	json_object_set_new(oscJ, "RxPort", json_integer(this->currentOSCSettings.oscRxPort));
	json_object_set_new(oscJ, "Client", json_integer(this->oscCurrentClient));
	json_object_set_new(oscJ, "AutoReconnectAtLoad", json_boolean(oscReconnectAtLoad)); // [v11, v0.6.3]
	json_object_set_new(oscJ, "Initialized", json_boolean(oscInitialized)); // [v11, v0.6.3] We know the settings are good at least at the time of save
	json_object_set_new(rootJ, "osc", oscJ);

	return rootJ;
} // end toJson()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// fromJson(void)
// Read in our junk from json.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-	
void TSSequencerModuleBase::fromJson(json_t *rootJ) {
	// running
	json_t *runningJ = json_object_get(rootJ, "running");
	if (runningJ)
		running = json_is_true(runningJ);

	// Current Items:
	json_t *currJ = NULL;
	currJ = json_object_get(rootJ, "currentPatternEditIx");
	if (currJ)
		currentPatternEditingIx = json_integer_value(currJ);
	currJ = json_object_get(rootJ, "currentTriggerEditIx");
	if (currJ)
		currentChannelEditingIx = json_integer_value(currJ);
	currJ = json_object_get(rootJ, "selectedOutputValueMode");
	if (currJ)
	{
		selectedOutputValueMode = static_cast<ValueMode>(json_integer_value(currJ));
		modeString = modeStrings[selectedOutputValueMode];
	}
	// Current BPM calculation note (i.e. 1/4, 1/8, 1/8T, 1/16)
	currJ = json_object_get(rootJ, "selectedBPMNoteIx");
	if (currJ)
		selectedBPMNoteIx = json_integer_value(currJ);

	// triggers
	json_t *triggersJ = json_object_get(rootJ, "triggers");
	if (triggersJ)
	{
		int i = 0;
		for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
		{
			for (int t = 0; t < TROWA_SEQ_NUM_CHNLS; t++)
			{
				for (int s = 0; s < maxSteps; s++)
				{
					json_t *gateJ = json_array_get(triggersJ, i++);
					if (gateJ)
						triggerState[p][t][s] = (float)json_real_value(gateJ);
				} // end for (steps)
			} // end for (triggers)
		} // end for (patterns)			
	}
	// gateMode
	json_t *gateModeJ = json_object_get(rootJ, "gateMode");
	if (gateModeJ)
		gateMode = (GateMode)json_integer_value(gateModeJ);

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
		currJ = json_object_get(oscJ, "Client");
		if (currJ)
			this->oscCurrentClient = static_cast<OSCClient>((uint8_t)(json_integer_value(currJ)));
		currJ = json_object_get(oscJ, "AutoReconnectAtLoad");
		if (currJ)
			oscReconnectAtLoad = json_boolean_value(currJ);
		if (oscReconnectAtLoad)
		{
			currJ = json_object_get(oscJ, "Initialized");
			if (currJ && json_boolean_value(currJ))
			{
				oscCurrentAction = OSCAction::Enable; // Will enable at next step
				//// Try to reconnect
				//cleanupOSC();
				//this->initOSC(this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);

				//if (oscError || !oscInitialized)
				//{
				//	warn("TSSequencerModuleBase::fromJson(): Error on auto-reconnect OSC %s :%d :%d.", this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);
				//}
				//else
				//{
				//	info("TSSequencerModuleBase::fromJson(): Successful auto-reconnection of OSC %s :%d :%d.", this->currentOSCSettings.oscTxIpAddress.c_str(), this->currentOSCSettings.oscTxPort, this->currentOSCSettings.oscRxPort);
				//}
			}
		}
	}

	saveVersion = 0;
	currJ = NULL;
	currJ = json_object_get(rootJ, "version");
	if (currJ)
	{
		saveVersion = (int)(json_integer_value(currJ));
	}
	firstLoad = true;
	return;
} // end fromJson()
