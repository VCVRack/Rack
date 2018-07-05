#include <string.h>
#include <vector>
#include <stdio.h>
#include <exception>
#include "util/common.hpp"
#include "util/math.hpp" //util.hpp"
#include "TSOSCSequencerListener.hpp"
#include "TSSequencerModuleBase.hpp"
#include "trowaSoftUtilities.hpp" // For debug
#include "TSOSCCommon.hpp"

#define OSC_MSG_SRC		TSExternalControlMessage::MessageSource::OSC


inline TSExternalControlMessage CreateOSCRecvMsg(TSExternalControlMessage::MessageType msgType)
{
	TSExternalControlMessage msg;
	msg.messageSource = OSC_MSG_SRC;
	msg.messageType = msgType;
	return msg;
}
inline TSExternalControlMessage CreateOSCRecvMsg(TSExternalControlMessage::MessageType msgType, int mode)
{
	TSExternalControlMessage msg;
	msg.messageSource = OSC_MSG_SRC;
	msg.messageType = msgType;
	msg.mode = mode;
	return msg;
}
inline TSExternalControlMessage CreateOSCRecvMsg(TSExternalControlMessage::MessageType msgType, int pattern, int channel, int step, float val)
{
	TSExternalControlMessage msg;
	msg.messageSource = OSC_MSG_SRC;
	msg.messageType = msgType;
	msg.pattern = pattern;
	msg.channel = channel;
	msg.step = step;
	msg.val = val;
	return msg;
}
inline TSExternalControlMessage CreateOSCRecvMsg(TSExternalControlMessage::MessageType msgType, int pattern, int channel, int step, float val, int mode)
{
	TSExternalControlMessage msg;
	msg.messageSource = OSC_MSG_SRC;
	msg.messageType = msgType;
	msg.pattern = pattern;
	msg.channel = channel;
	msg.step = step;
	msg.val = val;
	msg.mode = mode;
	return msg;
}
TSOSCSequencerListener::TSOSCSequencerListener()
{
	return;
}
//--------------------------------------------------------------------------------------------------------------------------------------------
// ProcessMessage()
// @rxMsg : (IN) The received message from the OSC library.
// @remoteEndPoint: (IN) The remove end point (sender).
// Handler for receiving messages from the OSC library. Taken from their example listener.
// Should create a generic TSExternalControlMessage for our trowaSoft sequencers and dump it in the module instance's queue.
//--------------------------------------------------------------------------------------------------------------------------------------------
void TSOSCSequencerListener::ProcessMessage(const osc::ReceivedMessage& rxMsg, const IpEndpointName& remoteEndpoint) 
{
	(void)remoteEndpoint; // suppress unused parameter warning
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
	debug("[RECV] OSC Message: %s", rxMsg.AddressPattern());
#endif
	osc::int32 step = -1;
	float stepVal = 0.0;
	osc::int32 pattern = CURRENT_EDIT_PATTERN_IX;
	osc::int32 channel = CURRENT_EDIT_CHANNEL_IX;
	osc::int32 intVal = -1;
	try {
		const char* ns = this->oscNamespace.c_str();
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

		std::vector<std::string> parts = str_split(subAddr.substr(1), '/');
		int numParts = parts.size();

#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
		debug("[RECV] %s - %d parts.", path, numParts);
#endif


		//int i = 0;
		//if (parts[i].compare("edit") == 0)
		//{
		//	i++;
		//	// Edit messages
		//	if (parts[i].compare("step"))
		//}
		//else if (parts[i].compare("play") == 0)
		//{
		//	// Play messages
		//	i++;
		//}

		int row = 0, col = 0;

		/// TODO: Try to order in order of frequency/commonality of the messages
		/// TODO: Do better/more efficient parsing (tree)
		if (std::strcmp(path, OSC_RANDOMIZE_EDIT_STEPVALUE) == 0)
		{
			// Set Randomize ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			// No params
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Randomize Current Edit Channel.", rxMsg.AddressPattern());
#endif
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::RandomizeEditStepValue));
		}
		else if (std::strncmp(path, OSC_SET_EDIT_GRIDSTEP, strlen(OSC_SET_EDIT_GRIDSTEP)) == 0)
		{
			// For touchOSC, a multi control grid.
			// /edit/stepgrid/<row>/<col>
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			pattern = CURRENT_EDIT_PATTERN_IX;
			channel = CURRENT_EDIT_CHANNEL_IX;
			try
			{
				row = std::stoi(parts[2]);
				col = std::stoi(parts[3]);
				args >> stepVal >> osc::EndMessage;
			}
			catch (const std::exception& ex)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Error getting step or stepvalue: %s\n%s", path, ex.what());
#endif
			}
			// --* touchOSC *--
			// Grid control is addressed by /row/col (1-based) and starts from the bottom (to top) and goes left to right.
			// Convert to our step #.
			// ASSUMPTION: We assume that the touchOSC multi control has the same # of cols and # rows as the module.
			step = touchOSC::mcRowCol_to_stepIndex(row, col, sequencerModule->numRows, sequencerModule->numCols);
			step = (int)clamp(step, 0, sequencerModule->maxSteps - 1);
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Row %d, Col %d => Ix %d. Value is %0.2f.", path, row, col, step, stepVal);
#endif
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetEditStepValue, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_TOGGLE_EDIT_STEPVALUE) == 0 || std::strncmp(path, OSC_SET_EDIT_STEPVALUE, strlen(OSC_SET_EDIT_STEPVALUE)) == 0)
		{
			// Set Step Value ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			// Normal:
			// /edit/step int stepNumber, float value, int pattern, int channel
			// or
			// /edit/step/tog int stepNumber
			// or
			// /edit/step/<step> float value  (touchOSC)
			// or
			// /edit/step/mtog/<row>/<col> (ignore any params) (touchOSC)
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			stepVal = 1.0;
			TSExternalControlMessage::MessageType messageType = TSExternalControlMessage::MessageType::SetEditStepValue;
			if (std::strcmp(path, OSC_TOGGLE_EDIT_STEPVALUE) == 0)
			{
				// /edit/step/tog int stepNumber
				messageType = TSExternalControlMessage::MessageType::ToggleEditStepValue;
			}
			if (numParts > 2 && messageType == TSExternalControlMessage::MessageType::SetEditStepValue)
			{
				// /edit/step/<step> float value
				// touchOSC will have to send the step number in the path because it can't send > 1 arg
				pattern = CURRENT_EDIT_PATTERN_IX;
				channel = CURRENT_EDIT_CHANNEL_IX;
				try
				{
					step = std::stoi(parts[2]);
					args >> stepVal >> osc::EndMessage;
				}
				catch (const std::exception& ex)
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Error getting step or stepvalue: %s\n%s", path, ex.what());
#endif
				}
			}
			else
			{
				try
				{
					// We should always get step and stepVal, but we should allow pattern & channel to be optional
					args >> step >> stepVal >> pattern >> channel >> osc::EndMessage;
				}
				catch (osc::MissingArgumentException& ex)
				{
					pattern = CURRENT_EDIT_PATTERN_IX;
					channel = CURRENT_EDIT_CHANNEL_IX;
				}
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Step %d, val %f (Pattern %d, Channel %d).", path, step, stepVal, pattern, channel);
#endif
			if (step > -1)
			{
				// If we at least have a step.
				step = (int)clamp(step, 1, sequencerModule->maxSteps) - 1;
				if (pattern != CURRENT_EDIT_PATTERN_IX)
					pattern = (int)clamp(pattern, 1, TROWA_SEQ_NUM_PATTERNS) - 1;
				if (channel != CURRENT_EDIT_CHANNEL_IX)
					channel = (int)clamp(channel, 1, TROWA_SEQ_NUM_CHNLS) - 1;
				// Queue up this message.
				sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(messageType, pattern, channel, step, stepVal));
			}
		}
		else if (std::strcmp(path, OSC_STORE_PLAY_PATTERN) == 0)
		{
			// Store Playing Pattern :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int pattern : 1-64
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			try
			{
				// touchOSC is VERY limited and will just send floats
				args >> pattern >> osc::EndMessage;
			}
			catch (osc::WrongArgumentTypeException touchOSCEx)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Wrong argument type: Error %s message: ", path, touchOSCEx.what());
				debug("We received %d.", pattern);
#endif
			}

#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Pattern %d.", path, pattern);
#endif
			pattern = (int)clamp(pattern, 1, TROWA_SEQ_NUM_PATTERNS) - 1;
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::StorePlayPattern, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_PATTERN) == 0)
		{
			// Set Playing Pattern :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int pattern : 1-64
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			try
			{
				// touchOSC is VERY limited and will just send floats
				args >> pattern >> osc::EndMessage;
			}
			catch (osc::WrongArgumentTypeException touchOSCEx)
			{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Wrong argument type: Error %s message: ", path, touchOSCEx.what());
				debug("We received %d.", pattern);
#endif
			}

#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Pattern %d.", path, pattern);
#endif
			if (pattern != CURRENT_EDIT_PATTERN_IX)
			{
				pattern = (int)clamp(pattern, 1, TROWA_SEQ_NUM_PATTERNS) - 1;
			}
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayPattern, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_CURRENTSTEP) == 0)
		{
			// Set Playing Step/Jump :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int step : 1-16
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Step %d.", path, step);
#endif
			step = (int)clamp(step, 1, sequencerModule->maxSteps) - 1;
			/// TODO: Should we purge the queue so this guaranteed to happen immediately?
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayCurrentStep, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_EDIT_PATTERN) == 0)
		{
			// Set Editing Pattern :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int pattern : 1-16
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> pattern >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Pattern %d.", path, pattern);
#endif
			pattern = (int)clamp(pattern, 1, TROWA_SEQ_NUM_PATTERNS) - 1;
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetEditPattern, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_EDIT_CHANNEL) == 0)
		{
			// Set Editing Channel :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int channel : 1-16
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> channel >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Channel %d.", path, channel);
#endif
			channel = (int)clamp(channel, 1, TROWA_SEQ_NUM_CHNLS) - 1;
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetEditChannel, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_OUTPUTMODE) == 0)
		{
			// Set Output Mode (TRIG, RTRIG, GATE) or (VOLT, NOTE, PATT) :::::::::::::::::::::::::::::::::::::::::::::::::::::
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> intVal >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Output Mode %d.", path, intVal);
#endif
			intVal = (int)clamp(intVal, TSSequencerModuleBase::ValueMode::MIN_VALUE_MODE, TSSequencerModuleBase::ValueMode::MAX_VALUE_MODE);
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayOutputMode, /*mode*/ intVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_RESET) == 0)
		{
			// Reset :::::::::::::::::::::::::::::::::::::::::::::::::::::
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message", path);
#endif
			stepVal = 1;
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayReset, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_PASTE_EDIT_CLIPBOARD) == 0)
		{
			// Copy Edit Pattern ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message.", path);
#endif
			// Queue up this message.
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::PasteEditClipboard, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_LENGTH) == 0)
		{
			// Set Play Length :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int step
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Step Length %d.", path, step);
#endif
			if (step != TROWA_INDEX_UNDEFINED)
			{
				step = (int)clamp(step, 1, sequencerModule->maxSteps); // Must be 1 to 64 (not 0 to 63)
			}
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayLength, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_STORE_PLAY_LENGTH) == 0)
		{
			// Store Play Length :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int step
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Step Length %d.", path, step);
#endif
			step = (int)clamp(step, 1, sequencerModule->maxSteps); // Must be 1 to 64 (not 0 to 63)
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::StorePlayLength, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_TOGGLE_PLAY_RUNNINGSTATE) == 0 || std::strcmp(path, OSC_SET_PLAY_RUNNINGSTATE) == 0)
		{
			// Set Playing State ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			try
			{
				// We may or not may not always get a value (we should really check on SET, but oh well).
				args >> intVal >> osc::EndMessage;
			}
			catch (osc::MissingArgumentException& ex)
			{
				intVal = 1;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Play Val %d.", path, intVal);
#endif
			if (intVal > -1)
			{
				TSExternalControlMessage::MessageType messageType = TSExternalControlMessage::MessageType::SetPlayRunningState;
				if (std::strcmp(path, OSC_TOGGLE_PLAY_RUNNINGSTATE) == 0)
					messageType = TSExternalControlMessage::MessageType::TogglePlayRunningState;
				// Queue up this message.
				sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(messageType, pattern, channel, step, intVal));
			}
		}
		else if (std::strcmp(path, OSC_TOGGLE_PLAY_MODE) == 0 || std::strcmp(path, OSC_SET_PLAY_MODE) == 0)
		{
			// Set Control Mode ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			try
			{
				// We may or not may not always get a value (we should really check on SET, but oh well).
				args >> intVal >> osc::EndMessage;
			}
			catch (osc::MissingArgumentException& ex)
			{
				intVal = 1;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Control Mode Val %d.", rxMsg.AddressPattern(), intVal);
#endif
			if (intVal > -1)
			{
				TSExternalControlMessage::MessageType messageType = TSExternalControlMessage::MessageType::SetPlayMode;
				if (std::strcmp(rxMsg.AddressPattern(), OSC_TOGGLE_PLAY_MODE) == 0)
					messageType = TSExternalControlMessage::MessageType::TogglePlayMode;
				// Queue up this message.
				sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(messageType, pattern, channel, step, intVal, intVal));
			}
		}
		else if (std::strcmp(path, OSC_STORE_PLAY_BPM) == 0)
		{
			// Store BPM/tempo :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int bpm
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> intVal >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - BPM %d.", path, intVal);
#endif
			intVal = (int)clamp(intVal, 4, 5000); // Just make sure it's not 0 or negative or too crazy.
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::StorePlayBPM, pattern, channel, step, stepVal, /*mode*/ intVal));
		}
		else if (std::strcmp(path, OSC_SET_PLAY_BPM) == 0)
		{
			// Set BPM/tempo :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int bpm - changed to BPM from tempo
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> intVal >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - BPM %d.", path, intVal);
#endif
			if (intVal != TROWA_INDEX_UNDEFINED)
				intVal = (int)clamp(intVal, 4, 5000); // Just make sure it's not 0 or negative or too crazy.
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayBPM, pattern, channel, step, stepVal, /*mode*/ intVal));
		} // end if BPM
		else if (std::strcmp(path, OSC_ADD_PLAY_BPM) == 0)
		{
			// Add BPM :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int bpm - changed to BPM from tempo
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> intVal >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - BPM Add %d.", path, intVal);
#endif
			intVal = (int)clamp(intVal, -5000, 5000); // Just make sure it's not too crazy
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::AddPlayBPM, pattern, channel, step, stepVal, /*mode*/ intVal));
		} // end if BPM Add
		else if (std::strcmp(path, OSC_SET_PLAY_TEMPO) == 0)
		{
			// Set Tempo :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//float tempo [0,1]
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> stepVal >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Tempo %.2f", path, stepVal);
#endif
			stepVal = clamp(stepVal, 0.0f, 1.0f); // Must be 0 to 1
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayTempo, pattern, channel, step, stepVal));
		} // end if Tempo
		else if (std::strcmp(path, OSC_COPYCURRENT_EDIT_PATTERN) == 0 || std::strcmp(path, OSC_COPY_EDIT_PATTERN) == 0)
		{
			// Copy Edit Pattern ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			if (std::strcmp(path, OSC_COPYCURRENT_EDIT_PATTERN))
			{
				pattern = CURRENT_EDIT_PATTERN_IX;
				channel = TROWA_SEQ_COPY_CHANNELIX_ALL;
			}
			else
			{
				osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
				try
				{
					// We may or not may not always get a value
					args >> pattern >> osc::EndMessage;
					if (pattern < 1)
						pattern = CURRENT_EDIT_PATTERN_IX;

				}
				catch (osc::MissingArgumentException& ex)
				{
					pattern = CURRENT_EDIT_PATTERN_IX;
				}
				if (pattern != CURRENT_EDIT_PATTERN_IX)
					pattern = (int)clamp(pattern, 1, TROWA_SEQ_NUM_PATTERNS) - 1;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Pattern %d.", path, pattern);
#endif
			// Queue up this message.
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::CopyEditPattern, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_COPY_EDIT_CHANNEL) == 0 || std::strcmp(path, OSC_COPYCURRENT_EDIT_CHANNEL) == 0)
		{
			// Copy Edit Channel ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			if (std::strcmp(path, OSC_COPYCURRENT_EDIT_CHANNEL) == 0)
			{
				pattern = CURRENT_EDIT_PATTERN_IX;
				channel = CURRENT_EDIT_CHANNEL_IX;
			}
			else 
			{
				osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
				try
				{
					// We may or not may not always get a value or both values
					args >> channel;
					if (channel < 1)
						channel = CURRENT_EDIT_CHANNEL_IX;
				}
				catch (osc::MissingArgumentException& ex)
				{
					channel = CURRENT_EDIT_CHANNEL_IX;
				}
				try
				{
					// We may or not may not always get a value or both values
					args >> pattern >> osc::EndMessage;
					if (pattern < 1)
						pattern = CURRENT_EDIT_PATTERN_IX;
				}
				catch (osc::MissingArgumentException& ex)
				{
					pattern = CURRENT_EDIT_PATTERN_IX;
				}
				if (pattern != CURRENT_EDIT_PATTERN_IX)
					pattern = (int)clamp(pattern, 1, TROWA_SEQ_NUM_PATTERNS) - 1;
				if (channel != CURRENT_EDIT_CHANNEL_IX)
					channel = (int)clamp(channel, 1, TROWA_SEQ_NUM_CHNLS) - 1;
			}
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Pattern %d, Channel %d.", path, pattern, channel);
#endif
			// Queue up this message.
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::CopyEditChannel, pattern, channel, step, stepVal));
		}
		else if (std::strcmp(path, OSC_ADD_PLAY_TEMPO) == 0)
		{
			// Add to Tempo :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//float tempo [0,1]
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> stepVal >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Tempo %.2f", path, stepVal);
#endif
			stepVal = clamp(stepVal, 0.0f, 1.0f); // Must be 0 to 1
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::AddPlayTempo, pattern, channel, step, stepVal));
		} // end if Tempo Add
		else if (std::strcmp(path, OSC_ADD_PLAY_BPMNOTE) == 0)
		{
			// Set BPM Note Index :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int bpmIx
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Add to BPM Note Ix %d.", path, step);
#endif
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::AddPlayBPMNote, pattern, channel, step, stepVal));
		} // end if BPMIncr
		else if (std::strcmp(path, OSC_SET_PLAY_BPMNOTE) == 0)
		{
			// Set BPM Note Index :::::::::::::::::::::::::::::::::::::::::::::::::::::
			//int bpmIx
			osc::ReceivedMessageArgumentStream args = rxMsg.ArgumentStream();
			args >> step >> osc::EndMessage;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - BPM Note Ix %d.", path, step);
#endif
			step = (int)clamp(step, 0, TROWA_TEMP_BPM_NUM_OPTIONS - 1); // Must be 0 to TROWA_TEMP_BPM_NUM_OPTIONS - 1
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::SetPlayBPMNote, pattern, channel, step, stepVal));
		} // end if BPMNote
		else if (std::strcmp(path, OSC_INITIALIZE_EDIT_MODULE) == 0)
		{
			// Set Initialize ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
			// No params
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Received %s message - Initialize module.", rxMsg.AddressPattern());
#endif
			sequencerModule->ctlMsgQueue.push(CreateOSCRecvMsg(TSExternalControlMessage::MessageType::InitializeEditModule));
		}
		else
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
			debug("Unknown OSC message: %s received.", rxMsg.AddressPattern());
#endif
		}
	} // end try
	catch (osc::Exception& e) {
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_LOW
		debug("Error parsing OSC message %s: %s", rxMsg.AddressPattern(), e.what());
#endif
	} // end catch
	return;
} // end ProcessMessage()
